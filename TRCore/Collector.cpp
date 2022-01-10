#include "stdafx.h"
#include "Collector.h"
#include "SourceTypes.h"
#include "ReportTypes.h"
#include "Sources.h"
#include "Content.h"
#include "Diff.h"
#include "Transaction.h"
#include "Executive.h"
#include "JobManager.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
#include "CurrentReportDataset.h"
namespace TR { namespace Core {

class CollectingJob: public Job, public std::enable_shared_from_this<CollectingJob>
{
public:
    class Routine
    {
    public:
        enum ResultType { INITIAL_REPORT, CONTENT_EQUAL, CONTENT_DIFFERENT };
        using FunctorType = std::function<ResultType()>;

        Routine(std::shared_ptr<CollectingJob> job):
            m_job(job),
            m_collector(job->m_collector),
            m_sources(*job->m_collector.m_sources),
            m_report_types(*job->m_collector.m_report_types)
        {
        }

        std::function<ResultType()> prepare_functor()
        {
            auto job = m_job.lock();
            if (!job)
            {
                return nullptr;
            }

            auto source = m_sources.get_source(job->m_source_key);
            auto& report_type = m_report_types.get_report_type(job->m_report_type_uuid);
            auto current_report = m_collector.find_current_report(source, report_type);
            if (!current_report)
            {
                return []{ return INITIAL_REPORT; };
            }

            auto current_content = current_report->get_content();           
            auto next_content = job->m_content;
            return [this, current_content, next_content]{
                if (!next_content->is_equal(*current_content))
                {
                    m_diff = current_content->make_diff(*next_content);
                    return CONTENT_DIFFERENT;
                }
                return CONTENT_EQUAL;
            };
        }

        void on_completed(ResultType result) const
        {
            auto job = m_job.lock();
            if (!job)
            {
                return;
            }

            Transaction t(*m_collector.m_db);
            auto source = m_sources.get_source(job->m_source_key);
            auto& report_type = m_report_types.get_report_type(job->m_report_type_uuid);

            switch (result)
            {
            case INITIAL_REPORT:
            {
                m_collector.persist_current_report(source, report_type, job->m_content, nullptr, job->m_time, job->m_time, job->m_transformation_circumstances, t);
                break;
            }

            case CONTENT_EQUAL:
            {
                m_collector.update_current_report(source, report_type, job->m_time, job->m_transformation_circumstances, t);
                break;
            }

            case CONTENT_DIFFERENT:
            {
                m_collector.update_current_report(source, report_type, job->m_content, m_diff, job->m_time, job->m_time, job->m_transformation_circumstances, t);
                break;
            }

            default:
                _ASSERT(false);
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
        std::weak_ptr<CollectingJob> m_job;
        Collector& m_collector;
        Sources& m_sources;
        ReportTypes& m_report_types;
        std::shared_ptr<Diff> m_diff;
    };
    
    CollectingJob(Collector& collector, const Report& report, const TransformationCircumstances& transformation_circumstances):
        m_collector(collector),
        m_source_key(report.get_source_ref().get_key()),
        m_report_type_uuid(report.get_report_type_ref().get_uuid()),
        m_content(report.get_content()),    
        m_time(report.get_time()),
        m_transformation_circumstances(transformation_circumstances)
    {
    }   

    void do_run()
    {
        m_collector.m_executive->run_processing(Routine(shared_from_this()));
    }

private:
    Collector& m_collector;
    SourceKey m_source_key;
    ReportTypeUUID m_report_type_uuid;
    std::shared_ptr<Content> m_content; 
    time_t m_time;
    TransformationCircumstances m_transformation_circumstances;
};

class Collector::CollectingManager
{
public:
    CollectingManager(Collector& collector, Executive& executive):
        m_collector(collector),
        m_executive(executive)
    {
    }

    void collect_report(const Report& report, const TransformationCircumstances& transformation_circumstances)
    {
        auto strand = std::make_pair(report.get_source_ref().get_key(), report.get_report_type_ref().get_uuid());
        m_job_manager.run_job(strand, std::make_shared<CollectingJob>(m_collector, report, transformation_circumstances));
    }

private:
    Collector& m_collector;
    Executive& m_executive;
    JobManager<std::pair<SourceKey, ReportTypeUUID>> m_job_manager;
};

Report::Report(SourceRef source_ref, ReportTypeRef report_type_ref, stl_tools::lazy_shared_ptr<Content> content, time_t time):
    m_source_ref(source_ref),
    m_report_type_ref(report_type_ref),
    m_content(content),
    m_time(time)
{
}

SourceRef Report::get_source_ref() const
{
    return m_source_ref;
}

ReportTypeRef Report::get_report_type_ref() const
{
    return m_report_type_ref;
}

std::shared_ptr<Content> Report::get_content() const
{
    return m_content.share();
}

time_t Report::get_time() const
{
    return m_time;
}

CurrentReport::CurrentReport(CurrentReportRef ref, stl_tools::lazy_shared_ptr<Content> content, std::shared_ptr<Diff> diff, time_t time, time_t check_time, UUID uuid, TransformationCircumstances transformation_circumstances):
    Report(ref.get_source_ref(), ref.get_report_type_ref(), content, time),
    m_ref(ref),
    m_diff(diff),
    m_check_time(check_time),
    m_uuid(uuid),
    m_transformation_circumstances(std::move(transformation_circumstances))
{
}

std::shared_ptr<Diff> CurrentReport::get_diff() const
{
    return m_diff;
}

time_t CurrentReport::get_check_time() const
{
    return m_check_time;
}

UUID CurrentReport::get_uuid() const
{
    return m_uuid;
}

const TransformationCircumstances& CurrentReport::get_transformation_circumstances() const
{
    return m_transformation_circumstances;
}

CurrentReportRef CurrentReport::get_ref() const
{
    return m_ref;
}

CurrentReport::operator CurrentReportRef() const
{
    return get_ref();
}

Collector::Collector():
    m_db(nullptr),
    m_report_types(nullptr),
    m_source_types(nullptr),
    m_sources(nullptr),
    m_collecting_manager(nullptr),
    m_executive(nullptr)
{
}

Collector::~Collector()
{
}

void Collector::set_database(Database* db)
{
    m_db = db;
    m_current_report_dataset = std::make_unique<CurrentReportDataset>(*m_db);
}

void Collector::set_report_types(ReportTypes* report_types)
{
    static_assert(COLLECTOR_PRIORITY > ReportTypes::REPORT_TYPES_PRIORITY, "Wrong component priority"); 
    m_report_types = report_types;
    m_report_types->connect_remove_report_type([this](UUID uuid, Transaction& t){
        on_remove_report_type(uuid, t);
    }, COLLECTOR_PRIORITY);
}

void Collector::set_source_types(SourceTypes* source_types)
{
    static_assert(COLLECTOR_PRIORITY > SourceTypes::SOURCE_TYPES_PRIORITY, "Wrong component priority");
    m_source_types = source_types;  
}

void Collector::set_sources(Sources* sources)
{
    static_assert(COLLECTOR_PRIORITY > Sources::SOURCES_PRIORITY, "Wrong component priority");
    m_sources = sources;
    m_sources->connect_remove_source([this](SourceKey source_key, const std::vector<SourceKey>& removed_sources, Transaction& t){
        on_remove_sources(removed_sources, t);
    }, COLLECTOR_PRIORITY);
}

void Collector::run(Executive* executive, Transaction& t)
{
    transact_assign(m_collecting_manager, std::make_unique<CollectingManager>(*this, *executive), t);
    transact_assign(m_executive, executive, t); 
}

void Collector::collect_report(const Report& report, const TransformationCircumstances& transformation_circumstances)
{
    m_collecting_manager->collect_report(report, transformation_circumstances);
}

CurrentReport Collector::get_current_report(SourceRef source_ref, ReportTypeRef report_type_ref) const
{
    ReadOnlyTransaction t(*m_db);
    auto current_report_data = m_current_report_dataset->load(pack(source_ref), pack(report_type_ref));
    return unpack_current_report(current_report_data);
}

std::vector<CurrentReport> Collector::get_current_reports(SourceRef source_ref) const
{
    ReadOnlyTransaction t(*m_db);
    std::vector<CurrentReport> current_reports;
    for (auto& current_report_data : m_current_report_dataset->query_source_equal(source_ref.get_key()))
    {
        current_reports.emplace_back(unpack_current_report(current_report_data));
    }
    return current_reports;
}

boost::optional<CurrentReport> Collector::find_current_report(SourceRef source_ref, ReportTypeRef m_report_type_ref) const
{
    ReadOnlyTransaction t(*m_db);
    if (auto current_report_data = m_current_report_dataset->find(source_ref.get_key(), m_report_type_ref.get_uuid()))
    {
        return unpack_current_report(*current_report_data);     
    }
    return boost::none; 
}

boost::optional<CurrentReport> Collector::find_current_report(UUID uuid) const
{
    ReadOnlyTransaction t(*m_db);
    if (auto current_report_data = m_current_report_dataset->find(uuid))
    {
        return unpack_current_report(*current_report_data);
    }
    return boost::none;
}

void Collector::update_current_report(SourceRef source_ref, ReportTypeRef report_type_ref, std::shared_ptr<Content> content, std::shared_ptr<Diff> diff, time_t time, time_t check_time, const TransformationCircumstances& transformation_circumstances, Transaction& t)
{   
    CurrentReport current_report({this, source_ref, report_type_ref}, content, diff, time, check_time, stl_tools::gen_uuid(), transformation_circumstances);
    m_current_report_dataset->update(pack_current_report(current_report), t);
    m_db->update(pack_current_report_content(current_report));
    m_update_current_report_sig(current_report, true, t);
}

void Collector::persist_current_report(SourceRef source_ref, ReportTypeRef report_type_ref, std::shared_ptr<Content> content, std::shared_ptr<Diff> diff, time_t time, time_t check_time, const TransformationCircumstances& transformation_circumstances, Transaction& t)
{
    CurrentReport current_report({this, source_ref, report_type_ref}, content, diff,time, check_time, stl_tools::gen_uuid(), transformation_circumstances);
    m_current_report_dataset->persist(pack_current_report(current_report), t);
    m_db->persist(pack_current_report_content(current_report));
    m_update_current_report_sig(current_report, true, t);
}

void Collector::update_current_report(SourceRef source_ref, ReportTypeRef report_type_ref, time_t check_time, const TransformationCircumstances& transformation_circumstances, Transaction& t)
{
    auto current_report_data = m_current_report_dataset->load(source_ref.get_key(), report_type_ref.get_uuid());
    current_report_data.m_check_time = check_time;
    current_report_data.m_transformation_circumstances = { 
        transformation_circumstances.m_transformation_uuid,
        transformation_circumstances.m_setup_uuid,
        transformation_circumstances.m_input_uuids};

    m_current_report_dataset->update(current_report_data, t);
    m_update_current_report_sig(CurrentReportRef(this, source_ref, report_type_ref), false, t);
}

CurrentReportData Collector::pack_current_report(const CurrentReport& current_report) const
{
    return CurrentReportData(
        current_report.get_source_ref().get_key(),
        current_report.get_report_type_ref().get_uuid(),
        current_report.m_diff ? boost::make_optional(current_report.m_diff->as_blob()) : boost::none,
        current_report.get_time(),
        current_report.get_check_time(),
        current_report.get_uuid(),
        {
            current_report.get_transformation_circumstances().m_transformation_uuid,
            current_report.get_transformation_circumstances().m_setup_uuid,
            current_report.get_transformation_circumstances().m_input_uuids
        });
}

CurrentReportContentData Collector::pack_current_report_content(const CurrentReport& current_report) const
{
    return CurrentReportContentData(
        current_report.get_source_ref().get_key(),
        current_report.get_report_type_ref().get_uuid(),
        current_report.get_content()->as_blob());
}

CurrentReport Collector::unpack_current_report(const CurrentReportData& current_report_data) const
{   
    auto source_key = current_report_data.m_identity.m_source_key;
    auto report_type_uuid = ReportTypeUUID(current_report_data.m_identity.m_report_type_uuid);
    auto& report_type = m_report_types->get_report_type(report_type_uuid);

    return CurrentReport(
        {this, unpack(source_key, m_sources), unpack(report_type_uuid, m_report_types)},
        [this, source_key, report_type_uuid]{
            ReadOnlyTransaction t(*m_db);
            CurrentReportContentData current_report_content_data;
            m_db->load<CurrentReportContentData>(CurrentReportIdentity(source_key, report_type_uuid), current_report_content_data);
            return unpack_current_report_content(current_report_content_data);
        },
        current_report_data.m_diff ? report_type.make_diff(*current_report_data.m_diff) : nullptr,
        current_report_data.m_time, 
        current_report_data.m_check_time,
        current_report_data.m_uuid,
        {
            current_report_data.m_transformation_circumstances.m_transformation_uuid,
            current_report_data.m_transformation_circumstances.m_setup_uuid,
            current_report_data.m_transformation_circumstances.m_input_uuids
        });
}

std::shared_ptr<Content> Collector::unpack_current_report_content(const CurrentReportContentData& current_report_content_data) const
{
    auto& report_type = m_report_types->get_report_type(ReportTypeUUID(current_report_content_data.m_identity.m_report_type_uuid));
    return report_type.make_content(current_report_content_data.m_content);
}

void Collector::on_remove_report_type(UUID report_type_uuid, Transaction& t)
{
    m_current_report_dataset->erase_report_type_equal(ReportTypeUUID(report_type_uuid), t);
    m_db->erase_query<CurrentReportContentData>(odb::query<CurrentReportContentData>::identity.report_type_uuid.value == report_type_uuid);
}

void Collector::on_remove_sources(const std::vector<SourceKey>& removed_sources, Transaction& t)
{
    m_current_report_dataset->erase_source_equal(removed_sources, t);
    m_db->erase_query<CurrentReportContentData>(odb::query<CurrentReportContentData>::identity.source_key.value.in_range(removed_sources.begin(), removed_sources.end()));
}

}} //namespace TR { namespace Core {
