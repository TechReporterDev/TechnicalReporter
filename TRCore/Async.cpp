#include "stdafx.h"
#include "Async.h"
#include "CoreImpl.h"
#include "Tasks.h"
#include "ExportTasks.h"
namespace TR { namespace Core {

Async::Async(CoreImpl& core_impl):
    m_core_impl(core_impl),
    m_sources(core_impl.m_core_domain.m_registry.m_sources),
    m_source_resources(core_impl.m_core_domain.m_registry.m_source_resources)
{
    m_core_impl.m_security.connect_open_session([this](Key session_key) {
        on_open_session(session_key);
    }, ASYNC_PRIORITY);

    m_core_impl.m_security.connect_close_session([this](Key session_key) {
        on_close_session(session_key);
    }, ASYNC_PRIORITY);
}

void Async::run(std::unique_ptr<Executive> executive, Transaction& t)
{
    transact_assign(m_executive, std::move(executive), t);
}

void Async::stop()
{
    m_executive->m_io->stop();
    m_executive->m_processing->stop();
    m_executive->m_control->stop();
}

void Async::connect_completed(CompletedHandler completed_handler)
{
    _ASSERT(!m_completed_handler);
    m_completed_handler = completed_handler;
}

void Async::connect_failed(FailedHandler failed_handler)
{
    _ASSERT(!m_failed_handler);
    m_failed_handler = failed_handler;
}

int Async::query_current_content(Key session_key, SourceKey source_key, ReportTypeUUID report_type_uuid)
{
    auto& workspace = get_workspace(session_key);
    auto query = std::make_shared<CurrentContentQuery>(m_core_impl.m_core_domain, source_key, report_type_uuid, *workspace.m_executive);
    workspace.m_job_manager.run_job(0, query);
    return query->get_id();
}

int Async::query_archived_content(Key session_key, SourceKey source_key, ReportTypeUUID report_type_uuid, time_t time)
{
    auto& workspace = get_workspace(session_key);
    auto query = std::make_shared<ArchivedContentQuery>(m_core_impl.m_core_domain, SourceKey(source_key), ReportTypeUUID(report_type_uuid), time, *workspace.m_executive);
    workspace.m_job_manager.run_job(0, query);
    return query->get_id();
}

int Async::query_stream_segment(Key session_key, SourceKey source_key, StreamTypeUUID stream_type_uuid, time_t start, time_t stop)
{
    auto& workspace = get_workspace(session_key);
    auto query = std::make_shared<ActualStreamQuery>(m_core_impl.m_core_domain, source_key, stream_type_uuid, start, stop, *workspace.m_executive);
    workspace.m_job_manager.run_job(0, query);
    return query->get_id();
}

int Async::launch_action(Key session_key, SourceKey source_key, ActionUUID action_uuid, std::unique_ptr<const XmlDoc> params)
{
    auto& workspace = get_workspace(session_key);
    auto action_task = std::make_shared<LaunchActionTask>(m_core_impl.m_core_domain, source_key, action_uuid, std::move(params), *workspace.m_executive);
    workspace.m_job_manager.run_job(0, action_task);
    return action_task->get_id();
}

int Async::launch_shortcut(Key session_key, SourceKey source_key, UUID shortcut_uuid)
{
    auto& workspace = get_workspace(session_key);
    auto shortcut_task = std::make_shared<LaunchShortcutTask>(m_core_impl.m_core_domain, source_key, shortcut_uuid, *workspace.m_executive);
    workspace.m_job_manager.run_job(0, shortcut_task);
    return shortcut_task->get_id();
}

int Async::reload(Key session_key, SourceKey source_key)
{
    auto& workspace = get_workspace(session_key);
    auto reload_batch = std::make_shared<Batch>();
    auto source = m_sources.get_source(source_key);
    for (auto& source_resource : m_source_resources.get_source_resources(source))
    {       
        if (!source_resource.m_significant)
        {
            continue;
        }

        if (auto loading_activity = boost::get<LoadingActivity>(&source_resource.m_activity))
        {
            reload_batch->add_job(std::make_shared<ReloadReportTask>(m_core_impl.m_core_domain, source_key, loading_activity->m_output_ref.get_uuid(), *workspace.m_executive));
        }
    }

    workspace.m_job_manager.run_job(0, reload_batch);
    return reload_batch->get_id();
}

int Async::reload_report(Key session_key, SourceKey source_key, ReportTypeUUID report_type_uuid)
{
    auto& workspace = get_workspace(session_key);
    auto reload_task = std::make_shared<ReloadReportTask>(m_core_impl.m_core_domain, source_key, report_type_uuid, *workspace.m_executive);
    workspace.m_job_manager.run_job(0, reload_task);
    return reload_task->get_id();
}

int Async::rebuild_report(Key session_key, SourceKey source_key, ReportTypeUUID report_type_uuid)
{
    auto& workspace = get_workspace(session_key);
    auto rebuild_task = std::make_shared<RebuildReportTask>(m_core_impl.m_core_domain, source_key, report_type_uuid, *workspace.m_executive);
    workspace.m_job_manager.run_job(0, rebuild_task);
    return rebuild_task->get_id();
}

int Async::export_summary(Key session_key)
{
    auto& workspace = get_workspace(session_key);
    auto session = m_core_impl.m_security.get_session(session_key);
    auto export_task = std::make_shared<ExportSummary>(m_core_impl.m_core_domain, m_core_impl.m_scheduler, session.m_user_key, *workspace.m_executive);
    workspace.m_job_manager.run_job(0, export_task);
    return export_task->get_id();
}

void Async::cancel(Key session_key, int job_id)
{
    get_workspace(session_key).m_job_manager.cancel_job(job_id);
}

Async::Workspace& Async::get_workspace(Key session_key)
{
    auto position = m_workspaces.find(session_key);
    if (position == m_workspaces.end())
    {
        throw Exception(L"Invalid session key");
    }

    return position->second;
}

void Async::on_open_session(Key session_key)
{
    _ASSERT(m_workspaces.find(session_key) == m_workspaces.end());

    m_workspaces.insert({
        session_key,
        Workspace{
            std::make_unique<Executive>(
                m_executive->m_control,
                std::make_unique<ExpandingExecutor>(*m_executive->m_processing),
                std::make_unique<ExpandingExecutor2>(std::vector<Executor*>{
                    m_core_impl.m_core_domain.m_registry.m_collector_executive->m_processing.get(),
                    m_executive->m_io.get() })),
            JobManager<int>(
                [session_key, this](int, std::shared_ptr<Job> job) {
                    m_completed_handler(session_key, job);
                },
                [session_key, this](int, std::shared_ptr<Job> job, const std::exception& err) {
                        m_failed_handler(session_key, job, err);
                })
        }});
}

void Async::on_close_session(Key session_key)
{
    _ASSERT(m_workspaces.find(session_key) != m_workspaces.end());
    m_workspaces.erase(session_key);
}
    
}} //namespace TR { namespace Core {