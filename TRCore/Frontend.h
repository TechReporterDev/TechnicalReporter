#pragma once
#include "PlainTypes.h"
namespace TR { namespace Core {
struct CoreImpl;
struct Executive;
class Features;
class PlainFacade;
class Security;
class Async;

class Frontend
{
public:
    Frontend(CoreImpl& core_impl);
    ~Frontend();

    Frontend(const Frontend&) = delete;
    Frontend& operator = (const Frontend&) = delete;
    Frontend(Frontend&&) = delete;
    Frontend& operator = (Frontend&&) = delete;

    void                                    set_features(Features* features);
    void                                    set_async(Async* async);

    // session
    Key                                     open_session(const std::wstring& user_name);
    void                                    close_session(Key session_key);
    void                                    cancel_job(Key session_key, int id);

    // security
    std::vector<UserInfo>                   get_users(Key session_key) const;
    Key                                     add_user(Key session_key, const UserSettings& user_settings);
    void                                    update_user(Key session_key, Key user_key, const UserSettings& user_settings);
    void                                    remove_user(Key session_key, Key user_key);
    SourceAccessInfo                        get_access_info(Key session_key, SourceKey source_key) const;
    std::vector<AccessEntry>                get_access_entries(Key session_key, SourceKey source_key) const;
    void                                    set_access_list(Key session_key, const SourceAccessList& source_access_list) const; 
    bool                                    check_access_rights(Key session_key, SourceKey source_key, TR::AccessRights access_rights) const;

    // summary info
    SummaryInfo                             get_summary_info(Key session_key) const;

    // report type info
    std::vector<ReportTypeInfo>             get_report_types_info(Key session_key) const;
    ReportTypeInfo                          get_report_type_info(Key session_key, ReportTypeUUID report_type_uuid) const;
    std::shared_ptr<const XmlDoc>           get_content_def(Key session_key, ReportTypeUUID report_type_uuid) const;
        
    // stream type info
    std::vector<StreamTypeInfo>             get_stream_types_info(Key session_key) const;
    StreamTypeInfo                          get_stream_type_info(Key session_key, StreamTypeUUID stream_type_uuid) const;
    std::shared_ptr<const XmlDefDoc>        get_message_def(Key session_key, StreamTypeUUID stream_type_uuid) const;

    // action info
    std::vector<ActionInfo>                 get_actions_info(Key session_key) const;
    ActionInfo                              get_action_info(Key session_key, ActionUUID action_uuid) const;
    Params                                  get_default_params(Key session_key, ActionUUID action_uuid) const;
    Params                                  create_params(Key session_key, ActionUUID action_uuid, const std::string& params_xml) const;

    // source type info
    std::vector<SourceTypeInfo>             get_source_types_info(Key session_key) const;
    SourceTypeInfo                          get_source_type_info(Key session_key, SourceTypeUUID source_type_uuid) const;
    std::shared_ptr<const XmlDefDoc>        get_config_def(Key session_key, SourceTypeUUID source_type_uuid) const;
    SourceConfig                            get_default_config(Key session_key, SourceTypeUUID source_type_uuid) const;

    // roles info
    std::vector<RoleInfo>                   get_roles_info(Key session_key) const ;
    RoleInfo                                get_role_info(Key session_key, RoleKey role_key) const;
    RoleKey                                 add_role(Key session_key, const RoleSettings& role_settings) const;
    void                                    update_role(Key session_key, RoleKey role_key, const RoleSettings& role_settings) const;
    void                                    remove_role(Key session_key, RoleKey role_key);

    // source tree access
    std::vector<SourceInfo>                 get_sources_info(Key session_key) const;
    SourceInfo                              get_source_info(Key session_key, SourceKey source_key) const;
    SourceKey                               add_source(Key session_key, const SourceSettings& source_settings);
    void                                    update_source(Key session_key, SourceKey source_key, const SourceSettings& source_settings);
    void                                    remove_source(Key session_key, SourceKey source_key);
    SourceConfig                            get_source_config(Key session_key, SourceKey source_key) const;
    void                                    set_source_config(Key session_key, SourceKey source_key, SourceConfig config);

    // source link
    std::vector<TR::Link>                   get_links(Key session_key) const;
    void                                    add_link(Key session_key, TR::Link link);
    void                                    remove_link(Key session_key, TR::Link link);

    // current report info
    CurrentReportInfo                       get_current_report(Key session_key, SourceKey source_key, ReportTypeUUID report_type_uuid) const;
    std::shared_ptr<Content>                get_current_content(Key session_key, SourceKey source_key, ReportTypeUUID report_type_uuid) const;
    int                                     query_current_content(Key session_key, SourceKey source_key, ReportTypeUUID report_type_uuid) const;

    // archive
    std::vector<ArchivedReportInfo>         get_archive(Key session_key, SourceKey source_key) const;
    ArchivedReportInfo                      get_archived_report(Key session_key, ArchivedReportKey archived_report_key) const;
    std::shared_ptr<Content>                get_archived_content(Key session_key, ArchivedReportKey archived_report_key) const;
    int                                     query_archived_content(Key session_key, SourceKey source_key, UUID report_type_uuid, time_t time) const;

    // streams
    StreamInfo                              get_stream_info(Key session_key, SourceKey source_key, StreamTypeUUID stream_type_uuid) const;
    int                                     query_stream_segment(Key session_key, SourceKey source_key, StreamTypeUUID stream_type_uuid, time_t start, time_t stop) const;
    
    // options
    SubjectReportOptions                    get_options(Key session_key, SubjectKey subject_key, ReportTypeUUID report_type_uuid) const;
    SubjectStreamOptions                    get_options(Key session_key, SubjectKey subject_key, StreamTypeUUID stream_type_uuid) const;
    SubjectActionOptions                    get_options(Key session_key, SubjectKey subject_key, ActionUUID action_uuid, boost::optional<UUID> shortcut_uuid) const;
    
    void                                    set_options(Key session_key, SubjectKey subject_key, const ReportOptions& report_options);
    void                                    set_options(Key session_key, SubjectKey subject_key, const StreamOptions& stream_options);
    void                                    set_options(Key session_key, SubjectKey subject_key, const ActionOptions& shortcut_options);
    
    // get options 
    SubjectOptions                          get_role_options(Key session_key, RoleKey role_key) const;
    SubjectOptions                          get_source_options(Key session_key, SourceKey source_key) const;
    ReportTypeOptions                       get_report_type_options(Key session_key, ReportTypeUUID report_type_uuid) const;
    StreamTypeOptions                       get_stream_type_options(Key session_key, StreamTypeUUID stream_type_uuid) const;
    ActionTypeOptions                       get_action_options(Key session_key, ActionUUID action_uuid, boost::optional<UUID> shortcut_uuid) const;
    
    // custom queries
    CustomQueryInfo                         get_custom_query_info(Key session_key, Key custom_query_key) const;
    CustomQueryDefinition                   get_custom_query_def(Key session_key, Key custom_query_key) const;
    CustomQueryDefinition                   create_custom_query(Key session_key, std::vector<ReportTypeUUID> input_uuids) const;
    Key                                     add_custom_query(Key session_key, std::wstring name, CustomQueryDefinition custom_query_def);
    void                                    remove_custom_query(Key session_key, Key custom_query_key);

    //custom filter
    CustomFilterInfo                        get_custom_filter_info(Key session_key, Key custom_filter_key) const;
    Key                                     add_custom_filter(Key session_key, std::wstring name, ReportTypeUUID input_uuid, CustomFilterSetup custom_filter_setup);
    CustomFilterSetup                       create_filter_setup(Key session_key, ReportTypeUUID input_uuid) const;
    CustomFilterSetup                       get_filter_setup(Key session_key, Key custom_filter_key) const;
    void                                    set_filter_setup(Key session_key, Key custom_filter_key, CustomFilterSetup custom_filter_setup);

    //tracking stream
    TrackingStreamInfo                      get_tracking_stream_info(Key session_key, Key tracking_stream_key) const;
    Key                                     add_tracking_stream(Key session_key, std::wstring name, ReportTypeUUID input_uuid, std::string input_xpath);    

    //stream filter
    StreamFilterInfo                        get_stream_filter_info(Key session_key, Key stream_filter_key) const;
    StreamFilterSetup                       create_filter_setup(Key session_key, StreamTypeUUID input_uuid) const;
    Key                                     add_stream_filter(Key session_key, std::wstring name, StreamTypeUUID  input_uuid, StreamFilterSetup stream_filter_setup);
    StreamFilterSetup                       get_stream_filter_setup(Key session_key, Key stream_filter_key) const;
    void                                    set_stream_filter_setup(Key session_key, Key stream_filter_key, StreamFilterSetup stream_filter_setup);

        // compliance
    std::vector<ComplianceInfo>             get_compliances_info(Key session_key) const;
    ComplianceInfo                          get_compliance_info(Key session_key, ComplianceKey compliance_key) const;
    ComplianceSetup                         get_compliance_setup(Key session_key, SubjectKey subject_key, ComplianceKey compliance_key) const;
    void                                    set_compliance_setup(Key session_key, SubjectKey subject_key, ComplianceKey compliance_key, ComplianceSetup compliance_setup);

    //validation                
    ValidationInfo                          get_validation_info(Key session_key, ValidationKey validation_key) const;
    ValidationKey                           add_validation(Key session_key, std::wstring name, ReportTypeUUID input_uuid, ValidationSettings default_settings) const;
    ValidationSettings                      create_validation_settings(Key session_key, ReportTypeUUID input_uuid) const;
    ValidationSettings                      get_validation_settings(Key session_key, SubjectKey subject_key, ValidationKey validation_key) const;
    void                                    set_validation_settings(Key session_key, SubjectKey subject_key, ValidationKey validation_key, ValidationSettings validation_settings);
    
    // grouping             
    GroupingInfo                            get_grouping_info(Key session_key, Key grouping_key) const;
    Key                                     add_grouping(Key session_key, std::wstring name, ReportTypeUUID input_uuid);

    //custom report 
    void                                    remove_custom_report(Key session_key, ReportTypeUUID report_type_uuid, bool recursive);

    // integrity checking 
    std::shared_ptr<Content>                create_pattern(Key session_key, ReportTypeUUID report_type_uuid) const;
    std::shared_ptr<Content>                create_pattern(Key session_key, ReportTypeUUID report_type_uuid, Blob blob) const;
    std::shared_ptr<Content>                get_pattern(Key session_key, SubjectKey subject_key, ReportTypeUUID report_type_uuid) const;
    std::shared_ptr<Content>                get_effective_pattern(Key session_key, SubjectKey subject_key, ReportTypeUUID report_type_uuid) const;
    void                                    set_pattern(Key session_key, SubjectKey subject_key, ReportTypeUUID report_type_uuid, Blob blob);
    void                                    reset_pattern(Key session_key, SubjectKey subject_key, ReportTypeUUID report_type_uuid);

    // custom actions
    UUID                                    add_custom_action(Key session_key, std::wstring name, ActionUUID action_uuid, Params params);
    void                                    remove_custom_action(Key session_key, UUID custom_action_uuid);

    // scheduler
    std::vector<ScheduleInfo>               get_schedules_info(Key session_key) const;
    ScheduleInfo                            get_schedule_info(Key session_key, Key schedule_key) const;
    Key                                     add_schedule(Key session_key, const ScheduleSettings& schedule_settings);
    void                                    update_schedule(Key session_key, Key schedule_key, const ScheduleSettings& schedule_settings);
    void                                    remove_schedule(Key session_key, Key schedule_key);
    
    // operations
    void                                    reload(Key session_key, SourceKey source_key);
    int                                     reload_report(Key session_key, SourceKey source_key, ReportTypeUUID report_type_uuid);
    int                                     rebuild_report(Key session_key, SourceKey source_key, ReportTypeUUID report_type_uuid);
    int                                     launch_action(Key session_key, SourceKey source_key, ActionUUID action_uuid, Params params);
    int                                     launch_shortcut(Key session_key, SourceKey source_key, UUID shortcut_uuid);

    //export
    int                                     export_summary(Key session_key);

    // features control
    std::vector<FeatureInfo>                get_features_info(Key session_key) const;
    FeatureInfo                             get_feature_info(Key session_key, UUID feature_uuid) const;
    void                                    install_feature(Key session_key, UUID feature_uuid);    
    void                                    uninstall_feature(Key session_key, UUID feature_uuid);
        
private:
    CoreImpl& m_core_impl;
    Executive& m_executive;
    std::unique_ptr<PlainFacade> m_facade;
    Security* m_security;
    Features* m_features;
    Async* m_async; 
};

}} //namespace TR { namespace Core {