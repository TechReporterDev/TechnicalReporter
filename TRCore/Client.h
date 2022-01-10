#pragma once
#include "PlainTypes.h"
#include "stddecl.h"
namespace TR {
class Client;
class JobHandler;

class EventHandler
{
public:
    friend class Client;

    EventHandler();
    EventHandler(Client* client);
    EventHandler(const EventHandler&) = delete;
    EventHandler(EventHandler&&) = delete;
    virtual ~EventHandler();

    EventHandler& operator = (const EventHandler&) = delete;
    EventHandler& operator = (EventHandler&&) = delete;

    virtual void on_add_source(const SourceInfo& source_info){}
    virtual void on_update_source(const SourceInfo& source_info){}
    virtual void on_remove_source(Key source_key, const std::vector<SourceKey>& removed_sources){}
    virtual void on_add_link(TR::Link link){}
    virtual void on_remove_link(TR::Link link){}
    virtual void on_update_current_report(const CurrentReportInfo& current_report_info, bool content_changed){}

protected:
    Client* m_client;
};

class Client
{
public:
    using PackedEvent = std::function<void()>;
    using EventTransporter = std::function<void(PackedEvent)>;
    using JobFailedHandler = std::function<void(const std::string&)>;
    using JobCompletedHandler = std::function<void(void)>;
    using ContentReadyHandler = std::function<void(std::shared_ptr<Content>)>;
    using StreamReadyHandler = std::function<void(StreamSegment)>;
    
    class Job
    {
    public:
        Job();
        Job(const Client*client, int id);
        ~Job();

        Job(const Job&) = delete;
        Job(Job&& job);

        Job& operator = (const Job&) = delete;
        Job& operator = (Job&& job);

        int     get_id() const;
        bool    is_running() const;
        void    release();
        void    cancel();       

    private:
        const Client* m_client;
        int m_id;
    };

    Client();
    Client(EventTransporter event_transporter);
    Client(const Client&) = delete;
    Client(Client&&) = delete;
    virtual ~Client();

    Client& operator = (const Client&) = delete;
    Client& operator = (Client&&) = delete;
            
    virtual Key                                 get_session_key() const;
    virtual void                                add_event_handler(EventHandler* event_handler);
    virtual void                                remove_event_handler(EventHandler* event_handler);  
    
    // security
    virtual std::vector<UserInfo>               get_users() const = 0;
    virtual Key                                 add_user(const UserSettings& user_settings) = 0;
    virtual void                                update_user(Key user_key, const UserSettings& user_settings) = 0;
    virtual void                                remove_user(Key user_key) = 0;
    virtual SourceAccessInfo                    get_access_info(SourceKey source_key) const = 0;
    virtual std::vector<AccessEntry>            get_access_entries(SourceKey source_key) const = 0;
    virtual void                                set_access_list(const SourceAccessList& source_access_list) const = 0;

    // summary info
    virtual SummaryInfo                         get_summary_info() const = 0;

    // report type info
    virtual std::vector<ReportTypeInfo>         get_report_types_info() const = 0;
    virtual ReportTypeInfo                      get_report_type_info(ReportTypeUUID report_type_uuid) const = 0;
    virtual std::shared_ptr<const XmlDoc>       get_content_def(ReportTypeUUID report_type_uuid) const = 0;

    // stream type info
    virtual std::vector<StreamTypeInfo>         get_stream_types_info() const = 0;
    virtual StreamTypeInfo                      get_stream_type_info(StreamTypeUUID stream_type_uuid) const = 0;
    virtual std::shared_ptr<const XmlDefDoc>    get_message_def(StreamTypeUUID stream_type_uuid) const = 0;

    // action info
    virtual std::vector<ActionInfo>             get_actions_info() const = 0;
    virtual ActionInfo                          get_action_info(ActionUUID action_uuid) const = 0;
    virtual Params                              get_default_params(ActionUUID action_uuid) const = 0;
    virtual Params                              create_params(ActionUUID action_uuid, const std::string& params_xml) const = 0;

    // source type info
    virtual std::vector<SourceTypeInfo>         get_source_types_info() const = 0;
    virtual SourceTypeInfo                      get_source_type_info(UUID source_type_uuid) const = 0;
    virtual std::shared_ptr<const XmlDefDoc>    get_config_def(UUID source_type_uuid) const = 0;
    virtual SourceConfig                        get_default_config(UUID source_type_uuid) const = 0;

    // roles info
    virtual std::vector<RoleInfo>               get_roles_info() const = 0;
    virtual RoleInfo                            get_role_info(RoleKey role_key) const = 0;
    virtual RoleKey                             add_role(const RoleSettings& role_settings) const = 0;
    virtual void                                update_role(RoleKey role_key, const RoleSettings& role_settings) const = 0;
    virtual void                                remove_role(RoleKey role_key) = 0;

    // source tree access
    virtual std::vector<SourceInfo>             get_sources_info() const = 0;
    virtual SourceInfo                          get_source_info(SourceKey source_key) const = 0;
    virtual SourceKey                           add_source(const SourceSettings& source_settings) = 0;
    virtual void                                update_source(SourceKey source_key, const SourceSettings& source_settings) = 0;
    virtual void                                remove_source(SourceKey source_key) = 0;

    virtual SourceConfig                        get_source_config(SourceKey source_key) const = 0;
    virtual void                                set_source_config(SourceKey source_key, SourceConfig config) = 0;

    // source link
    virtual std::vector<Link>                   get_links() const = 0;
    virtual void                                add_link(Link link) = 0;
    virtual void                                remove_link(Link link) = 0;

    // report info
    virtual CurrentReportInfo                   get_current_report(SourceKey source_key, ReportTypeUUID report_type_uuid) const = 0;
    virtual std::shared_ptr<Content>            get_current_content(SourceKey source_key, ReportTypeUUID report_type_uuid) const = 0;
    virtual Job                                 query_current_content(SourceKey source_key, ReportTypeUUID report_type_uuid, ContentReadyHandler completed_handler, JobFailedHandler failed_handler) const;

    // archive
    virtual std::vector<ArchivedReportInfo>     get_archive(SourceKey source_key) const = 0;
    virtual ArchivedReportInfo                  get_archived_report(ArchivedReportKey archived_report_key) const = 0;
    virtual std::shared_ptr<Content>            get_archived_content(ArchivedReportKey archived_report_key) const = 0;
    virtual Job                                 query_archived_content(SourceKey source_key, ReportTypeUUID report_type_uuid, time_t time, ContentReadyHandler completed_handler, JobFailedHandler failed_handler) const;

    // stream
    virtual StreamInfo                          get_stream_info(SourceKey source_key, StreamTypeUUID stream_type_uuid) const = 0;
    virtual Job                                 query_stream_segment(SourceKey source_key, StreamTypeUUID stream_type_uuid, time_t start, time_t stop, StreamReadyHandler completed_handler, JobFailedHandler failed_handler) const;

    // options
    virtual SubjectReportOptions                get_options(SubjectKey subject_key, ReportTypeUUID report_type_uuid) const = 0;
    virtual SubjectStreamOptions                get_options(SubjectKey subject_key, StreamTypeUUID stream_type_uuid) const = 0;
    virtual SubjectActionOptions                get_options(SubjectKey subject_key, ActionUUID action_uuid, boost::optional<UUID> shortcut_uuid) const = 0;
    virtual void                                set_options(SubjectKey subject_key, const ReportOptions& report_options) = 0;
    virtual void                                set_options(SubjectKey subject_key, const StreamOptions& stream_options) = 0;
    virtual void                                set_options(SubjectKey subject_key, const ActionOptions& shortcut_options) = 0;

    virtual SubjectOptions                      get_subject_options(SubjectKey subject_key) const = 0;
    virtual ReportTypeOptions                   get_report_type_options(ReportTypeUUID report_type_uuid) const = 0;
    virtual StreamTypeOptions                   get_stream_type_options(StreamTypeUUID stream_type_uuid) const = 0;
    virtual ActionTypeOptions                   get_action_options(ActionUUID action_uuid, boost::optional<UUID> shortcut_uuid) const = 0;
    
    // custom query
    virtual CustomQueryInfo                     get_custom_query_info(Key custom_query_key) const = 0;
    virtual CustomQueryDefinition               get_custom_query_def(Key custom_query_key) const = 0;
    virtual CustomQueryDefinition               create_custom_query(std::vector<ReportTypeUUID> input_uuids) const = 0;
    virtual Key                                 add_custom_query(std::wstring name, CustomQueryDefinition custom_query_def) = 0;
    virtual void                                remove_custom_query(Key custom_query_key) = 0;

    //custom filter
    virtual CustomFilterInfo                    get_custom_filter_info(Key custom_filter_key) const = 0;
    virtual Key                                 add_custom_filter(std::wstring name, ReportTypeUUID input_uuid, CustomFilterSetup custom_filter_setup) = 0;
    virtual CustomFilterSetup                   create_filter_setup(ReportTypeUUID input_uuid) const = 0;
    virtual CustomFilterSetup                   get_filter_setup(Key custom_filter_key) const = 0;
    virtual void                                set_filter_setup(Key custom_filter_key, CustomFilterSetup custom_filter_setup) const = 0;

    //tracking stream
    virtual TrackingStreamInfo                  get_tracking_stream_info(Key tracking_stream_key) const = 0;
    virtual Key                                 add_tracking_stream(std::wstring name, ReportTypeUUID input_uuid, std::string input_xpath) = 0;

    //stream filter
    virtual StreamFilterInfo                    get_stream_filter_info(Key stream_filter_key) const = 0;
    virtual StreamFilterSetup                   create_filter_setup(StreamTypeUUID input_uuid) const = 0;
    virtual Key                                 add_stream_filter(std::wstring name, StreamTypeUUID  input_uuid, StreamFilterSetup stream_filter_setup) = 0;
    virtual StreamFilterSetup                   get_stream_filter_setup(Key stream_filter_key) const = 0;
    virtual void                                set_stream_filter_setup(Key stream_filter_key, StreamFilterSetup stream_filter_setup) = 0;

    // compliance
    virtual std::vector<ComplianceInfo>         get_compliances_info() const = 0;
    virtual ComplianceInfo                      get_compliance_info(ComplianceKey compliance_key) const = 0;
    virtual ComplianceSetup                     get_compliance_setup(SubjectKey subject_key, ComplianceKey compliance_key) const = 0;
    virtual void                                set_compliance_setup(SubjectKey subject_key, ComplianceKey compliance_key, ComplianceSetup compliance_setup) = 0;

    // validation               
    virtual ValidationInfo                      get_validation_info(ValidationKey validation_key) const = 0;
    virtual ValidationKey                       add_validation(std::wstring name, ReportTypeUUID input_uuid, ValidationSettings default_settings) const = 0;
    virtual ValidationSettings                  create_validation_settings(ReportTypeUUID input_uuid) const = 0;
    virtual ValidationSettings                  get_validation_settings(SubjectKey subject_key, ValidationKey validation_key) const = 0;
    virtual void                                set_validation_settings(SubjectKey subject_key, ValidationKey validation_key, ValidationSettings validation_settings) = 0;

    // grouping             
    virtual GroupingInfo                        get_grouping_info(Key grouping_key) const = 0;
    virtual Key                                 add_grouping(std::wstring name, ReportTypeUUID input_uuid) = 0;
    
    //custom report 
    virtual void                                remove_custom_report(ReportTypeUUID report_type_uuid, bool recursive) = 0;

    // integrity checking 
    virtual std::shared_ptr<Content>            create_pattern(ReportTypeUUID report_type_uuid) const = 0;
    virtual std::shared_ptr<Content>            create_pattern(ReportTypeUUID report_type_uuid, Blob blob) const = 0;
    virtual std::shared_ptr<Content>            get_pattern(SubjectKey subject_key, ReportTypeUUID report_type_uuid) const = 0;
    virtual std::shared_ptr<Content>            get_effective_pattern(SubjectKey subject_key, ReportTypeUUID report_type_uuid) const = 0;
    virtual void                                set_pattern(SubjectKey subject_key, ReportTypeUUID report_type_uuid, Blob blob) = 0;
    virtual void                                reset_pattern(SubjectKey subject_key, ReportTypeUUID report_type_uuid) = 0; 

    // custom actions
    virtual UUID                                add_custom_action(std::wstring name, ActionUUID action_uuid, Params params) = 0;
    virtual void                                remove_custom_action(UUID custom_action_uuid) = 0;

    // scheduler
    virtual std::vector<ScheduleInfo>           get_schedules_info() const = 0;
    virtual ScheduleInfo                        get_schedule_info(Key schedule_key) const = 0;
    virtual Key                                 add_schedule(const ScheduleSettings& schedule_settings) = 0;
    virtual void                                update_schedule(Key schedule_key, const ScheduleSettings& schedule_settings) = 0;
    virtual void                                remove_schedule(Key schedule_key) = 0;

    // operations
    virtual void                                reload(SourceKey source_key) = 0;
    virtual Job                                 reload_report(SourceKey source_key, ReportTypeUUID report_type_uuid, ContentReadyHandler completed_handler, JobFailedHandler failed_handler);
    virtual Job                                 rebuild_report(SourceKey source_key, ReportTypeUUID report_type_uuid, JobCompletedHandler completed_handler, JobFailedHandler failed_handler);
    virtual Job                                 launch_action(SourceKey source_key, ActionUUID action_uuid, Params params, ContentReadyHandler completed_handler, JobFailedHandler failed_handler);
    virtual Job                                 launch_shortcut(SourceKey source_key, UUID shortcut_uuid, ContentReadyHandler completed_handler, JobFailedHandler failed_handler);

    //export
    virtual Job                                 export_summary(ContentReadyHandler completed_handler, JobFailedHandler failed_handler);

    // features control
    virtual std::vector<FeatureInfo>            get_features_info() const = 0;
    virtual FeatureInfo                         get_feature_info(UUID feature_uuid) const = 0;
    virtual void                                install_feature(UUID feature_uuid) = 0;
    virtual void                                uninstall_feature(UUID feature_uuid) = 0;

protected:
    virtual void                                do_cancel_job(int id) const = 0;
    virtual int                                 do_query_current_content(SourceKey source_key, ReportTypeUUID report_type_uuid) const = 0;
    virtual int                                 do_query_archived_content(SourceKey source_key, ReportTypeUUID report_type_uuid, time_t time) const = 0;
    virtual int                                 do_query_stream_segment(SourceKey source_key, StreamTypeUUID stream_type_uuid, time_t start, time_t stop) const = 0;
    virtual int                                 do_reload_report(SourceKey source_key, ReportTypeUUID report_type_uuid) const = 0;
    virtual int                                 do_rebuild_report(SourceKey source_key, ReportTypeUUID report_type_uuid) const = 0;
    virtual int                                 do_launch_action(SourceKey source_key, ActionUUID action_uuid, Params params) const = 0;
    virtual int                                 do_launch_shortcut(SourceKey source_key, UUID shortcut_uuid) const = 0;
    virtual int                                 do_export_summary() const = 0;


    void forward_add_source_event(const SourceInfo& source_info);
    void forward_update_source_event(const SourceInfo& source_info);
    void forward_remove_source_event(Key source_key, const std::vector<SourceKey>& removed_sources);
    void forward_add_link_event(Link link);
    void forward_remove_link_event(Link link);
    void forward_update_current_report(const CurrentReportInfo& current_report_info, bool content_changed);
    void forward_job_completed(int id, boost::any any);
    void forward_job_failed(int id, const std::string& err);    

    Key m_session_key;
    EventTransporter m_event_transporter;
    std::vector<EventHandler*> m_event_handlers;
    mutable std::vector<std::unique_ptr<JobHandler>> m_job_handlers;
};

} //namespace TR