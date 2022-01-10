#include "stdafx.h"
#include "Syslog.h"
#include "CoreDomain.h"
#include "Transaction.h"
#include "Executive.h"
#include "JobManager.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
#include "SyslogData-odb.hxx"
namespace TR { namespace Core {

class ParseSyslogJob: public Job, public std::enable_shared_from_this<ParseSyslogJob>
{
public:
    class Routine
    {
    public:
        Routine(std::shared_ptr<ParseSyslogJob> job):
            m_job(job),
            m_sources(*job->m_syslog.m_sources),
            m_stream_types(*job->m_syslog.m_stream_types),
            m_reservoir(*job->m_syslog.m_reservoir)
        {
        }

        std::function<std::vector<StreamMessage>()> prepare_functor()
        {
            auto job = m_job.lock();
            if (!job)
            {
                return nullptr;
            }

            auto source = m_sources.get_source(job->m_source_key);
            auto& source_type = *source.get_source_type();
            auto& stream_type = m_stream_types.get_stream_type(job->m_stream_type_uuid);

            auto& syslog_transport = dynamic_cast<const SyslogTransport&>(source_type.get_transport(stream_type));
            auto parser = syslog_transport.prepare_parser();
            auto message = job->m_message;
            return [parser, message](){
                return parser(message);
            };
        }

        void on_completed(std::vector<StreamMessage> stream_messages) const
        {
            auto job = m_job.lock();
            if (!job)
            {
                return;
            }

            Transaction t(*job->m_syslog.m_db);
            auto source = m_sources.get_source(job->m_source_key);
            auto& stream_type = m_stream_types.get_stream_type(job->m_stream_type_uuid);

            for (auto& message : stream_messages)
            {
                m_reservoir.append_record({
                    stream_type,
                    source,                 
                    std::move(message)
                }, t);
            }           

            t.commit();
            job->emit_completed();
        }

        void on_failed(const std::exception& err) const
        {
            auto job = m_job.lock();
            if (!job)
            {
                return;
            }
            job->emit_failed(err);
        }

        void on_cancel() const
        {
            _ASSERT(!m_job.lock());
        }

    private:
        std::weak_ptr<ParseSyslogJob> m_job;
        Sources& m_sources;
        StreamTypes& m_stream_types;
        Reservoir& m_reservoir;
    };

    ParseSyslogJob(Syslog& syslog, SourceKey source_key, StreamTypeUUID stream_type_uuid, std::string message):
        m_syslog(syslog),
        m_source_key(source_key),
        m_stream_type_uuid(stream_type_uuid),
        m_message(std::move(message))
    {
    }

    void do_run()
    {
        m_syslog.m_executive->run_processing(Routine(shared_from_this()));
    }

private:
    Syslog& m_syslog;
    SourceKey m_source_key; 
    StreamTypeUUID m_stream_type_uuid;
    std::string m_message;
};

class Syslog::ParseSyslogManager
{
public:
    ParseSyslogManager(Syslog& syslog, Executive& executive):
        m_syslog(syslog),
        m_executive(executive)
    {
    }

    void append_message(SourceRef source_ref, StreamTypeRef stream_type_ref, std::string message)
    {
        auto strand = std::make_pair(source_ref.get_key(), stream_type_ref.get_uuid());
        m_job_manager.run_job(strand, std::make_shared<ParseSyslogJob>(m_syslog, source_ref.get_key(), stream_type_ref.get_uuid(), std::move(message)));
    }

private:
    Syslog& m_syslog;
    Executive& m_executive;
    JobManager<std::pair<SourceKey, StreamTypeUUID>> m_job_manager;
};

SyslogTransport::SyslogTransport(StreamTypeRef stream_type_ref):
    StreamTransport(stream_type_ref)
{
}

Syslog::Syslog(Database& db, CoreDomain& core_domain):
    m_db(&db),
    m_stream_types(&core_domain.m_basis.m_stream_types),
    m_reservoir(&core_domain.m_registry.m_reservoir),
    m_sources(&core_domain.m_registry.m_sources),
    m_executive(nullptr)
{
    static_assert(Syslog::SYSLOG_PRIORITY > StreamTypes::STREAM_TYPES_PRIORITY, "Wrong component priority");
    static_assert(Syslog::SYSLOG_PRIORITY > Reservoir::RESERVOIR_PRIORITY, "Wrong component priority");
    static_assert(Syslog::SYSLOG_PRIORITY > Sources::SOURCES_PRIORITY, "Wrong component priority");

    m_sources->connect_add_source([this](SourceRef source_ref, Transaction& t){
        on_add_source(source_ref, t);
    }, SYSLOG_PRIORITY);

    m_sources->connect_update_source([this](SourceRef source_ref, Transaction& t){
        on_update_source(source_ref, t);
    }, SYSLOG_PRIORITY);

    m_sources->connect_remove_source([this](SourceKey source_key, const std::vector<SourceKey>& removed_sources, Transaction& t){
        on_remove_sources(removed_sources, t);
    }, SYSLOG_PRIORITY);
}

Syslog::~Syslog()
{
}

void Syslog::run(Executive* executive, Transaction& t)
{
    _ASSERT(!m_executive);
    transact_assign(m_executive, executive, t);
    transact_assign(m_parse_syslog_manager, std::make_unique<ParseSyslogManager>(*this, *m_executive), t);

    t.connect_commit([this]{
        m_asio_syslog = std::make_unique<AsioSyslog>();
        m_asio_syslog->connect([this](const std::string& address, const std::string& message)
        {
            m_executive->async(std::bind(&Syslog::dispatch_message, this, address, message));
        });
    });
}

void Syslog::on_add_source(SourceRef source_ref, Transaction& t)
{
    auto source = *source_ref;
    auto& source_type = *source.get_source_type();
    for (auto stream_ref : source_type.get_streams())
    {
        if (auto syslog_transport = dynamic_cast<const SyslogTransport*>(&source_type.get_transport(stream_ref)))
        {
            m_db->persist<SyslogData>(SyslogData{
                pack(source.get_ref()),
                pack(syslog_transport->get_stream_type()),
                syslog_transport->get_source_addr(*source.get_config())
            });
        }
    }
}

void Syslog::on_update_source(SourceRef source_ref, Transaction& t)
{
    auto source = *source_ref;
    auto& source_type = *source.get_source_type();
    for (auto syslog_data : m_db->query<SyslogData>(odb::query<SyslogData>::identity.source_key.value == source_ref.get_key()))
    {
        auto stream_ref = unpack(syslog_data.m_identity.m_stream_type_uuid, m_stream_types);
        auto syslog_transport = dynamic_cast<const SyslogTransport*>(&source_type.get_transport(stream_ref));
        syslog_data.m_address = syslog_transport->get_source_addr(*source.get_config());
        m_db->update(syslog_data);
    }
}

void Syslog::on_remove_sources(const std::vector<SourceKey>& removed_sources, Transaction& t)
{
    m_db->erase_query<SyslogData>(odb::query<SyslogData>::identity.source_key.value.in_range(removed_sources.begin(), removed_sources.end()));
}

void Syslog::dispatch_message(const std::string& address, const std::string& message)
{
    ReadOnlyTransaction t(*m_db);
    for (auto syslog_data : m_db->query<SyslogData>(odb::query<SyslogData>::address == address))
    {
        SourceRef source_ref = unpack(syslog_data.m_identity.m_source_key, m_sources);
        StreamTypeRef stream_type_ref = unpack(syslog_data.m_identity.m_stream_type_uuid, m_stream_types);
        m_parse_syslog_manager->append_message(source_ref, stream_type_ref, message);
    }
}

}} //namespace TR { namespace Core {