#include "stdafx.h"
#include "Conveyor.h"
#include "Basis.h"
#include "Sources.h"
#include "Links.h"
#include "Collector.h"
#include "Reservoir.h"
#include "SourceResources.h"
#include "Executive.h"
#include "JobManager.h"
#include "Transaction.h"
namespace TR { namespace Core {

class TransformationJob : public Job, public std::enable_shared_from_this<TransformationJob>
{
public:
    class RefreshReportRoutine
    {
    public:
        RefreshReportRoutine(std::shared_ptr<TransformationJob> job) :
            m_job(job),
            m_conveyor(job->m_conveyor),
            m_sources(*job->m_conveyor.m_sources),
            m_report_types(job->m_conveyor.m_basis.m_report_types),
            m_collector(*job->m_conveyor.m_collector),
            m_transformations(job->m_conveyor.m_basis.m_transformations),
            m_source_resources(*job->m_conveyor.m_source_resources),
            m_source_key(job->m_source_key),
            m_report_type_uuid(boost::get<ReportTypeUUID>(job->m_resource_uuid)),
            m_time(0)
        {
        }

        std::function<std::unique_ptr<Content>()> prepare_functor()
        {
            auto job = m_job.lock();
            if (!job)
            {
                return nullptr;
            }

            auto source = m_sources.get_source(m_source_key);
            auto& report_type = m_report_types.get_report_type(m_report_type_uuid);
            auto source_resource = m_source_resources.get_source_resource(source, report_type);

            if (!source_resource.m_significant)
            {
                return nullptr;
            }

            if (m_transformation_circumstances.m_input_uuids.empty())
            {
                if (auto current_report = m_collector.find_current_report(source, report_type))
                {
                    m_transformation_circumstances = current_report->get_transformation_circumstances();
                    m_time = current_report->get_check_time();
                }
            }

            auto current_circumstances = m_conveyor.get_transformation_circumstances(source_resource);
            if (m_transformation_circumstances == current_circumstances)
            {
                // update time
                return nullptr;
            }
            m_transformation_circumstances = std::move(current_circumstances);

            if (source_resource.m_activity.type() == typeid(SelectionActivity))
            {
                return prepare_selection(source_resource);
            }

            else if (source_resource.m_activity.type() == typeid(GroupingActivity))
            {
                return prepare_grouping(source_resource);
            }

            else
            {
                //_ASSERT(false);
            }

            return nullptr;
        }

        void on_completed(std::unique_ptr<Content> content) const
        {
            auto job = m_job.lock();
            if (!job)
            {
                return;
            }

            auto source = m_sources.get_source(m_source_key);
            auto& report_type = m_report_types.get_report_type(m_report_type_uuid);
            if (content)
            {
                m_collector.collect_report({ source, report_type, std::move(content), m_time }, m_transformation_circumstances);
            }

            m_conveyor.m_executive->run_processing(*this);           
        }

        void on_failed(const std::exception& err) const
        {
            if (auto job = m_job.lock())
            {
                job->emit_failed(err);
            }
        }

        void on_cancel() const
        {
            if (auto job = m_job.lock())
            {
                job->emit_completed();
            }
        }

    private:
        std::function<std::unique_ptr<Content>()> prepare_selection(const SourceResource& source_resource)
        {
            auto selection_activity = boost::get<SelectionActivity>(source_resource.m_activity);            
            auto selection = m_transformations.m_selections.get_selection({ 
                selection_activity.m_input_refs, 
                selection_activity.m_output_ref });

            std::vector<std::shared_ptr<Content>> inputs;
            for (auto& input_ref : selection->get_plan().m_input_refs)
            {
                auto input_report = m_collector.find_current_report(source_resource.m_source_ref, input_ref.m_report_type_ref);
                if (!input_report)
                {
                    if (!input_ref.is_strict())
                    {
                        inputs.push_back(nullptr);
                        continue;
                    }
                    return nullptr;
                }

                inputs.push_back(input_report->get_content());
                m_time = std::max(m_time, input_report->get_check_time());
            }

            return std::bind(selection->prepare_functor(source_resource.m_source_ref), std::move(inputs));
        }

        std::function<std::unique_ptr<Content>()> prepare_grouping(const SourceResource& source_resource)
        {
            auto grouping_activity = boost::get<GroupingActivity>(source_resource.m_activity);
            auto grouping = m_transformations.m_groupings.get_grouping({ grouping_activity.m_input_ref, grouping_activity.m_output_ref });

            std::vector<SourceRef> childs;
            std::vector<std::shared_ptr<Content>> inputs;

            for (auto& input : m_source_resources.get_inputs(source_resource))
            {
                if (auto input_report = m_collector.find_current_report(input.m_source_ref, boost::get<ReportTypeRef>(input.m_resource_ref)))
                {
                    inputs.push_back(input_report->get_content());
                    m_time = std::max(m_time, input_report->get_time());
                }
                else
                {
                    inputs.push_back(nullptr);
                }
                childs.push_back(input.m_source_ref);
            }

            return std::bind(grouping->prepare_functor(source_resource.m_source_ref, childs), std::move(inputs));
        }

    private:
        std::weak_ptr<TransformationJob> m_job;
        Conveyor& m_conveyor;
        Sources& m_sources;
        ReportTypes& m_report_types;
        Collector& m_collector;
        Transformations& m_transformations;
        SourceResources& m_source_resources;

        SourceKey m_source_key;
        ReportTypeUUID m_report_type_uuid;
        TransformationCircumstances m_transformation_circumstances;
        time_t m_time;
    };

    class ContinueStreamRoutine
    {
    public:
        ContinueStreamRoutine(std::shared_ptr<TransformationJob> job) :
            m_job(job),
            m_conveyor(job->m_conveyor),
            m_sources(*job->m_conveyor.m_sources),
            m_stream_types(job->m_conveyor.m_basis.m_stream_types),
            m_reservoir(*job->m_conveyor.m_reservoir),
            m_transformations(job->m_conveyor.m_basis.m_transformations),
            m_source_resources(*job->m_conveyor.m_source_resources),
            m_source_key(job->m_source_key),
            m_stream_type_uuid(boost::get<StreamTypeUUID>(job->m_resource_uuid)),
            m_input_key(0)
        {
        }

        std::function<std::vector<StreamMessage>()> prepare_functor()
        {
            auto job = m_job.lock();
            if (!job)
            {
                return nullptr;
            }

            auto source = m_sources.get_source(m_source_key);
            auto& stream_type = m_stream_types.get_stream_type(m_stream_type_uuid);
            auto source_resource = m_source_resources.get_source_resource(source, stream_type);

            if (!source_resource.m_significant)
            {
                return nullptr;
            }

            if (source_resource.m_activity.type() == typeid(ConvertionActivity))
            {
                return prepare_convertion(source_resource);
            }       

            else
            {
                //_ASSERT(false);
            }

            return nullptr;
        }

        void on_completed(std::vector<StreamMessage> stream_messages) const
        {
            auto job = m_job.lock();
            if (!job)
            {
                return;
            }

            Transaction t(job->m_conveyor.m_db);
            auto source = m_sources.get_source(m_source_key);
            auto& stream_type = m_stream_types.get_stream_type(m_stream_type_uuid);
            for (auto& message : stream_messages)
            {
                m_reservoir.append_record({ stream_type, source, message }, t);
            }
            m_reservoir.erase_input(source, stream_type, m_input_key, t);
            t.commit();

            m_conveyor.m_executive->run_processing(ContinueStreamRoutine(job));
        }

        void on_failed(const std::exception& err) const
        {
            if (auto job = m_job.lock())
            {
                job->emit_failed(err);
            }
        }

        void on_cancel() const
        {
            if (auto job = m_job.lock())
            {
                job->emit_completed();
            }
        }

    private:
        std::function<std::vector<StreamMessage>()> prepare_convertion(const SourceResource& source_resource)
        {
            auto convertion_activity = boost::get<ConvertionActivity>(source_resource.m_activity);
            auto convertion = m_transformations.m_convertions.get_convertion({ convertion_activity.m_input_ref, convertion_activity.m_output_ref });

            std::vector<StreamRecord> input_records;
            if (m_input_key = m_reservoir.read_input(source_resource.m_source_ref, boost::get<StreamTypeRef>(source_resource.m_resource_ref), input_records))
            {
                std::vector<StreamMessage> input;
                for (auto& input_record : input_records)
                {
                    if (input_record.m_stream_type_ref == convertion->get_direction().m_input_ref)
                    {
                        input.push_back(input_record.m_message);
                    }
                }

                return std::bind(convertion->prepare_functor(source_resource.m_source_ref), input);
            }

            return nullptr;         
        }       

    private:
        std::weak_ptr<TransformationJob> m_job;
        Conveyor& m_conveyor;
        Sources& m_sources;
        StreamTypes& m_stream_types;
        Reservoir& m_reservoir;
        Transformations& m_transformations;
        SourceResources& m_source_resources;

        SourceKey m_source_key;
        StreamTypeUUID m_stream_type_uuid;
        Key m_input_key;
    };

    TransformationJob(Conveyor& conveyor, SourceRef source_ref, ResourceRef resource_ref) :
        m_conveyor(conveyor),
        m_source_key(source_ref.get_key()),
        m_resource_uuid(get_uuid(resource_ref))
    {
    }

    void do_run()
    {
        if (m_resource_uuid.type() == typeid(ReportTypeUUID))
        {
            m_conveyor.m_executive->run_processing(RefreshReportRoutine(shared_from_this()));
        }
        else if (m_resource_uuid.type() == typeid(StreamTypeUUID))
        {
            m_conveyor.m_executive->run_processing(ContinueStreamRoutine(shared_from_this()));
        }
        else
        {
            emit_completed();
        }
    }

private:
    Conveyor& m_conveyor;
    SourceKey m_source_key;
    ResourceUUID m_resource_uuid;
};

class TrackingJob: public Job, public std::enable_shared_from_this<TrackingJob>
{
public:
    class Routine
    {
    public:
        Routine(std::shared_ptr<TrackingJob> job):
            m_job(job),
            m_sources(*job->m_conveyor.m_sources),
            m_report_types(job->m_conveyor.m_basis.m_report_types),
            m_stream_types(job->m_conveyor.m_basis.m_stream_types),
            m_collector(*job->m_conveyor.m_collector),
            m_reservoir(*job->m_conveyor.m_reservoir),
            m_transformations(job->m_conveyor.m_basis.m_transformations),
            m_source_resources(*job->m_conveyor.m_source_resources),
            m_source_key(job->m_source_key),
            m_input_uuid(job->m_input_uuid),
            m_output_uuid(job->m_output_uuid)
        {
        }

        std::function<std::vector<StreamMessage>()> prepare_functor()
        {
            auto job = m_job.lock();
            if (!job)
            {
                return nullptr;
            }           

            auto source = m_sources.get_source(m_source_key);           
            auto input_ref = m_report_types.get_report_type(m_input_uuid).get_ref();        
            auto output_ref = m_stream_types.get_stream_type(m_output_uuid).get_ref();

            if (!m_source_resources.get_source_resource(source, output_ref).m_significant)
            {
                return nullptr;
            }

            auto current_report = m_collector.find_current_report(source, input_ref);
            if (!current_report)
            {
                return nullptr;
            }

            auto tracking = m_transformations.m_trackings.get_tracking({input_ref, output_ref});
            return std::bind(tracking->prepare_functor(source), current_report->get_content());
        }

        void on_completed(std::vector<StreamMessage> messages) const
        {
            auto job = m_job.lock();
            if (!job)
            {
                return;
            }

            Transaction t(job->m_conveyor.m_db);
            auto source = m_sources.get_source(m_source_key);
            auto& output = m_stream_types.get_stream_type(m_output_uuid);
            for (auto& message : messages)
            {
                m_reservoir.append_record({output, source, message}, t);
            }
            t.commit();

            job->m_conveyor.m_executive->run_processing(*this, time(nullptr) + 60);
        }

        void on_failed(const std::exception& err) const
        {
            if (auto job = m_job.lock())
            {
                job->emit_failed(err);
            }           
        }

        void on_cancel() const
        {
            if (auto job = m_job.lock())
            {
                job->emit_completed();
            }
        }

    private:
        std::weak_ptr<TrackingJob> m_job;
        Sources& m_sources;
        ReportTypes& m_report_types;
        StreamTypes& m_stream_types;
        Collector& m_collector;
        Reservoir& m_reservoir;
        Transformations& m_transformations;
        SourceResources& m_source_resources;
        SourceKey m_source_key;
        ReportTypeUUID m_input_uuid;
        StreamTypeUUID m_output_uuid;
    };

    TrackingJob(Conveyor& conveyor, SourceRef source_ref, TrackingDirection direction):
        m_conveyor(conveyor),
        m_source_key(source_ref.get_key()),
        m_input_uuid(direction.m_input_ref.get_uuid()),
        m_output_uuid(direction.m_output_ref.get_uuid())
    {
    }

    void do_run()
    {
        m_conveyor.m_executive->run_processing(Routine(shared_from_this()));
    }

private:
    Conveyor& m_conveyor;
    SourceKey m_source_key;
    ReportTypeUUID m_input_uuid;
    StreamTypeUUID m_output_uuid;
};

class Conveyor::TransformManager
{
public:
    TransformManager(Conveyor& conveyor, Executive& executive):
        m_conveyor(conveyor),
        m_executive(executive)
    {
    }

    void run_transformation(SourceRef source_ref, ResourceRef output_ref)
    {
        auto strand = std::make_pair(source_ref.get_key(), get_uuid(output_ref));
        if (auto current_job = m_job_manager.current_job(strand))
        {
            if (typeid(*current_job) == typeid(TransformationJob))
            {
                return;
            }            
        }
        
        m_job_manager.clear_strand(strand);
        m_job_manager.run_job(strand, std::make_shared<TransformationJob>(m_conveyor, source_ref, output_ref));        
    }   

    void run_tracking(SourceRef source_ref, TrackingDirection direction)
    {
        auto strand = std::make_pair(source_ref.get_key(), direction.m_output_ref.get_uuid());
        if (auto current_job = m_job_manager.current_job(strand))
        {
            if (typeid(*current_job) == typeid(TrackingJob))
            {
                return;
            }           
        }

        m_job_manager.clear_strand(strand);
        m_job_manager.run_job(strand, std::make_shared<TrackingJob>(m_conveyor, source_ref, direction));
    }

    void run_refresh(SourceRef source_ref, ResourceRef resource_ref)
    {
        auto source_resource = m_conveyor.m_source_resources->find_source_resource(source_ref, resource_ref);
        if (!source_resource || !source_resource->m_significant)
        {
            return;
        }

        if (auto trackig_activity = boost::get<TrackingActivity>(&source_resource->m_activity))
        {
            run_tracking(source_ref, { trackig_activity->m_input_ref, trackig_activity->m_output_ref });
        }
        else
        {
            run_transformation(source_ref, resource_ref);
        }
    }

private:
    Conveyor& m_conveyor;
    Executive& m_executive;
    JobManager<std::pair<SourceKey, ResourceUUID>> m_job_manager;
};

Conveyor::Conveyor(Database& db, Basis& basis):
    m_db(db),
    m_basis(basis),
    m_transformations(m_basis.m_transformations),
    m_source_type_activities(m_basis.m_source_type_activities),
    m_sources(nullptr),
    m_links(nullptr),
    m_collector(nullptr),
    m_reservoir(nullptr),
    m_source_resources(nullptr),
    m_executive(nullptr),
    m_running(false)
{
    static_assert(Conveyor::CONVEYOR_PRIORITY > Basis::BASIS_PRIORITY, "Wrong component priority");
}

Conveyor::~Conveyor()
{
}

void Conveyor::set_sources(Sources* sources)
{
    static_assert(CONVEYOR_PRIORITY > Sources::SOURCES_PRIORITY, "Wrong component priority");
    _ASSERT(!m_sources);
    m_sources = sources;

    m_sources->connect_replace_source([this](SourceRef source_ref, SourceRef prev_parent_ref, SourceRef next_parent_ref, Transaction& t){
        _ASSERT(m_running);
        on_update_childs(prev_parent_ref, t);
        on_update_childs(next_parent_ref, t);
    }, CONVEYOR_PRIORITY);
}

void Conveyor::set_links(Links* links)
{
    links->connect_add_link([this](Link link, Transaction& t){
        _ASSERT(m_running);
        on_update_childs(link.m_parent_ref, t); 
    }, CONVEYOR_PRIORITY);

    links->connect_remove_link([this](Link link, Transaction& t){
        _ASSERT(m_running);
        on_update_childs(link.m_parent_ref, t);
    }, CONVEYOR_PRIORITY);
}

void Conveyor::set_collector(Collector* collector)
{
    static_assert(CONVEYOR_PRIORITY > Collector::COLLECTOR_PRIORITY, "Wrong component priority");
    
    _ASSERT(!m_collector);
    m_collector = collector;

    m_collector->connect_update_current_report([this](CurrentReportRef current_report_ref, bool content_changed, Transaction& t){
        _ASSERT(m_running);
        on_update_current_report(current_report_ref, content_changed, t);
    }, CONVEYOR_PRIORITY);
}

void Conveyor::set_reservoir(Reservoir* reservoir)
{
    static_assert(CONVEYOR_PRIORITY > Reservoir::RESERVOIR_PRIORITY, "Wrong component priority");

    _ASSERT(!m_reservoir);
    m_reservoir = reservoir;

    m_reservoir->connect_append_record([this](const StreamRecord& stream_record, Transaction& t){
        _ASSERT(m_running);
        on_append_record(stream_record, t);
    }, CONVEYOR_PRIORITY);
}

void Conveyor::set_source_resources(SourceResources* source_resources)
{
    static_assert(Conveyor::CONVEYOR_PRIORITY > SourceResources::SOURCE_RESOURCES_PRIORITY, "Wrong component priority");

    _ASSERT(!m_source_resources);
    m_source_resources = source_resources;

    m_source_resources->connect_update_source_resource([this](const SourceResource& source_resource, Transaction& t)
    {
        if (m_running)
        {
            on_update_source_resource(source_resource.m_source_ref, source_resource.m_resource_ref, t);
        }
    }, CONVEYOR_PRIORITY);
}

void Conveyor::run(Executive* executive, Transaction& t)
{
    _ASSERT(!m_executive);
    transact_assign(m_executive, executive, t);
    transact_assign(m_transform_manager, std::make_unique<TransformManager>(*this, *executive), t);
    transact_assign(m_running, true, t);

    t.connect_commit([this] {
        for (auto source : m_sources->get_sources())
        {
            for (auto source_resource : m_source_resources->get_source_resources(source))
            {
                m_transform_manager->run_refresh(source, source_resource.m_resource_ref);
            }
        }
    });
}

TransformationCircumstances Conveyor::get_transformation_circumstances(const SourceResource& source_resource) const
{
    std::vector<UUID> input_uuids;
    for (const auto& input_resource : m_source_resources->get_inputs(source_resource))
    {
        if (auto input_report_type_ref = boost::get<ReportTypeRef>(&input_resource.m_resource_ref))
        {
            if (auto input = m_collector->find_current_report(input_resource.m_source_ref, *input_report_type_ref))
            {
                input_uuids.push_back(input->get_uuid());
            }
        }
    }
    
    return {
        get_uuid(source_resource.m_activity),
        source_resource.m_setup_uuid,
        std::move(input_uuids)
    };
}

void Conveyor::on_update_current_report(CurrentReportRef current_report_ref, bool content_changed, Transaction& t)
{
    t.connect_commit([this, current_report_ref, content_changed]{
        auto source_ref = current_report_ref.get_source_ref();
        auto report_type_ref = current_report_ref.get_report_type_ref();

        auto source_resource = m_source_resources->find_source_resource(source_ref, report_type_ref);
        if (!source_resource)
        {
            return;
        }

        for (auto dependent_resource : m_source_resources->get_depends(*source_resource))
        {
            if (!dependent_resource.m_significant)
            {
                continue;
            }

            if (auto tracking_activity = boost::get<TrackingActivity>(&dependent_resource.m_activity))
            {
                TrackingDirection direction(tracking_activity->m_input_ref, tracking_activity->m_output_ref);
                m_transform_manager->run_tracking(source_ref, direction);
            }
            else
            {
                m_transform_manager->run_transformation(dependent_resource.m_source_ref, dependent_resource.m_resource_ref);
            }           
        }       
    });
}

void Conveyor::on_append_record(const StreamRecord& stream_record, Transaction& t)
{
    auto source_resource = m_source_resources->find_source_resource(stream_record.m_source_ref, stream_record.m_stream_type_ref);
    if (!source_resource)
    {
        return;
    }

    for (auto dependent_resource : m_source_resources->get_depends(*source_resource))
    {
        if (!dependent_resource.m_significant)
        {
            continue;
        }

        if (auto stream_type_ref = boost::get<StreamTypeRef>(&dependent_resource.m_resource_ref))
        {
            m_reservoir->append_input(dependent_resource.m_source_ref, *stream_type_ref, { stream_record }, t);
        }

        t.connect_commit([this, dependent_resource] {
            m_transform_manager->run_transformation(dependent_resource.m_source_ref, dependent_resource.m_resource_ref);
        });
    }
}

void Conveyor::on_update_source_resource(SourceRef source_ref, ResourceRef resource_ref, Transaction& t)
{
    t.connect_commit([this, source_key = source_ref.get_key(), resource_uuid = get_uuid(resource_ref)]{
        auto source_ref = m_sources->get_source(source_key);
        auto resource_ref = get_ref(m_basis, resource_uuid);
        m_transform_manager->run_refresh(source_ref, resource_ref);  
    });    
}

void Conveyor::on_update_childs(SourceRef source_ref, Transaction& t)
{
    t.connect_commit([this, source_ref]{
        for (auto& source_resource : m_source_resources->get_source_resources(source_ref))
        {
            if (!source_resource.m_significant)
            {
                continue;
            }

            if (auto grouping_activity = boost::get<GroupingActivity>(&source_resource.m_activity))
            {
                m_transform_manager->run_transformation(source_ref, grouping_activity->m_output_ref);
            }
        }
    });
}

}} //namespace TR { namespace Core {