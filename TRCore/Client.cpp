#include "stdafx.h"
#include "Client.h"

namespace TR {

EventHandler::EventHandler():
    m_client(nullptr)
{
}

EventHandler::EventHandler(Client* client):
    m_client(nullptr)
{
    _ASSERT(client);
    client->add_event_handler(this);
}

EventHandler::~EventHandler()
{
    if (m_client)
    {
        m_client->remove_event_handler(this);
    }
}

class JobHandler
{
public:
    JobHandler(int id, Client::JobCompletedHandler completed_handler, Client::JobFailedHandler failed_handler):
        m_id(id),
        m_completed_handler(completed_handler),
        m_failed_handler(failed_handler)        
    {
    }

    virtual ~JobHandler() = default;

    int get_id() const
    {
        return m_id;
    }

    virtual void on_job_failed(const std::string& err)
    {
        m_failed_handler(err);
    }

    virtual void on_job_completed(boost::any any)
    {
        m_completed_handler();
    }

protected:
    int m_id;
    Client::JobCompletedHandler m_completed_handler;
    Client::JobFailedHandler m_failed_handler;  
};

class ContentQueryHandler: public JobHandler
{
public:
    ContentQueryHandler(int id, Client::ContentReadyHandler completed_handler, Client::JobFailedHandler failed_handler):
        JobHandler(id, nullptr, failed_handler),
        m_completed_handler(completed_handler)
    {
    }

    virtual void on_job_completed(boost::any any)
    {
        auto content = boost::any_cast<std::shared_ptr<Content>>(any);
        m_completed_handler(content);
    }

protected:
    Client::ContentReadyHandler m_completed_handler;
};

class StreamQueryHandler: public JobHandler
{
public:
    StreamQueryHandler(int id, Client::StreamReadyHandler completed_handler, Client::JobFailedHandler failed_handler):
        JobHandler(id, nullptr, failed_handler),
        m_completed_handler(completed_handler)
    {
    }

    virtual void on_job_completed(boost::any any)
    {
        auto& stream_segment = boost::any_cast<StreamSegment>(any);
        m_completed_handler(std::move(stream_segment));
    }

protected:
    Client::StreamReadyHandler m_completed_handler;
};

Client::Job::Job():
    m_client(nullptr),
    m_id(0)
{
}

Client::Job::Job(const Client*client, int id):
    m_client(client),
    m_id(id)
{
}

Client::Job::~Job()
{
    if (is_running())
    {
        cancel();
    }
}

Client::Job::Job(Job&& job):
    m_client(job.m_client),
    m_id(job.m_id)
{
    job.m_id = 0;
}

Client::Job& Client::Job::operator = (Client::Job&& job)
{
    if (is_running())
    {
        cancel();
    }

    m_client = job.m_client;
    m_id = job.m_id;
    job.m_id = 0;
    return *this;
}

int Client::Job::get_id() const
{
    return m_id;    
}

bool Client::Job::is_running() const
{
    if (!m_id)
    {
        return false;
    }

    return boost::find_if(m_client->m_job_handlers, [&](std::unique_ptr<JobHandler>& job_handler){
        return job_handler->get_id() == m_id;
    }) != m_client->m_job_handlers.end();
}

void Client::Job::release()
{
    _ASSERT(m_id);
    m_id = 0;
}

void Client::Job::cancel()
{
    _ASSERT(m_id);
    auto handler_position = boost::find_if(m_client->m_job_handlers, [&](std::unique_ptr<JobHandler>& job_handler){
        return job_handler->get_id() == m_id;
    });

    if (handler_position != m_client->m_job_handlers.end())
    {
        m_client->m_job_handlers.erase(handler_position);
        m_client->do_cancel_job(m_id);
    }
}

Client::Client():
    m_session_key(0),
    m_event_transporter([](PackedEvent evt){evt(); })   
{
}

Client::Client(EventTransporter event_transporter):
    m_session_key(0),   
    m_event_transporter(event_transporter)
{
}

Client::~Client()
{
    _ASSERT(m_event_handlers.empty());
}

Key Client::get_session_key() const
{
    return m_session_key;
}

void Client::add_event_handler(EventHandler* event_handler)
{
    _ASSERT(boost::find(m_event_handlers, event_handler) == m_event_handlers.end()); // add once only
    m_event_handlers.push_back(event_handler);
    _ASSERT(!event_handler->m_client);
    event_handler->m_client = this;
}

void Client::remove_event_handler(EventHandler* event_handler)
{
    auto found = boost::find(m_event_handlers, event_handler);
    _ASSERT(found != m_event_handlers.end());
    (*found)->m_client = nullptr;
    m_event_handlers.erase(found);
}

Client::Job Client::query_current_content(SourceKey source_key, ReportTypeUUID report_type_uuid, ContentReadyHandler completed_handler, JobFailedHandler failed_handler) const
{
    int id = do_query_current_content(source_key, report_type_uuid);
    m_job_handlers.push_back(std::make_unique<ContentQueryHandler>(id, completed_handler, failed_handler));
    return Job(this, id);
}

Client::Job Client::query_archived_content(SourceKey source_key, ReportTypeUUID report_type_uuid, time_t time, ContentReadyHandler completed_handler, JobFailedHandler failed_handler) const
{
    int id = do_query_archived_content(source_key, report_type_uuid, time);
    m_job_handlers.push_back(std::make_unique<ContentQueryHandler>(id, completed_handler, failed_handler));
    return Job(this, id);
}

Client::Job Client::query_stream_segment(SourceKey source_key, StreamTypeUUID stream_type_uuid, time_t start, time_t stop, StreamReadyHandler completed_handler, JobFailedHandler failed_handler) const
{
    int id = do_query_stream_segment(source_key, stream_type_uuid, start, stop);
    m_job_handlers.push_back(std::make_unique<StreamQueryHandler>(id, completed_handler, failed_handler));
    return Job(this, id);
}


Client::Job Client::reload_report(SourceKey source_key, ReportTypeUUID report_type_uuid, ContentReadyHandler completed_handler, JobFailedHandler failed_handler)
{
    int id = do_reload_report(source_key, report_type_uuid);
    m_job_handlers.push_back(std::make_unique<ContentQueryHandler>(id, completed_handler, failed_handler));
    return Job(this, id);
}
    
Client::Job Client::rebuild_report(SourceKey source_key, ReportTypeUUID report_type_uuid, JobCompletedHandler completed_handler, JobFailedHandler failed_handler)
{
    int id = do_rebuild_report(source_key, report_type_uuid);
    m_job_handlers.push_back(std::make_unique<JobHandler>(id, completed_handler, failed_handler));
    return Job(this, id);
}

Client::Job Client::launch_action(SourceKey source_key, ActionUUID action_uuid, Params params, ContentReadyHandler completed_handler, JobFailedHandler failed_handler)
{
    int id = do_launch_action(source_key, action_uuid, std::move(params));
    m_job_handlers.push_back(std::make_unique<ContentQueryHandler>(id, completed_handler, failed_handler));
    return Job(this, id);
}

Client::Job Client::launch_shortcut(SourceKey source_key, UUID shortcut_uuid, ContentReadyHandler completed_handler, JobFailedHandler failed_handler)
{
    int id = do_launch_shortcut(source_key, shortcut_uuid);
    m_job_handlers.push_back(std::make_unique<ContentQueryHandler>(id, completed_handler, failed_handler));
    return Job(this, id);
}

Client::Job Client::export_summary(ContentReadyHandler completed_handler, JobFailedHandler failed_handler)
{
    int id = do_export_summary();
    m_job_handlers.push_back(std::make_unique<ContentQueryHandler>(id, completed_handler, failed_handler));
    return Job(this, id);
}

void Client::forward_add_source_event(const SourceInfo& source_info)
{
    m_event_transporter([=]
    {
        for (auto event_handler : m_event_handlers)
        {
            event_handler->on_add_source(source_info);
        }
    });
}

void Client::forward_update_source_event(const SourceInfo& source_info)
{
    m_event_transporter([=]
    {
        for (auto event_handler : m_event_handlers)
        {
            event_handler->on_update_source(source_info);
        }
    });
}

void Client::forward_remove_source_event(Key source_key, const std::vector<SourceKey>& removed_sources)
{
    m_event_transporter([=]
    {
        for (auto event_handler : m_event_handlers)
        {
            event_handler->on_remove_source(source_key, removed_sources);
        }
    });
}

void Client::forward_add_link_event(Link link)
{
    m_event_transporter([=]
    {
        for (auto event_handler : m_event_handlers)
        {
            event_handler->on_add_link(link);
        }
    });
}
void Client::forward_remove_link_event(Link link)
{
    m_event_transporter([=]
    {
        for (auto event_handler : m_event_handlers)
        {
            event_handler->on_remove_link(link);
        }
    });
}

void Client::forward_update_current_report(const CurrentReportInfo& current_report_info, bool content_changed)
{
    m_event_transporter([=]
    {
        for (auto event_handler : m_event_handlers)
        {
            event_handler->on_update_current_report(current_report_info, content_changed);
        }
    });
}

void Client::forward_job_completed(int id, boost::any any)
{
    m_event_transporter([=]
    {
        auto handler_position = boost::find_if(m_job_handlers, [&](std::unique_ptr<JobHandler>& job_handler){
            return job_handler->get_id() == id;
        });

        if (handler_position != m_job_handlers.end())
        {
            auto handler = std::move(*handler_position);
            m_job_handlers.erase(handler_position);
            handler->on_job_completed(std::move(any));
        }
    });
}

void Client::forward_job_failed(int id, const std::string& err)
{
    m_event_transporter([=]
    {
        auto handler_position = boost::find_if(m_job_handlers, [&](std::unique_ptr<JobHandler>& job_handler){
            return job_handler->get_id() == id;
        });

        if (handler_position != m_job_handlers.end())
        {
            auto handler = std::move(*handler_position);
            m_job_handlers.erase(handler_position);
            handler->on_job_failed(err);
        }
    });
}

}// namespace TR {
