#pragma once
#include "Activity.h"
#include "JobManager.h"
#include "AppDecl.h"
namespace TR {  namespace Core {
struct CoreImpl;

class Task: public Job
{
protected:
    using Subtasks = std::vector<std::shared_ptr<Task>>;    
    
    Task(Executive& executive):
        m_executive(executive)
    {
    }

    template<class CompleteHandler>
    void run_batch(const Subtasks& subtasks, Batch::Policy policy, CompleteHandler complete_handler)
    {
        _ASSERT(!m_subtasks || m_subtasks->get_state() != Task::State::STATE_RUNNING);

        m_subtasks = std::make_shared<Batch>(policy);
        for (auto subtask : subtasks)
        {
            m_subtasks->add_job(subtask);
        }

        m_subtasks->run(
            [this, complete_handler]{
                try
                {
                    complete_handler();
                }
                catch (const std::exception& err)
                {
                    emit_failed(err);
                }
            }, 
                
            [this](const std::exception& err){
                emit_failed(err);
            }
        );
    }

    template<class CompleteHandler>
    void run_subtasks(const Subtasks& subtasks, CompleteHandler complete_handler)
    {
        _ASSERT(!m_subtasks || m_subtasks->get_state() != Task::State::STATE_RUNNING);
        run_batch(subtasks, Batch::Policy::CONCURRENTLY, complete_handler);     
    }

    template<class CompleteHandler>
    void run_sequence(const Subtasks& subtasks, CompleteHandler complete_handler)
    {
        _ASSERT(!m_subtasks || m_subtasks->get_state() != Task::State::STATE_RUNNING);
        run_batch(subtasks, Batch::Policy::SEQUENTAL, complete_handler);
    }

    Subtasks subtasks()
    {
        Subtasks subtasks;
        _ASSERT(m_subtasks);
        for (auto subtask : *m_subtasks)
        {
            subtasks.push_back(std::dynamic_pointer_cast<Task>(subtask));
        }
        return subtasks;
    }

    virtual void do_cancel() override
    {
        if (m_subtasks && m_subtasks->get_state() == Task::State::STATE_RUNNING)
        {
            m_subtasks->cancel();
        }
    }

protected:
    Executive& m_executive;
    std::shared_ptr<Batch> m_subtasks;  
};

class ContentQuery: public Task
{
public:
    ContentQuery(CoreDomain& core_domain, SourceKey source_key, ReportTypeUUID report_type_uuid, Executive& executive);

    SourceKey                   get_source_key() const;
    ReportTypeUUID              get_report_type_uuid() const;
    std::shared_ptr<Content>    get_content() const;

protected:
    SourceKey m_source_key;
    ReportTypeUUID m_report_type_uuid;
    std::shared_ptr<Content> m_content;

    CoreDomain& m_core_domain;
    Sources& m_sources;
    ReportTypes& m_report_types;
    SourceResources& m_source_resources;
};

class CurrentContentQuery: public ContentQuery
{
public:
    CurrentContentQuery(CoreDomain& core_domain, SourceKey source_key, ReportTypeUUID report_type_uuid, Executive& executive);

protected:
    // Job override
    virtual void do_run() override;

private:
    Collector& m_collector;
};

class ArchivedContentQuery: public ContentQuery
{
public:
    ArchivedContentQuery(CoreDomain& core_domain, SourceKey source_key, ReportTypeUUID report_type_uuid, time_t time, Executive& executive);

protected:
    // Job override
    virtual void do_run() override;

private:
    time_t m_time;
    Archive& m_archive;
};

class SelectionTask: public ContentQuery, public std::enable_shared_from_this<SelectionTask>
{
public:
    class Routine;
    SelectionTask(CoreDomain& core_domain, SourceKey source_key, ReportTypeUUID report_type_uuid, Executive& executive);
};

class CurrentSelectionTask: public SelectionTask
{
public:
    CurrentSelectionTask(CoreDomain& core_domain, SourceKey source_key, ReportTypeUUID report_type_uuid, Executive& executive);

protected:
    // Job override
    virtual void do_run() override;
};

class ArchivedSelectionTask: public SelectionTask
{
public:
    ArchivedSelectionTask(CoreDomain& core_domain, SourceKey source_key, ReportTypeUUID report_type_uuid, time_t time, Executive& executive);

protected:
    // Job override
    virtual void do_run() override;

private:
    time_t m_time;
};

class GroupingTask: public ContentQuery, public std::enable_shared_from_this<GroupingTask>
{
public:
    class Routine;
    GroupingTask(CoreDomain& core_domain, SourceKey source_key, ReportTypeUUID report_type_uuid, Executive& executive);
};

class CurrentGroupingTask: public GroupingTask
{
public:
    CurrentGroupingTask(CoreDomain& core_domain, SourceKey source_key, ReportTypeUUID report_type_uuid, Executive& executive);

protected:
    // Job override
    virtual void do_run() override;
};

class ArchivedGroupingTask: public GroupingTask
{
public:
    ArchivedGroupingTask(CoreDomain& core_domain, SourceKey source_key, ReportTypeUUID report_type_uuid, time_t time, Executive& executive);

protected:
    // Job override
    virtual void do_run() override;

private:
    time_t m_time;
};

class StreamSegmentQuery: public Task
{
public:
    StreamSegmentQuery(CoreDomain& core_domain, SourceKey source_key, StreamTypeUUID stream_type_uuid, time_t start, time_t stop, Executive& executive);
    
    SourceKey                           get_source_key() const;
    StreamTypeUUID                      get_stream_type_uuid() const;
    const std::vector<StreamMessage>&   get_messages() const;

protected:
    SourceKey m_source_key;
    StreamTypeUUID m_stream_type_uuid;
    time_t m_start;
    time_t m_stop;
    std::vector<StreamMessage> m_messages;
    CoreDomain& m_core_domain;
};

class ActualStreamQuery: public StreamSegmentQuery
{
public:
    ActualStreamQuery(CoreDomain& core_domain, SourceKey source_key, StreamTypeUUID stream_type_uuid, time_t start, time_t stop, Executive& executive);

protected:
    // Job override
    virtual void do_run() override;

private:
    Reservoir& m_reservoir;
};

class StreamConvertionTask: public StreamSegmentQuery, public std::enable_shared_from_this<StreamConvertionTask>
{
public:
    class Routine;
    StreamConvertionTask(CoreDomain& core_domain, SourceKey source_key, StreamTypeUUID stream_type_uuid, time_t start, time_t stop, Executive& executive);

protected:
    // Job override
    virtual void do_run() override;
};

class ReloadReportTask: public ContentQuery, public std::enable_shared_from_this<ReloadReportTask>
{
public:
    class Routine;
    ReloadReportTask(CoreDomain& core_domain, SourceKey source_key, ReportTypeUUID report_type_uuid, Executive& executive);

protected:
    // Job override
    virtual void do_run() override;
};

class PrepareActionTask: public Task, public std::enable_shared_from_this<PrepareActionTask>
{
public:
    using ActionFunctor = std::function<std::unique_ptr<Content>(const XmlDoc& params)>;

    PrepareActionTask(CoreDomain& core_domain, SourceKey source_key, ActionUUID action_uuid, Executive& executive);
    ActionFunctor get_action_functor() const;

protected:
    // Job override
    virtual void do_run() override;

    void prepare_direct_action_functor();
    void prepare_delegate_action_functor();

private:
    CoreDomain& m_core_domain;
    Actions& m_actions;
    ActionDelegates& m_action_delegates;
    SourceTypeActivities& m_source_type_activities;
    Sources& m_sources;

    SourceKey m_source_key;
    ActionUUID m_action_uuid;

    boost::optional<Activity> m_task_activity;
    ActionFunctor m_action_functor;
};

class LaunchActionTask: public Task, public std::enable_shared_from_this<LaunchActionTask>
{
public:
    LaunchActionTask(CoreDomain& core_domain, SourceKey source_key, ActionUUID action_uuid, std::unique_ptr<const XmlDoc> params, Executive& executive);
    std::shared_ptr<Content> get_output() const;

protected:
    // Job override
    virtual void do_run() override;
    
    void reload();

protected:
    class Routine;
    CoreDomain& m_core_domain;
    Actions& m_actions;
    SourceTypeActivities& m_source_type_activities;
    Sources& m_sources;
    SourceResources& m_source_resources;

    SourceKey m_source_key;
    ActionUUID m_action_uuid;
    std::shared_ptr<const XmlDoc> m_params;
    std::shared_ptr<PrepareActionTask> m_prepare_action_task;
    std::shared_ptr<Content> m_output;
};

class LaunchShortcutTask: public ContentQuery, public std::enable_shared_from_this<LaunchShortcutTask>
{
public:
    LaunchShortcutTask(CoreDomain& core_domain, SourceKey source_key, UUID shortcut_uuid, Executive& executive);
    std::shared_ptr<Content>    get_output() const;
    UUID                        get_shortcut_uuid() const;

protected:
    // Job override
    virtual void do_run() override;

protected:
    ActionShortcuts& m_shortcuts;
    Collector& m_collector;

    UUID m_shortcut_uuid;
    std::shared_ptr<LaunchActionTask> m_action_task;
};

class RebuildReportTask: public Task, public std::enable_shared_from_this<RebuildReportTask>
{
public:
    RebuildReportTask(CoreDomain& core_domain, SourceKey source_key, ReportTypeUUID report_type_uuid, Executive& executive);
    SourceKey                   get_source_key() const;
    ReportTypeUUID              get_report_type_uuid() const;

protected:
    // Job override
    virtual void do_run() override;

protected:
    SourceKey m_source_key;
    ReportTypeUUID m_report_type_uuid;
    CoreDomain& m_core_domain;
    Sources& m_sources;
    ReportTypes& m_report_types;
    SourceResources& m_source_resources;
    std::shared_ptr<Job> m_delegate_task;
};

}} //namespace TR { namespace Core {