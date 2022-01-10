#pragma once
#include "AppDecl.h"
#include "PlainTypes.h"
namespace TR { namespace Core {
struct CoreImpl;

class PlainFacade
{
public: 
    PlainFacade(CoreImpl* core_impl);

    // summary info
    SummaryInfo                             get_summary_info() const;

    // security
    std::vector<UserInfo>                   get_users() const;
    Key                                     add_user(const UserSettings& user_settings);
    void                                    update_user(Key user_key, const UserSettings& user_settings);
    void                                    remove_user(Key user_key);

    SourceAccessInfo                        get_access_info(SourceKey source_key) const;
    std::vector<AccessEntry>                get_access_entries(SourceKey source_key) const;
    void                                    set_access_list(const SourceAccessList& source_access_list) const;  

    // report type info
    std::vector<ReportTypeInfo>             get_report_types_info() const;
    ReportTypeInfo                          get_report_type_info(ReportTypeUUID report_type_uuid) const;
    std::shared_ptr<const XmlDoc>           get_content_def(ReportTypeUUID report_type_uuid) const;

    // stream type info
    std::vector<StreamTypeInfo>             get_stream_types_info() const;
    StreamTypeInfo                          get_stream_type_info(StreamTypeUUID stream_type_uuid) const;
    StreamTypeInfo                          get_stream_type_info(const StreamType& stream_type) const;
    std::shared_ptr<const XmlDefDoc>        get_message_def(StreamTypeUUID stream_type_uuid) const;

    // action info
    std::vector<ActionInfo>                 get_actions_info() const;
    ActionInfo                              get_action_info(ActionUUID action_uuid) const;
    Params                                  get_default_params(ActionUUID action_uuid) const;
    Params                                  create_params(ActionUUID action_uuid, const std::string& params_xml) const;

    // source type info
    std::vector<SourceTypeInfo>             get_source_types_info() const;
    SourceTypeInfo                          get_source_type_info(UUID source_type_uuid) const;
    std::shared_ptr<const XmlDefDoc>        get_config_def(UUID source_type_uuid) const;
    SourceConfig                            get_default_config(UUID source_type_uuid) const;
    
    // roles info
    std::vector<RoleInfo>                   get_roles_info() const;
    RoleInfo                                get_role_info(RoleKey role_key) const;
    RoleKey                                 add_role(const RoleSettings& role_settings) const;
    void                                    update_role(RoleKey role_key, const RoleSettings& role_settings) const;
    void                                    remove_role(RoleKey role_key);

    // source tree access
    std::vector<SourceInfo>                 get_sources_info() const;
    SourceInfo                              get_source_info(SourceKey source_key) const;
    SourceKey                               add_source(const SourceSettings& source_settings);
    void                                    update_source(SourceKey source_key, const SourceSettings& source_settings);
    void                                    remove_source(SourceKey source_key);
    void                                    unlink_source(SourceRef source_ref, Transaction& t);
    
    SourceConfig                            get_source_config(SourceKey source_key) const;
    void                                    set_source_config(SourceKey source_key, SourceConfig config);
    
    // source link
    std::vector<TR::Link>                   get_links() const;
    void                                    add_link(TR::Link link);
    void                                    remove_link(TR::Link link);

    // current report
    CurrentReportInfo                       get_current_report_info(SourceKey source_key, ReportTypeUUID report_type_uuid) const;
    std::shared_ptr<Content>                get_current_content(SourceKey source_key, ReportTypeUUID report_type_uuid) const;

    //archive
    std::vector<ArchivedReportInfo>         get_archive(SourceKey source_key) const;
    ArchivedReportInfo                      get_archived_report(ArchivedReportKey archived_report_key) const;
    std::shared_ptr<Content>                get_archived_content(ArchivedReportKey archived_report_key) const;

    // streams
    StreamInfo                              get_stream_info(SourceKey source_key, StreamTypeUUID stream_type_uuid) const;

    //options
    SubjectReportGenerating                 get_report_generating_options(SubjectRef subject_ref, ReportTypeRef report_type_ref) const;
    SubjectIntegrityChecking                get_integrity_checking_options(Key user_key, SubjectRef subject_ref, ReportTypeRef report_type_ref) const;
    SubjectComplianceChecking               get_compliance_options(Key user_key, SubjectRef subject_ref, ComplianceRef compliance_ref) const;
    SubjectValidating                       get_validation_options(Key user_key, SubjectRef subject_ref, ValidationRef validation_ref) const;
    SubjectArchiving                        get_archiving_options(SubjectRef subject_ref, ReportTypeRef report_type_ref) const;
    SubjectStreamGenerating                 get_stream_generating_options(Key user_key, SubjectRef subject_ref, StreamTypeRef stream_type_ref) const;

    SubjectReportOptions                    get_options(Key user_key, SubjectRef subject_ref, ReportTypeRef report_type_ref) const;
    SubjectStreamOptions                    get_options(Key user_key, SubjectRef subject_ref, StreamTypeRef stream_type_ref) const;
    SubjectActionOptions                    get_options(SubjectRef subject_ref, ActionShortcutRef shortcut_ref) const;
    SubjectActionOptions                    get_options(SubjectRef subject_ref, ActionRef action_ref) const;

    SubjectReportOptions                    get_options(Key user_key, SubjectKey subject_key, ReportTypeUUID report_type_uuid) const;
    SubjectStreamOptions                    get_options(Key user_key, SubjectKey subject_key, StreamTypeUUID stream_type_uuid) const;
    SubjectActionOptions                    get_options(SubjectKey subject_key, ActionUUID action_uuid, boost::optional<UUID> shortcut_uuid) const;

    void                                    set_options(Key user_key, SubjectKey subject_key, const ReportOptions& report_options);
    void                                    set_options(Key user_key, SubjectKey subject_key, const StreamOptions& stream_options);
    void                                    set_options(SubjectKey subject_key, const ActionOptions& action_options);
    
    SubjectOptions                          get_role_options(Key user_key, RoleKey role_key) const;
    SubjectOptions                          get_source_options(Key user_key, SourceKey source_key) const;   
    ReportTypeOptions                       get_report_type_options(Key user_key, ReportTypeUUID report_type_uuid) const;
    StreamTypeOptions                       get_stream_type_options(Key user_key, StreamTypeUUID stream_type_uuid) const;
    ActionTypeOptions                       get_action_options(ActionUUID action_uuid, boost::optional<UUID> shortcut_uuid) const;      

    //custom query
    Key                                     get_custom_query_key(ReportTypeRef output_ref) const;
    CustomQueryInfo                         get_custom_query_info(Key custom_query_key) const;
    CustomQueryDefinition                   get_custom_query_def(Key custom_query_key) const;
    CustomQueryDefinition                   create_custom_query(std::vector<ReportTypeUUID> input_uuids) const;
    Key                                     add_custom_query(std::wstring name, CustomQueryDefinition custom_query_def);
    void                                    remove_custom_query(Key custom_query_key);  

    //custom filter
    Key                                     get_custom_filter_key(ReportTypeRef output_ref) const;
    CustomFilterInfo                        get_custom_filter_info(Key custom_filter_key) const;
    Key                                     add_custom_filter(std::wstring name, ReportTypeUUID input_uuid, CustomFilterSetup custom_filter_setup);
    //void                                  update_custom_filter(Key custom_filter_key, std::wstring name);
    CustomFilterSetup                       create_filter_setup(ReportTypeUUID input_uuid) const;
    CustomFilterSetup                       get_filter_setup(Key custom_filter_key) const;
    void                                    set_filter_setup(Key custom_filter_key, CustomFilterSetup custom_filter_setup);

    //tracking stream
    Key                                     get_tracking_stream_key(StreamTypeRef output_ref) const;
    TrackingStreamInfo                      get_tracking_stream_info(Key tracking_stream_key) const;
    Key                                     add_tracking_stream(std::wstring name, ReportTypeUUID input_uuid, std::string input_xpath);

    //stream filter
    Key                                     get_stream_filter_key(StreamTypeRef output_ref) const;
    StreamFilterInfo                        get_stream_filter_info(Key stream_filter_key) const;
    StreamFilterSetup                       create_filter_setup(StreamTypeUUID input_uuid) const;
    Key                                     add_stream_filter(std::wstring name, StreamTypeUUID input_uuid, StreamFilterSetup stream_filter_setup); 
    StreamFilterSetup                       get_stream_filter_setup(Key stream_filter_key) const;
    void                                    set_stream_filter_setup(Key stream_filter_key, StreamFilterSetup stream_filter_setup);

    //compliance                
    ComplianceKey                           get_compliance_key(ReportTypeRef output_ref) const;
    std::vector<ComplianceInfo>             get_compliances_info() const;
    ComplianceInfo                          get_compliance_info(ComplianceKey compliance_key) const;
    ComplianceSetup                         get_compliance_setup(SubjectKey subject_key, ComplianceKey compliance_key) const;
    void                                    set_compliance_setup(SubjectKey subject_key, ComplianceKey compliance_key, ComplianceSetup compliance_setup);

    //validation                
    ValidationKey                           get_validation_key(ReportTypeRef output_ref) const;
    ValidationInfo                          get_validation_info(ValidationKey validation_key) const;    
    ValidationKey                           add_validation(std::wstring name, ReportTypeUUID input_uuid, ValidationSettings default_settings) const;
    ValidationSettings                      create_validation_settings(ReportTypeUUID input_uuid) const;
    ValidationSettings                      get_validation_settings(SubjectKey subject_key, ValidationKey validation_key) const;
    void                                    set_validation_settings(SubjectKey subject_key, ValidationKey validation_key, ValidationSettings validation_settings);
    
    //grouping              
    Key                                     get_grouping_key(ReportTypeRef output_ref) const;
    GroupingInfo                            get_grouping_info(Key grouping_key) const;
    Key                                     add_grouping(std::wstring name, ReportTypeUUID input_uuid);

    //custom report 
    void                                    remove_custom_report(ReportTypeRef report_type_ref, bool recursive, Transaction& t);
    void                                    remove_custom_report(ReportTypeUUID report_type_uuid, bool recursive);

    //integrity checking 
    std::shared_ptr<Content>                create_pattern(ReportTypeUUID report_type_uuid) const;
    std::shared_ptr<Content>                create_pattern(ReportTypeUUID report_type_uuid, Blob blob) const;
    std::shared_ptr<Content>                get_pattern(SubjectKey subject_key, ReportTypeUUID report_type_uuid) const;
    std::shared_ptr<Content>                get_effective_pattern(SubjectKey subject_key, ReportTypeUUID report_type_uuid) const;
    void                                    set_pattern(SubjectKey subject_key, ReportTypeUUID report_type_uuid, Blob blob);
    void                                    reset_pattern(SubjectKey subject_key, ReportTypeUUID report_type_uuid);

    //custom actions
    UUID                                    add_custom_action(std::wstring name, ActionUUID action_uuid, Params params);
    void                                    remove_custom_action(UUID custom_action_uuid);

    // scheduler
    std::vector<ScheduleInfo>               get_schedules_info() const;
    ScheduleInfo                            get_schedule_info(Key schedule_key) const;
    Key                                     add_schedule(const ScheduleSettings& schedule_settings);
    void                                    update_schedule(Key schedule_key, const ScheduleSettings& schedule_settings);
    void                                    remove_schedule(Key schedule_key);

private:
    CoreImpl* m_core_impl;
    Database& m_db;
    Basis& m_basis;
    Registry& m_registry;
    Services& m_services;
    SourceTypes& m_source_types;
    ReportTypes& m_report_types;
    StreamTypes& m_stream_types;
    Transformations& m_transformations;
    Actions& m_actions;
    ActionShortcuts& m_action_shortcuts;    
    SourceTypeActivities& m_source_type_activities;     
    Roles& m_roles;
    Sources& m_sources;
    Links& m_links;
    Collector& m_collector;
    Reservoir& m_reservoir;
    ResourcePolicies& m_resource_policies;  
    SourceResources& m_source_resources;
    CustomQueries& m_custom_queries;
    CustomFilters& m_custom_filters;
    TrackingStreams& m_tracking_streams;
    StreamFilters& m_stream_filters;
    Compliances& m_compliances;
    Validations& m_validations;
    GroupingReports& m_grouping_reports;
    IntegrityChecking& m_integrity_checking;
    Archive& m_archive;
    CustomActions& m_custom_actions;
    Security& m_security;
    Scheduler& m_scheduler;
    Notifier& m_notifier;
    Mailing& m_mailing;
};

}} //namespace TR { namespace Core {