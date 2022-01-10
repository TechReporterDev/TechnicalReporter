#include "stdafx.h"
#include "Tasks.h"
#include "CoreDomain.h"
namespace TR { namespace Core {

ContentQuery::ContentQuery(CoreDomain& core_domain, SourceKey source_key, ReportTypeUUID report_type_uuid, Executive& executive):
    Task(executive),
    m_source_key(source_key),
    m_report_type_uuid(report_type_uuid),
    m_core_domain(core_domain),
    m_sources(core_domain.m_registry.m_sources),
    m_report_types(core_domain.m_basis.m_report_types),
    m_source_resources(core_domain.m_registry.m_source_resources)
{
}

SourceKey ContentQuery::get_source_key() const
{
    return m_source_key;
}

ReportTypeUUID ContentQuery::get_report_type_uuid() const
{
    return m_report_type_uuid;
}

std::shared_ptr<Content> ContentQuery::get_content() const
{
    return m_content;
}

CurrentContentQuery::CurrentContentQuery(CoreDomain& core_domain, SourceKey source_key, ReportTypeUUID report_type_uuid, Executive& executive):
    ContentQuery(core_domain, source_key, report_type_uuid, executive),
    m_collector(core_domain.m_registry.m_collector)
{
}

void CurrentContentQuery::do_run()
{
    auto source = m_sources.get_source(m_source_key);
    auto& report_type = m_report_types.get_report_type(m_report_type_uuid);

    auto source_resource = m_source_resources.find_source_resource(source, report_type);
    if (!source_resource)
    {
        emit_completed();
        return;
    }

    if (source_resource->m_persistent)
    {
        if (auto current_report = m_collector.find_current_report(source, report_type))
        {
            m_content = current_report->get_content();
        }
        emit_completed();
        return;
    }

    std::shared_ptr<ContentQuery> content_query;
    if (auto selection_activity = boost::get<SelectionActivity>(&source_resource->m_activity))
    {
        content_query = std::make_shared<CurrentSelectionTask>(m_core_domain, m_source_key, m_report_type_uuid, m_executive);
    }

    else if (auto grouping_activity = boost::get<GroupingActivity>(&source_resource->m_activity))
    {
        content_query = std::make_shared<CurrentGroupingTask>(m_core_domain, m_source_key, m_report_type_uuid, m_executive);
    }

    else if (auto loading_activity = boost::get<LoadingActivity>(&source_resource->m_activity))
    {
        content_query = std::make_shared<ReloadReportTask>(m_core_domain, m_source_key, m_report_type_uuid, m_executive);
    }

    else if (auto shortcut_activity = boost::get<ShortcutActivity>(&source_resource->m_activity))
    {
        content_query = std::make_shared<LaunchShortcutTask>(m_core_domain, m_source_key, shortcut_activity->m_shortcut_ref.get_uuid(), m_executive);
    }

    else
    {
        throw Exception(L"Invalid activity type");
    }

    run_subtasks({content_query}, [this, content_query]{
        m_content = content_query->get_content();
        emit_completed();
    });
}

ArchivedContentQuery::ArchivedContentQuery(CoreDomain& core_domain, SourceKey source_key, ReportTypeUUID report_type_uuid, time_t time, Executive& executive):
    ContentQuery(core_domain, source_key, report_type_uuid, executive),
    m_time(time),
    m_archive(core_domain.m_services.m_archive)
{
}

void ArchivedContentQuery::do_run()
{
    auto source = m_sources.get_source(m_source_key);
    auto& report_type = m_report_types.get_report_type(m_report_type_uuid);
    
    auto source_resource = m_source_resources.find_source_resource(source, report_type);
    if (!source_resource)
    {
        emit_completed();
        return;
    }

    if (auto archived_report = m_archive.find_archived_report(source, report_type, m_time))
    {
        m_content = archived_report->get_content();
        emit_completed();
        return;
    }

    std::shared_ptr<ContentQuery> content_query;
    if (auto selection_activity = boost::get<SelectionActivity>(&source_resource->m_activity))
    {
        content_query = std::make_shared<ArchivedSelectionTask>(m_core_domain, m_source_key, m_report_type_uuid, m_time, m_executive);
    }

    else if (auto grouping_activity = boost::get<GroupingActivity>(&source_resource->m_activity))
    {
        content_query = std::make_shared<ArchivedGroupingTask>(m_core_domain, m_source_key, m_report_type_uuid, m_time, m_executive);
    }

    else if (auto loading_activity = boost::get<LoadingActivity>(&source_resource->m_activity))
    {
        emit_completed();
        return;
    }

    else if (auto shortcut_activity = boost::get<ShortcutActivity>(&source_resource->m_activity))
    {
        emit_completed();
        return;
    }

    else
    {
        throw Exception(L"Invalid activity type");
    }

    run_subtasks({content_query}, [this, content_query]{
        m_content = content_query->get_content();
        emit_completed();
    });
}

class SelectionTask::Routine
{
public:
    Routine(std::shared_ptr<SelectionTask> task):
        m_weak_ptr(task),
        m_sources(task->m_sources),
        m_report_types(task->m_report_types),
        m_source_resources(task->m_source_resources),
        m_selections(task->m_core_domain.m_basis.m_transformations.m_selections)
    {
    }

    std::function<std::unique_ptr<Content>()> prepare_functor()
    {
        auto task = m_weak_ptr.lock();
        if (!task)
        {
            return nullptr;
        }

        auto source = m_sources.get_source(task->m_source_key);
        auto& report_type = m_report_types.get_report_type(task->m_report_type_uuid);
        auto source_resource = m_source_resources.get_source_resource(source, report_type);
        auto selection_activity = boost::get<SelectionActivity>(source_resource.m_activity);

        auto selection = m_selections.find_selection({ selection_activity.m_input_refs, selection_activity.m_output_ref });       
        _ASSERT(selection);
        auto selection_plan = selection->get_plan();

        std::vector<std::shared_ptr<Content>> inputs;
        for (size_t position = 0; position < selection_plan.m_input_refs.size(); position++)
        {
            auto& subtask = task->subtasks().at(position);
            auto content_query = std::dynamic_pointer_cast<ContentQuery>(subtask);
            auto input = content_query->get_content();
            if (input == nullptr && selection_plan.m_input_refs.at(position).is_strict())
            {
                return nullptr;
            }
            inputs.push_back(input);
        }

        return std::bind(selection->prepare_functor(source), std::move(inputs));
    }

    void on_completed(std::unique_ptr<Content> content) const
    {
        if (auto task = m_weak_ptr.lock())
        {
            task->m_content = std::move(content);
            task->emit_completed();         
        }       
    }

    void on_failed(const std::exception& err) const
    {
        if (auto task = m_weak_ptr.lock())
        {
            task->emit_failed(err);
        }
    }

    void on_cancel() const
    {
        if (auto task = m_weak_ptr.lock())
        {
            task->emit_completed();
        }
    }

private:
    std::weak_ptr<SelectionTask> m_weak_ptr;

    Sources& m_sources;
    ReportTypes& m_report_types;
    SourceResources& m_source_resources;
    Selections& m_selections;
};

SelectionTask::SelectionTask(CoreDomain& core_domain, SourceKey source_key, ReportTypeUUID report_type_uuid, Executive& executive):
    ContentQuery(core_domain, source_key, report_type_uuid, executive)
{
}

CurrentSelectionTask::CurrentSelectionTask(CoreDomain& core_domain, SourceKey source_key, ReportTypeUUID report_type_uuid, Executive& executive):
    SelectionTask(core_domain, source_key, report_type_uuid, executive)
{
}

void CurrentSelectionTask::do_run()
{
    auto source = m_sources.get_source(m_source_key);
    auto& report_type = m_report_types.get_report_type(m_report_type_uuid);
    auto source_resource = m_source_resources.get_source_resource(source, report_type); 
    auto selection_activity = boost::get<SelectionActivity>(source_resource.m_activity);

    Subtasks subtasks;
    for (auto input_ref : selection_activity.m_input_refs)
    {
        subtasks.push_back(std::make_shared<CurrentContentQuery>(m_core_domain, m_source_key, input_ref.get_uuid(), m_executive));
    }

    run_subtasks(subtasks, [this]{
        m_executive.run_processing(Routine(shared_from_this()));
    });
}

ArchivedSelectionTask::ArchivedSelectionTask(CoreDomain& core_domain, SourceKey source_key, ReportTypeUUID report_type_uuid, time_t time, Executive& executive):
    SelectionTask(core_domain, source_key, report_type_uuid, executive),
    m_time(time)
{
}

void ArchivedSelectionTask::do_run()
{
    auto source = m_sources.get_source(m_source_key);
    auto& report_type = m_report_types.get_report_type(m_report_type_uuid);
    auto source_resource = m_source_resources.get_source_resource(source, report_type);
    auto selection_activity = boost::get<SelectionActivity>(source_resource.m_activity);

    Subtasks subtasks;
    for (auto input_ref : selection_activity.m_input_refs)
    {
        subtasks.push_back(std::make_shared<ArchivedContentQuery>(m_core_domain, m_source_key, input_ref.get_uuid(), m_time, m_executive));
    }

    run_subtasks(subtasks, [this]{
        m_executive.run_processing(Routine(shared_from_this()));
    });
}

class GroupingTask::Routine
{
public:
    Routine(std::shared_ptr<GroupingTask> task):
        m_task(task),
        m_sources(task->m_sources),
        m_report_types(task->m_report_types),
        m_source_resources(task->m_source_resources),
        m_groupings(task->m_core_domain.m_basis.m_transformations.m_groupings)
    {
    }

    std::function<std::unique_ptr<Content>()> prepare_functor()
    {
        auto source = m_sources.get_source(m_task->m_source_key);
        auto& report_type = m_report_types.get_report_type(m_task->m_report_type_uuid);
        auto source_resource = m_source_resources.get_source_resource(source, report_type);
        auto grouping_activity = boost::get<GroupingActivity>(source_resource.m_activity);

        auto grouping = m_groupings.find_grouping({grouping_activity.m_input_ref, grouping_activity.m_output_ref});
        _ASSERT(grouping);

        std::vector<SourceRef> source_refs;
        std::vector<std::shared_ptr<Content>> inputs;
        for (auto& subtask : m_task->subtasks())
        {
            auto content_query = std::dynamic_pointer_cast<ContentQuery>(subtask);
            source_refs.push_back(m_sources.get_source(content_query->get_source_key()));
            inputs.push_back(content_query->get_content());
        }

        return std::bind(grouping->prepare_functor(source, source_refs), std::move(inputs));
    }

    void on_completed(std::unique_ptr<Content> content) const
    {
        m_task->m_content = std::move(content);
        m_task->emit_completed();
    }

    void on_failed(const std::exception& err) const
    {
        m_task->emit_failed(err);
    }

    void on_cancel() const
    {
        _ASSERT(false);
    }

private:
    std::shared_ptr<GroupingTask> m_task;

    Sources& m_sources;
    ReportTypes& m_report_types;
    SourceResources& m_source_resources;
    Groupings& m_groupings;
};

GroupingTask::GroupingTask(CoreDomain& core_domain, SourceKey source_key, ReportTypeUUID report_type_uuid, Executive& executive):
    ContentQuery(core_domain, source_key, report_type_uuid, executive)
{
}


CurrentGroupingTask::CurrentGroupingTask(CoreDomain& core_domain, SourceKey source_key, ReportTypeUUID report_type_uuid, Executive& executive):
    GroupingTask(core_domain, source_key, report_type_uuid, executive)
{
}

void CurrentGroupingTask::do_run()
{
    auto source = m_sources.get_source(m_source_key);
    auto& report_type = m_report_types.get_report_type(m_report_type_uuid);
    auto source_resource = m_source_resources.get_source_resource(source, report_type);
    auto grouping_activity = boost::get<GroupingActivity>(source_resource.m_activity);

    Subtasks subtasks;
    for (auto& child : m_sources.get_sources(source))
    {
        subtasks.push_back(std::make_shared<CurrentContentQuery>(m_core_domain, child.get_ref().get_key(), grouping_activity.m_input_ref.get_uuid(), m_executive));
    }

    run_subtasks(subtasks, [this]{
        m_executive.run_processing(Routine(shared_from_this()));
    });
}

ArchivedGroupingTask::ArchivedGroupingTask(CoreDomain& core_domain, SourceKey source_key, ReportTypeUUID report_type_uuid, time_t time, Executive& executive):
    GroupingTask(core_domain, source_key, report_type_uuid, executive),
    m_time(time)
{
}

void ArchivedGroupingTask::do_run()
{
    auto source = m_sources.get_source(m_source_key);
    auto& report_type = m_report_types.get_report_type(m_report_type_uuid);
    auto source_resource = m_source_resources.get_source_resource(source, report_type);
    auto grouping_activity = boost::get<GroupingActivity>(source_resource.m_activity);
    
    Subtasks subtasks;
    for (auto& child : m_sources.get_sources(source))
    {
        subtasks.push_back(std::make_shared<ArchivedContentQuery>(m_core_domain, child.get_ref().get_key(), grouping_activity.m_input_ref.get_uuid(), m_time, m_executive));
    }

    run_subtasks(subtasks, [this]{
        m_executive.run_processing(Routine(shared_from_this()));
    });
}

StreamSegmentQuery::StreamSegmentQuery(CoreDomain& core_domain, SourceKey source_key, StreamTypeUUID stream_type_uuid, time_t start, time_t stop, Executive& executive):
    Task(executive),
    m_source_key(source_key),
    m_stream_type_uuid(stream_type_uuid),
    m_start(start),
    m_stop(stop),
    m_core_domain(core_domain)
{
}

SourceKey StreamSegmentQuery::get_source_key() const
{
    return m_source_key;
}

StreamTypeUUID StreamSegmentQuery::get_stream_type_uuid() const
{
    return m_stream_type_uuid;
}

const std::vector<StreamMessage>& StreamSegmentQuery::get_messages() const
{
    return m_messages;
}

ActualStreamQuery::ActualStreamQuery(CoreDomain& core_domain, SourceKey source_key, StreamTypeUUID stream_type_uuid, time_t start, time_t stop, Executive& executive):
    StreamSegmentQuery(core_domain, source_key, stream_type_uuid, start, stop, executive),
    m_reservoir(core_domain.m_registry.m_reservoir)
{
}

void ActualStreamQuery::do_run()
{
    auto source = m_core_domain.m_registry.m_sources.get_source(m_source_key);
    auto& stream_type = m_core_domain.m_basis.m_stream_types.get_stream_type(m_stream_type_uuid);
    
    auto source_resource = m_core_domain.m_registry.m_source_resources.find_source_resource(source, stream_type);
    if (!source_resource)
    {
        emit_completed();
        return;
    }

    if (source_resource->m_significant)
    {
        boost::copy(m_reservoir.read_stream(source, stream_type, m_start, m_stop) | stl_tools::members(&StreamRecord::m_message), std::back_inserter(m_messages));
        emit_completed();
        return;
    }

    if (auto convertionActivity = boost::get<ConvertionActivity>(&source_resource->m_activity))
    {
        auto convertion_task = std::make_shared<StreamConvertionTask>(m_core_domain, m_source_key, m_stream_type_uuid, m_start, m_stop, m_executive);
        run_subtasks({convertion_task}, [this, convertion_task]{
            m_messages = convertion_task->get_messages();
            emit_completed();
        });
    }

    else if (auto inflowActivity = boost::get<InflowActivity>(&source_resource->m_activity))
    {
        emit_completed();
    }

    else
    {
        throw Exception(L"Invalid activity type");
    }
}

class StreamConvertionTask::Routine
{
public:
    Routine(std::shared_ptr<StreamConvertionTask> stream_convertion_task):
        m_sources(stream_convertion_task->m_core_domain.m_registry.m_sources),
        m_stream_types(stream_convertion_task->m_core_domain.m_basis.m_stream_types),
        m_source_resources(stream_convertion_task->m_core_domain.m_registry.m_source_resources),
        m_convertions(stream_convertion_task->m_core_domain.m_basis.m_transformations.m_convertions),
        m_weak_ptr(stream_convertion_task)
    {
    }

    std::function<std::vector<StreamMessage>()> prepare_functor()
    {
        auto task = m_weak_ptr.lock();
        if (!task)
        {
            return nullptr;
        }

        auto source = m_sources.get_source(task->m_source_key);
        auto& stream_type = m_stream_types.get_stream_type(task->m_stream_type_uuid);
        auto source_resource = m_source_resources.get_source_resource(source, stream_type);     
        auto& convertion_activity = boost::get<ConvertionActivity>(source_resource.m_activity);
        
        auto convertion = m_convertions.get_convertion({convertion_activity.m_input_ref, convertion_activity.m_output_ref});
        auto input_query = std::dynamic_pointer_cast<StreamSegmentQuery>(task->subtasks().at(0));
        return std::bind(convertion->prepare_functor(source), std::move(input_query->get_messages()));
    }

    void on_completed(std::vector<StreamMessage> messages) const
    {       
        if (auto task = m_weak_ptr.lock())
        {
            task->m_messages = std::move(messages);
            task->emit_completed();
        }       
    }

    void on_failed(const std::exception& err) const
    {
        if (auto task = m_weak_ptr.lock())
        {
            task->emit_failed(err);
        }
    }

    void on_cancel() const
    {
    }

private:
    Sources& m_sources;
    StreamTypes& m_stream_types;
    SourceResources& m_source_resources;
    Convertions& m_convertions;

    std::weak_ptr<StreamConvertionTask> m_weak_ptr;
};

StreamConvertionTask::StreamConvertionTask(CoreDomain& core_domain, SourceKey source_key, StreamTypeUUID stream_type_uuid, time_t start, time_t stop, Executive& executive):
    StreamSegmentQuery(core_domain, source_key, stream_type_uuid, start, stop, executive)
{
}

void StreamConvertionTask::do_run()
{
    auto source = m_core_domain.m_registry.m_sources.get_source(m_source_key);
    auto& stream_type = m_core_domain.m_basis.m_stream_types.get_stream_type(m_stream_type_uuid);
    auto source_resource = m_core_domain.m_registry.m_source_resources.get_source_resource(source, stream_type);
    auto& convertion_activity = boost::get<ConvertionActivity>(source_resource.m_activity);

    auto input_query = std::make_shared<ActualStreamQuery>(m_core_domain, m_source_key, convertion_activity.m_input_ref.get_uuid(), m_start, m_stop, m_executive);
    run_subtasks({input_query}, [this]{
        m_executive.run_processing(Routine(shared_from_this()));
    });
}

class ReloadReportTask::Routine
{
public:
    Routine(std::shared_ptr<ReloadReportTask> reload_task):
        m_sources(reload_task->m_sources),
        m_report_types(reload_task->m_report_types),
        m_source_resources(reload_task->m_source_resources),
        m_collector(reload_task->m_core_domain.m_registry.m_collector),
        m_weak_ptr(reload_task)
    {
    }

    std::function<std::unique_ptr<Content>()> prepare_functor()
    {
        auto reload_task = m_weak_ptr.lock();
        if (!reload_task)
        {
            return nullptr;
        }

        auto source = m_sources.get_source(reload_task->m_source_key);
        auto& report_type = m_report_types.get_report_type(reload_task->m_report_type_uuid);
        auto& source_type = *source.get_source_type();

        auto download_functor = source_type.prepare_download(report_type, source);
        auto config = source.get_config();

        return  [download_functor, config]{
            return download_functor(*config);
        };
    }

    void on_completed(std::unique_ptr<Content> content) const
    {
        auto reload_task = m_weak_ptr.lock();
        if (!reload_task)
        {
            return;
        }

        reload_task->m_content = std::move(content);

        auto source = m_sources.get_source(reload_task->m_source_key);
        auto& report_type = m_report_types.get_report_type(reload_task->m_report_type_uuid);
        auto source_resource = m_source_resources.get_source_resource(source, report_type);
        if (source_resource.m_significant && reload_task->m_content)
        {
            m_collector.collect_report({source, report_type, reload_task->m_content, time(nullptr)});
        }

        reload_task->emit_completed();
    }

    void on_failed(const std::exception& err) const
    {
        auto reload_task = m_weak_ptr.lock();
        if (!reload_task)
        {
            return;
        }

        reload_task->emit_failed(err);
    }

    void on_cancel() const
    {
    }

private:
    Sources& m_sources;
    ReportTypes& m_report_types;
    SourceResources& m_source_resources;
    Collector& m_collector;

    std::weak_ptr<ReloadReportTask> m_weak_ptr;
};

ReloadReportTask::ReloadReportTask(CoreDomain& core_domain, SourceKey source_key, ReportTypeUUID report_type_uuid, Executive& executive):
    ContentQuery(core_domain, source_key, report_type_uuid, executive)
{
}

void ReloadReportTask::do_run()
{
    m_executive.run_io(m_source_key, Routine(shared_from_this()));
}

PrepareActionTask::PrepareActionTask(CoreDomain& core_domain, SourceKey source_key, ActionUUID action_uuid, Executive& executive):
    Task(executive),
    m_core_domain(core_domain),
    m_actions(core_domain.m_basis.m_actions),
    m_action_delegates(core_domain.m_basis.m_action_delegates),
    m_source_type_activities(core_domain.m_basis.m_source_type_activities),
    m_sources(core_domain.m_registry.m_sources),
    m_source_key(source_key),
    m_action_uuid(action_uuid)
{
}

PrepareActionTask::ActionFunctor PrepareActionTask::get_action_functor() const
{
    return m_action_functor;
}

void PrepareActionTask::do_run()
{
    auto source = m_sources.get_source(m_source_key);
    auto& action = m_actions.get_action(m_action_uuid);
    auto& source_type = *source.get_source_type();
    m_task_activity = m_source_type_activities.find_by_action(source_type, action);
        
    if (!m_task_activity)
    {
        emit_failed(std::logic_error("activity not found"));
        return;
    }       
        
    if (m_task_activity->type() == typeid(ActionActivity))
    {
        prepare_direct_action_functor();
        emit_completed();
        return;
    }

    else if (auto action_delegate_activity = boost::get<ActionDelegateActivity>(&*m_task_activity))
    {
        Subtasks subtasks;
        for (auto host_action_ref : action_delegate_activity->m_host_action_refs)
        {
            subtasks.push_back(std::make_shared<PrepareActionTask>(m_core_domain, m_source_key, host_action_ref.get_uuid(), m_executive));
        }

        for (auto input_ref : action_delegate_activity->m_input_refs)
        {
            subtasks.push_back(std::make_shared<CurrentContentQuery>(m_core_domain, m_source_key, input_ref.get_uuid(), m_executive));
        }

        run_subtasks(subtasks, [this]{
            prepare_delegate_action_functor();
            emit_completed();
        });
    }
    else
    {
        throw Exception(L"Invalid activity type");
    }
}

void PrepareActionTask::prepare_direct_action_functor()
{
    auto source = m_sources.get_source(m_source_key);
    auto& action = m_actions.get_action(m_action_uuid);
    auto& source_type = *source.get_source_type();
    auto activity = m_source_type_activities.find_by_action(source_type, action);
    if (activity != m_task_activity)
    {
        throw Exception(L"Task conditions failed");
    }

    auto action_functor = source_type.prepare_action(action, source);
    auto config = source.get_config();

    m_action_functor =  [action_functor, config](const XmlDoc& params){
        return action_functor(params, *config);
    };
}

void PrepareActionTask::prepare_delegate_action_functor()
{
    auto source = m_sources.get_source(m_source_key);
    auto& action = m_actions.get_action(m_action_uuid);
    auto& source_type = *source.get_source_type();
    auto activity = m_source_type_activities.find_by_action(source_type, action);
    if (activity != m_task_activity)
    {
        throw Exception(L"Task conditions failed");
    }

    auto action_delegate_activity = boost::get<ActionDelegateActivity>(*activity);
    auto& action_delegate = m_action_delegates.get_action_delegate({action_delegate_activity.m_host_action_refs, action_delegate_activity.m_guest_action_ref});
    auto action_delegate_functor = action_delegate.prepare_action(source);

    std::vector<ActionDelegate::HostActionFunctor> host_action_functors;
    std::vector<std::shared_ptr<Content>> inputs;
    for (auto& subtask : subtasks())
    {
        if (auto prepare_action_task = std::dynamic_pointer_cast<PrepareActionTask>(subtask))
        {
            host_action_functors.push_back(prepare_action_task->get_action_functor());
        }
        else if (auto content_query_task = std::dynamic_pointer_cast<CurrentContentQuery>(subtask))
        {
            inputs.push_back(content_query_task->get_content());                
        }
        else
        {
            _ASSERT(false);
        }
    }   

    m_action_functor = [action_delegate_functor, host_action_functors, inputs](const XmlDoc& params){
        return action_delegate_functor(params, inputs, host_action_functors);
    };
}

class LaunchActionTask::Routine
{
public:
    Routine(std::shared_ptr<LaunchActionTask> action_task):
        m_sources(action_task->m_core_domain.m_registry.m_sources),
        m_actions(action_task->m_core_domain.m_basis.m_actions),
        m_action_delegates(action_task->m_core_domain.m_basis.m_action_delegates),
        m_source_type_activities(action_task->m_core_domain.m_basis.m_source_type_activities),
        m_weak_ptr(action_task)
    {
    }

    std::function<std::shared_ptr<Content>(void)> prepare_functor()
    {
        auto action_task = m_weak_ptr.lock();
        if (!action_task)
        {
            return nullptr;
        }

        auto source = m_sources.get_source(action_task->m_source_key);
        auto& action = m_actions.get_action(action_task->m_action_uuid);
        auto action_functor = action_task->m_prepare_action_task->get_action_functor();
        auto params = action_task->m_params;

        return  [action_functor, params]{
            return action_functor(*params);
        };
    }

    void on_completed(std::shared_ptr<Content> output) const
    {
        if (auto action_task = m_weak_ptr.lock())
        {
            action_task->m_output = output;
            action_task->reload();
        }
    }

    void on_failed(const std::exception& err) const
    {
        if (auto action_task = m_weak_ptr.lock())
        {
            action_task->emit_failed(err);
        }
    }

    void on_cancel() const
    {
    }

private:
    Sources& m_sources;
    Actions& m_actions;
    ActionDelegates& m_action_delegates;
    SourceTypeActivities& m_source_type_activities;

    std::weak_ptr<LaunchActionTask> m_weak_ptr;
};

LaunchActionTask::LaunchActionTask(CoreDomain& core_domain, SourceKey source_key, ActionUUID action_uuid, std::unique_ptr<const XmlDoc> params, Executive& executive):
    Task(executive),
    m_source_key(source_key),
    m_action_uuid(action_uuid),
    m_params(std::move(params)),
    m_core_domain(core_domain),
    m_actions(core_domain.m_basis.m_actions),
    m_source_type_activities(core_domain.m_basis.m_source_type_activities),
    m_sources(core_domain.m_registry.m_sources),
    m_source_resources(core_domain.m_registry.m_source_resources)
{
}

std::shared_ptr<Content> LaunchActionTask::get_output() const
{
    return m_output;
}

void LaunchActionTask::do_run()
{
    m_prepare_action_task = std::make_shared<PrepareActionTask>(m_core_domain, m_source_key, m_action_uuid, m_executive);
    run_subtasks({m_prepare_action_task}, [this]{
        m_executive.run_io(m_source_key, Routine(shared_from_this()));
    }); 
}

void LaunchActionTask::reload()
{
    auto source = m_sources.get_source(m_source_key);
    auto& source_type = *source.get_source_type();
    auto& action = m_actions.get_action(m_action_uuid);

    Subtasks reloads;
    for (auto report_type_ref : m_source_type_activities.get_reloads(source_type, action))
    {
        auto source_resource = m_source_resources.get_source_resource(source, report_type_ref);
        if (source_resource.m_significant)
        {
            reloads.push_back(std::make_shared<ReloadReportTask>(m_core_domain, m_source_key, report_type_ref.get_uuid(), m_executive));
        }
    }

    run_subtasks(reloads, [this]{
        emit_completed();
    });
}

LaunchShortcutTask::LaunchShortcutTask(CoreDomain& core_domain, SourceKey source_key, UUID shortcut_uuid, Executive& executive):
    ContentQuery(core_domain, source_key, ReportTypeUUID(), executive),
    m_shortcuts(core_domain.m_basis.m_action_shortcuts),
    m_collector(core_domain.m_registry.m_collector),
    m_shortcut_uuid(shortcut_uuid)
{
}

std::shared_ptr<Content> LaunchShortcutTask::get_output() const
{
    return m_content;
}

UUID LaunchShortcutTask::get_shortcut_uuid() const
{
    return m_shortcut_uuid;
}

void LaunchShortcutTask::do_run()
{
    auto& shortcut = m_shortcuts.get_shortcut(m_shortcut_uuid);
    auto source = m_sources.get_source(m_source_key);
    m_action_task = std::make_shared<LaunchActionTask>(m_core_domain, m_source_key,
        shortcut.get_action_ref().get_uuid(),
        shortcut.get_action_params(source),
        m_executive);

    run_subtasks({m_action_task}, [this]{
        auto& shortcut = m_shortcuts.get_shortcut(m_shortcut_uuid);
        if (auto output_ref = shortcut.get_output_ref())
        {
            m_content = m_action_task->get_output();
            auto source = m_sources.get_source(m_source_key);
            if (m_source_resources.get_source_resource(source, *output_ref).m_significant && m_content)
            {
                m_collector.collect_report({source, *output_ref, m_content, time(nullptr)});
            }
        }
        emit_completed();
    });
}

RebuildReportTask::RebuildReportTask(CoreDomain& core_domain, SourceKey source_key, ReportTypeUUID report_type_uuid, Executive& executive):
    Task(executive),
    m_source_key(source_key),
    m_report_type_uuid(report_type_uuid),
    m_core_domain(core_domain),
    m_sources(core_domain.m_registry.m_sources),
    m_report_types(core_domain.m_basis.m_report_types),
    m_source_resources(core_domain.m_registry.m_source_resources)
{
}

SourceKey RebuildReportTask::get_source_key() const
{
    return m_source_key;
}
ReportTypeUUID RebuildReportTask::get_report_type_uuid() const
{
    return m_report_type_uuid;
}

void RebuildReportTask::do_run()
{
    auto source = m_sources.get_source(m_source_key);
    auto& report_type = m_report_types.get_report_type(m_report_type_uuid);
    auto source_resource = m_source_resources.get_source_resource(source, report_type);

    Subtasks subtasks;
    if (auto loading_activity = boost::get<LoadingActivity>(&source_resource.m_activity))
    {
        if (source_resource.m_significant)
        {
            subtasks.push_back(std::make_shared<ReloadReportTask>(m_core_domain, m_source_key, loading_activity->m_output_ref.get_uuid(), m_executive));
        }
    }
    else if (auto shortcut_activity = boost::get<ShortcutActivity>(&source_resource.m_activity))
    {
        if (source_resource.m_significant)
        {
            subtasks.push_back(std::make_shared<LaunchShortcutTask>(m_core_domain, m_source_key, shortcut_activity->m_shortcut_ref.get_uuid(), m_executive));
        }
    }
    else if (auto selection_activity = boost::get<SelectionActivity>(&source_resource.m_activity))
    {
        for (auto input_ref : selection_activity->m_input_refs)
        {
            subtasks.push_back(std::make_shared<RebuildReportTask>(m_core_domain, m_source_key, input_ref.get_uuid(), m_executive));
        }       
    }
    else if (auto grouping_activity = boost::get<GroupingActivity>(&source_resource.m_activity))
    {
        //Should reload all child sourcess?
    }   
    else
    {
        throw Exception(L"Invalid activity type");
    }

    run_subtasks(subtasks, [this]{
        emit_completed();
    });
}

}} //namespace TR { namespace Core {