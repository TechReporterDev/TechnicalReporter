#include "stdafx.h"
#include "EventFlow.h"
#include "PlainFacade.h"
#include "CoreImpl.h"
#include "Security.h"
#include "Async.h"
#include "Tasks.h"
#include "ExportTasks.h"
#include "Transaction.h"
namespace TR { namespace Core {

bool operator < (const SourceSubject& left, const SourceSubject& right)
{
    return left.m_source_key < right.m_source_key;
}

bool operator < (const ReportSubject& left, const ReportSubject& right)
{
    if (left.m_report_type_uuid < right.m_report_type_uuid)
    {
        return true;
    }

    if (right.m_report_type_uuid < left.m_report_type_uuid)
    {
        return false;
    }

    return left.m_source_key < right.m_source_key;
}

bool operator < (const LinkSubject& left, const LinkSubject& right)
{
    if (left.m_parent_key < right.m_parent_key)
    {
        return true;
    }

    if (right.m_parent_key < left.m_parent_key)
    {
        return false;
    }

    return left.m_child_key < right.m_child_key;
}

Event::Event(EventFlow& event_flow, Cathegory cathegory):
    m_event_flow(event_flow),   
    m_core_impl(event_flow.m_core_impl),
    m_security(event_flow.m_security),
    m_cathegory(cathegory)
{
}

Event::Cathegory Event::get_cathegory() const
{
    return m_cathegory;
}

SourceEvent::SourceEvent(EventFlow& event_flow, Cathegory cathegory, SourceKey source_key):
    Event(event_flow, cathegory),
    m_sources(m_core_impl.m_core_domain.m_registry.m_sources),
    m_source_key(source_key)
{
}

CoreSubject SourceEvent::get_subject() const
{
    return SourceSubject{m_source_key};
}

std::vector<CoreSubject> SourceEvent::get_relations() const
{
    auto source = m_sources.get_source(m_source_key);   
    if (source.has_parent())
    {
        return{SourceSubject{source.get_parent().get_key()}};
    }
    return{};
}

AddSourceEvent::AddSourceEvent(EventFlow& event_flow, SourceRef source_ref):
    SourceEvent(event_flow, ADD_UPDATE_EVENT, source_ref.get_key())
{
}

std::unique_ptr<Event> AddSourceEvent::combine(const Event& evt) const
{
    if (dynamic_cast<const UpdateSourceEvent*>(&evt))
    {
        return std::make_unique<AddSourceEvent>(*this);
    }   
    else if (dynamic_cast<const RemoveSourceEvent*>(&evt))
    {
        return nullptr;
    }
    else
    {
        _ASSERT(false);
    }
    
    throw std::logic_error("Invalid event found");
}

void AddSourceEvent::emit() const
{
    auto source = m_sources.get_source(m_source_key);

    std::vector<Key> sessions;
    for (auto& session : m_security.get_sessions())
    {
        if (m_security.check_access_rights(session.m_user_key, source, Access::READ_ACCESS))
        {
            sessions.push_back(session.m_key);
        }
    }

    PlainFacade facade(&m_core_impl);
    auto source_info = facade.get_source_info(m_source_key);    
    dispatch(boost::bind(&EventDispatcher::on_add_source, _1, sessions, source_info));
}

UpdateSourceEvent::UpdateSourceEvent(EventFlow& event_flow, SourceRef source_ref):
    SourceEvent(event_flow, ADD_UPDATE_EVENT, source_ref.get_key())
{
}

std::unique_ptr<Event> UpdateSourceEvent::combine(const Event& evt) const
{
    if (dynamic_cast<const UpdateSourceEvent*>(&evt))
    {
        return std::make_unique<UpdateSourceEvent>(*this);
    }
    else if (dynamic_cast<const RemoveSourceEvent*>(&evt))
    {
        return nullptr;
    }
    else
    {
        _ASSERT(false);
    }

    throw std::logic_error("Invalid event found");
}

void UpdateSourceEvent::emit() const
{
    auto source = m_sources.get_source(m_source_key);

    std::vector<Key> sessions;
    for (auto& session : m_security.get_sessions())
    {
        if (m_security.check_access_rights(session.m_user_key, source, Access::READ_ACCESS))
        {
            sessions.push_back(session.m_key);
        }
    }

    PlainFacade facade(&m_core_impl);
    auto source_info = facade.get_source_info(m_source_key);
    dispatch(boost::bind(&EventDispatcher::on_update_source, _1, sessions, source_info));
}

RemoveSourceEvent::RemoveSourceEvent(EventFlow& event_flow, SourceKey source_key, std::vector<SourceKey> removed_sources):
    SourceEvent(event_flow, REMOVE_EVENT, source_key),
    m_removed_sources(std::move(removed_sources))
{
}

std::vector<CoreSubject> RemoveSourceEvent::get_relations() const
{
    return{};
}

std::unique_ptr<Event> RemoveSourceEvent::combine(const Event& evt) const
{
    _ASSERT(false);
    throw std::logic_error("Invalid event found");
}

void RemoveSourceEvent::emit() const
{
    std::vector<Key> sessions;
    for (auto& session : m_security.get_sessions())
    {
        sessions.push_back(session.m_key);
    }

    dispatch(boost::bind(&EventDispatcher::on_remove_source, _1, sessions, m_source_key, m_removed_sources));
}

LinkEvent::LinkEvent(EventFlow& event_flow, Cathegory cathegory, Link link):
    Event(event_flow, cathegory),
    m_parent_key(link.m_parent_ref.get_key()),
    m_child_key(link.m_child_ref.get_key())
{
}

CoreSubject LinkEvent::get_subject() const
{
    return LinkSubject{m_parent_key, m_child_key};
}

std::vector<CoreSubject> LinkEvent::get_relations() const
{
    return {SourceSubject{m_parent_key}, SourceSubject{m_child_key}};
}

AddLinkEvent::AddLinkEvent(EventFlow& event_flow, Link link):
    LinkEvent(event_flow, ADD_UPDATE_EVENT, link)
{
}

std::unique_ptr<Event> AddLinkEvent::combine(const Event& evt) const
{
    if (dynamic_cast<const RemoveLinkEvent*>(&evt))
    {
        return nullptr;
    }
    else
    {
        _ASSERT(false);
    }
    throw std::logic_error("Invalid event found");
}

void AddLinkEvent::emit() const
{
    std::vector<Key> sessions;
    for (auto& session : m_security.get_sessions())
    {
        sessions.push_back(session.m_key);
    }

    dispatch(boost::bind(&EventDispatcher::on_add_link, _1, sessions, TR::Link{m_parent_key, m_child_key}));
}

RemoveLinkEvent::RemoveLinkEvent(EventFlow& event_flow, Link link):
    LinkEvent(event_flow, REMOVE_EVENT, link)
{
}

std::unique_ptr<Event> RemoveLinkEvent::combine(const Event& evt) const
{
    _ASSERT(false);
    throw std::logic_error("Invalid event found");
}

void RemoveLinkEvent::emit() const
{
    std::vector<Key> sessions;
    for (auto& session : m_security.get_sessions())
    {
        sessions.push_back(session.m_key);
    }

    dispatch(boost::bind(&EventDispatcher::on_remove_link, _1, sessions, TR::Link{m_parent_key, m_child_key}));
}

UpdateReportEvent::UpdateReportEvent(EventFlow& event_flow, CurrentReportRef current_report_ref, bool content_changed):
    Event(event_flow, ADD_UPDATE_EVENT),
    m_source_key(current_report_ref.get_source_ref().get_key()),
    m_report_type_uuid(current_report_ref.get_report_type_ref().get_uuid()),
    m_content_changed(content_changed)
{
}

CoreSubject UpdateReportEvent::get_subject() const
{
    return ReportSubject{m_report_type_uuid, m_source_key};
}

std::vector<CoreSubject> UpdateReportEvent::get_relations() const
{
    return {SourceSubject{m_source_key}};
}

std::unique_ptr<Event> UpdateReportEvent::combine(const Event& evt) const
{
    if (dynamic_cast<const UpdateReportEvent*>(&evt))
    {
        return std::make_unique<UpdateReportEvent>(*this);
    }   
    else
    {
        _ASSERT(false);
    }

    throw std::logic_error("Invalid event found");
}

void UpdateReportEvent::emit() const
{
    auto& sources = m_core_impl.m_core_domain.m_registry.m_sources;
    auto source = sources.get_source(m_source_key);

    std::vector<Key> sessions;
    for (auto& session : m_security.get_sessions())
    {
        if (m_security.check_access_rights(session.m_user_key, source, Access::READ_ACCESS))
        {
            sessions.push_back(session.m_key);
        }
    }

    PlainFacade facade(&m_core_impl);
    auto report_info = facade.get_current_report_info(m_source_key, m_report_type_uuid);
    dispatch(boost::bind(&EventDispatcher::on_update_current_report, _1, sessions, report_info, m_content_changed));
}

bool EventBatch::empty() const
{
    return m_events.empty();
}

void EventBatch::add_event(std::unique_ptr<Event> evt)
{
    auto current_event = m_events.find(evt->get_subject());
    if (current_event != m_events.end())
    {
        evt = current_event->second->combine(*evt);
        m_events.erase(current_event);
    }

    if (evt)
    {
        auto event_subject = evt->get_subject();
        m_events.insert(std::make_pair(event_subject, std::move(evt)));
    }
}

std::list<const Event*> topo_sort_events(std::map<CoreSubject, const Event*> events)
{
    std::list<const Event*> sorted_events;
    while (!events.empty())
    {
        std::vector<const Event*> stack;
        stack.push_back(std::move(events.begin()->second));
        events.erase(events.begin());

        while (!stack.empty())
        {
            auto& back = stack.back();
            for (auto& related_subject : back->get_relations())
            {
                auto related_event = events.find(related_subject);
                if (related_event != events.end())
                {
                    stack.push_back(std::move(related_event->second));
                    events.erase(related_event);
                }
            }

            if (back == stack.back())
            {
                sorted_events.push_back(std::move(back));
                stack.pop_back();
            }
        }
    }

    return sorted_events;
}

std::map<CoreSubject, const Event*> select_events(const std::map<CoreSubject, std::unique_ptr<Event>>& events, Event::Cathegory cathegory)
{
    std::map<CoreSubject, const Event*> selected_events;
    for (auto& evt : events | boost::adaptors::map_values | boost::adaptors::indirected)
    {
        if (evt.get_cathegory() == cathegory)
        {
            selected_events[evt.get_subject()] = &evt;
        }
    }
    return selected_events;
}

void EventBatch::emit()
{
    auto add_update_events = topo_sort_events(select_events(m_events, Event::ADD_UPDATE_EVENT));
    for (auto& evt : add_update_events | boost::adaptors::indirected)
    {
        evt.emit();
    }

    auto remove_events = topo_sort_events(select_events(m_events, Event::REMOVE_EVENT));
    for (auto& evt : remove_events | boost::adaptors::reversed | boost::adaptors::indirected)
    {
        evt.emit();
    }
}

EventFlow::EventFlow(CoreImpl& core_impl):
    m_core_impl(core_impl),
    m_security(core_impl.m_security),
    m_async(nullptr),
    m_dispatchers(nullptr)
{
    auto& sources = m_core_impl.m_core_domain.m_registry.m_sources;
    sources.connect_add_source([this](SourceRef source_ref, Transaction& t){
        auto& event_batch = get_event_batch(t);
        event_batch.add_event(std::make_unique<AddSourceEvent>(*this, source_ref));
    }, EVENT_FLOW_PRIORITY);

    sources.connect_update_source([this](SourceRef source_ref, Transaction& t){     
        auto& event_batch = get_event_batch(t);
        event_batch.add_event(std::make_unique<UpdateSourceEvent>(*this, source_ref));
    }, EVENT_FLOW_PRIORITY);

    sources.connect_remove_source([this](SourceKey source_key, const std::vector<SourceKey>& removed_sources, Transaction& t){
        auto& event_batch = get_event_batch(t);
        event_batch.add_event(std::make_unique<RemoveSourceEvent>(*this, source_key, removed_sources));
    }, EVENT_FLOW_PRIORITY);

    auto& links = m_core_impl.m_core_domain.m_registry.m_links;
    links.connect_add_link([this](Link link, Transaction& t){
        auto& event_batch = get_event_batch(t);
        event_batch.add_event(std::make_unique<AddLinkEvent>(*this, link));
    }, EVENT_FLOW_PRIORITY);

    links.connect_remove_link([this](Link link, Transaction& t){
        auto& event_batch = get_event_batch(t);
        event_batch.add_event(std::make_unique<RemoveLinkEvent>(*this, link));
    }, EVENT_FLOW_PRIORITY);

    auto& collector = m_core_impl.m_core_domain.m_registry.m_collector;
    collector.connect_update_current_report([this](CurrentReportRef current_report_ref, bool content_changed, Transaction& t){
        auto& event_batch = get_event_batch(t);
        event_batch.add_event(std::make_unique<UpdateReportEvent>(*this, current_report_ref, content_changed));
        event_batch.add_event(std::make_unique<UpdateSourceEvent>(*this, current_report_ref.get_source_ref()));
    }, EVENT_FLOW_PRIORITY);

    auto& integrity_checking = m_core_impl.m_core_domain.m_services.m_integrity_checking;
    integrity_checking.connect_update_checking_result([this](SourceRef source_ref, ReportTypeRef report_type_ref, IntegrityCheckingResult checking_result, Transaction& t){
        auto& event_batch = get_event_batch(t);
        event_batch.add_event(std::make_unique<UpdateSourceEvent>(*this, source_ref));
    }, EVENT_FLOW_PRIORITY);

    auto& validations = m_core_impl.m_core_domain.m_services.m_validations;
    validations.connect_update_validation_result([this](ValidationRef validation_ref, SourceRef source_ref, ValidationResult validation_result, Transaction& t){
        auto& event_batch = get_event_batch(t);
        event_batch.add_event(std::make_unique<UpdateSourceEvent>(*this, source_ref));
    }, EVENT_FLOW_PRIORITY);

    auto& compliances = m_core_impl.m_core_domain.m_services.m_compliances;
    compliances.connect_update_checking_result([this](ComplianceRef compliance_ref, SourceRef source_ref, ComplianceCheckingResult checking_result, Transaction& t){
        auto& event_batch = get_event_batch(t);
        event_batch.add_event(std::make_unique<UpdateSourceEvent>(*this, source_ref));
    }, EVENT_FLOW_PRIORITY);    

    auto& scheduler = m_core_impl.m_scheduler;
    scheduler.connect_update_task_result([this](SourceRef source_ref, PeriodicOperation operation, Transaction& t){
        auto& event_batch = get_event_batch(t);
        event_batch.add_event(std::make_unique<UpdateSourceEvent>(*this, source_ref));
    }, EVENT_FLOW_PRIORITY);
}

void EventFlow::set_async(Async* async)
{
    _ASSERT(!m_async);
    m_async = async;
    m_async->connect_completed([this](Key session_key, std::shared_ptr<Job> job){
        if (auto content_query = std::dynamic_pointer_cast<ContentQuery>(job))
        {
            dispatch_event(boost::bind(&EventDispatcher::on_query_content_completed, _1, session_key, content_query->get_id(), content_query->get_content()));
        }
        if (auto stream_segment_query = std::dynamic_pointer_cast<ActualStreamQuery>(job))
        {
            StreamSegment stream_segment(stream_segment_query->get_source_key(), stream_segment_query->get_stream_type_uuid());
            stream_segment.m_messages = stream_segment_query->get_messages();
            dispatch_event(boost::bind(&EventDispatcher::on_query_stream_completed, _1, session_key, stream_segment_query->get_id(), stream_segment));
        }
        else if (auto action_task = std::dynamic_pointer_cast<LaunchActionTask>(job))
        {
            dispatch_event(boost::bind(&EventDispatcher::on_action_completed, _1, session_key, action_task->get_id(), action_task->get_output()));
        }
        else if (auto shortcut_task = std::dynamic_pointer_cast<LaunchShortcutTask>(job))
        {
            dispatch_event(boost::bind(&EventDispatcher::on_action_completed, _1, session_key, shortcut_task->get_id(), shortcut_task->get_output()));
        }
        else if (auto reload_task = std::dynamic_pointer_cast<ReloadReportTask>(job))
        {
            dispatch_event(boost::bind(&EventDispatcher::on_reload_completed, _1, session_key, reload_task->get_id(), reload_task->get_content()));
        }
        else if (auto reload_task = std::dynamic_pointer_cast<RebuildReportTask>(job))
        {
            dispatch_event(boost::bind(&EventDispatcher::on_rebuild_completed, _1, session_key, reload_task->get_id()));
        }
        else if (auto reload_task = std::dynamic_pointer_cast<RebuildReportTask>(job))
        {
            dispatch_event(boost::bind(&EventDispatcher::on_rebuild_completed, _1, session_key, reload_task->get_id()));
        }
        else if (auto export_task = std::dynamic_pointer_cast<ExportSummary>(job))
        {
            dispatch_event(boost::bind(&EventDispatcher::on_export_completed, _1, session_key, export_task->get_id(), std::make_shared<BinaryContent>(export_task->get_output())));
        }
    });

    m_async->connect_failed([this](Key session_key, std::shared_ptr<Job> job, const std::exception& err){
        dispatch_event(boost::bind(&EventDispatcher::on_job_failed, _1, session_key, job->get_id(), boost::ref(err)));
    });
}

void EventFlow::set_dispatchers(EventDispatchers* dispatchers)
{
    _ASSERT(!m_dispatchers);
    m_dispatchers = dispatchers;
}

EventBatch& EventFlow::get_event_batch(Transaction& t)
{
    if (m_event_batch)
    {
        return *m_event_batch;
    }
    
    m_event_batch = std::make_unique<EventBatch>();
    t.connect_commit(std::bind(&EventFlow::emit_event_batch, this));
    t.connect_rollback(std::bind(&EventFlow::reset_event_batch, this));
    return *m_event_batch; 
}

void EventFlow::emit_event_batch()
{
    no_except([this]{
        if (m_event_batch->empty())
        {
            return;
        }

        m_event_batch->emit();
        reset_event_batch();
    });
}

void EventFlow::reset_event_batch()
{
    m_event_batch.reset();
}

}} //namespace TR { namespace Core {