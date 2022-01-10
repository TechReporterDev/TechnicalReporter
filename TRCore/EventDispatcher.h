#pragma once
#include "PlainTypes.h"
namespace TR { namespace Core {

class EventDispatcher
{
public:
    virtual ~EventDispatcher(){}

    virtual void    on_add_source(const std::vector<Key>& target_sessions, const SourceInfo& source_info){}
    virtual void    on_update_source(const std::vector<Key>& target_sessions, const SourceInfo& source_info){}
    virtual void    on_remove_source(const std::vector<Key>& target_sessions, Key source_key, const std::vector<SourceKey>& removed_sources){}
    virtual void    on_add_link(const std::vector<Key>& target_sessions, TR::Link link){}
    virtual void    on_remove_link(const std::vector<Key>& target_sessions, TR::Link link){}
    virtual void    on_update_current_report(const std::vector<Key>& target_sessions, const CurrentReportInfo& current_report_info, bool content_changed){}

    //task events
    virtual void    on_query_content_completed(Key session_key, int id, std::shared_ptr<Content> content){};
    virtual void    on_query_stream_completed(Key session_key, int id, const StreamSegment& stream_segment){};
    virtual void    on_action_completed(Key session_key, int id, std::shared_ptr<Content> output){};
    virtual void    on_reload_completed(Key session_key, int id, std::shared_ptr<Content> output){};
    virtual void    on_rebuild_completed(Key session_key, int id){};
    virtual void    on_export_completed(Key session_key, int id, std::shared_ptr<Content> output) {};
    virtual void    on_job_failed(Key session_key, int id, const std::exception& err){};
};

}} //namespace TR { namespace Core {