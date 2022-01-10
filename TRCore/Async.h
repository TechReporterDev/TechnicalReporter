#pragma once
#include "AppDecl.h"
#include "JobManager.h"
#include "Executive.h"
namespace TR { namespace Core {
struct CoreImpl;
class Security;

class Async
{
public:
    static const Priority ASYNC_PRIORITY = 5000;

    using CompletedHandler = std::function<void(Key, std::shared_ptr<Job>)>;
    using FailedHandler = std::function<void(Key, std::shared_ptr<Job>, const std::exception&)>;

    Async(CoreImpl& core_impl);

    void        run(std::unique_ptr<Executive> executive, Transaction& t);
    void        stop();

    void        connect_completed(CompletedHandler completed_handler);
    void        connect_failed(FailedHandler failed_handler);

    int         query_current_content(Key session_key, SourceKey source_key, ReportTypeUUID report_type_uuid);
    int         query_archived_content(Key session_key, SourceKey source_key, ReportTypeUUID report_type_uuid, time_t time);
    int         query_stream_segment(Key session_key, SourceKey source_key, StreamTypeUUID stream_type_uuid, time_t start, time_t stop);
    int         launch_action(Key session_key, SourceKey source_key, ActionUUID action_uuid, std::unique_ptr<const XmlDoc> params);
    int         launch_shortcut(Key session_key, SourceKey source_key, UUID shortcut_uuid);
    int         reload(Key session_key, SourceKey source_key);
    int         reload_report(Key session_key, SourceKey source_key, ReportTypeUUID report_type_uuid);
    int         rebuild_report(Key session_key, SourceKey source_key, ReportTypeUUID report_type_uuid);
    int         export_summary(Key session_key);
    void        cancel(Key session_key, int job_id);

private:
    struct Workspace
    {
        std::unique_ptr<Executive> m_executive;
        JobManager<int> m_job_manager;
    };

    Workspace&  get_workspace(Key session_key);
    void        on_open_session(Key session_key);
    void        on_close_session(Key session_key);

    CoreImpl& m_core_impl;
    Sources& m_sources;
    SourceResources &m_source_resources;
    std::map<Key, Workspace> m_workspaces;
    CompletedHandler m_completed_handler;
    FailedHandler m_failed_handler;

    std::unique_ptr<Executive> m_executive;
};

}} //namespace TR { namespace Core {