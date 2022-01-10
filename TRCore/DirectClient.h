#pragma once
#include "Client.h"
#include "CoreFwd.h"
namespace TR {

class DirectClientDispatcher;
void connect_direct_client_dispatcher(Core::Core& core);

class DirectClient: public Client
{
public:
    friend class DirectClientDispatcher;
    DirectClient();
    DirectClient(EventTransporter event_transporter);
    DirectClient(const DirectClient&) = delete;
    DirectClient(DirectClient&&) = delete;
    virtual ~DirectClient();

    DirectClient& operator = (const DirectClient&) = delete;
    DirectClient& operator = (DirectClient&&) = delete;

    void                                        connect(Core::Core& core, const std::wstring& user);
    bool                                        is_connected() const;
    
    // security
    virtual std::vector<UserInfo>               get_users() const override;
    virtual Key                                 add_user(const UserSettings& user_settings) override;
    virtual void                                update_user(Key user_key, const UserSettings& user_settings) override;
    virtual void                                remove_user(Key user_key) override;
    virtual SourceAccessInfo                    get_access_info(SourceKey source_key) const override;
    virtual std::vector<AccessEntry>            get_access_entries(SourceKey source_key) const override;
    virtual void                                set_access_list(const SourceAccessList& source_access_list) const override;

    // summary info
    virtual SummaryInfo                         get_summary_info() const override;

    // report type info
    virtual std::vector<ReportTypeInfo>         get_report_types_info() const override;
    virtual ReportTypeInfo                      get_report_type_info(ReportTypeUUID report_type_uuid) const override;
    virtual std::shared_ptr<const XmlDoc>       get_content_def(ReportTypeUUID report_type_uuid) const override;

    // stream type info
    virtual std::vector<StreamTypeInfo>         get_stream_types_info() const override;
    virtual StreamTypeInfo                      get_stream_type_info(StreamTypeUUID stream_type_uuid) const override;
    virtual std::shared_ptr<const XmlDefDoc>    get_message_def(StreamTypeUUID stream_type_uuid) const override;

    // action info
    virtual std::vector<ActionInfo>             get_actions_info() const  override;
    virtual ActionInfo                          get_action_info(ActionUUID action_uuid) const override;
    virtual Params                              get_default_params(ActionUUID action_uuid) const override;
    virtual Params                              create_params(ActionUUID action_uuid, const std::string& params_xml) const override;

    // source type info
    virtual std::vector<SourceTypeInfo>         get_source_types_info() const override;
    virtual SourceTypeInfo                      get_source_type_info(UUID source_type_uuid) const override;
    virtual std::shared_ptr<const XmlDefDoc>    get_config_def(UUID source_type_uuid) const override;
    virtual SourceConfig                        get_default_config(UUID source_type_uuid) const override;

    // roles info
    virtual std::vector<RoleInfo>               get_roles_info() const override;
    virtual RoleInfo                            get_role_info(RoleKey role_key) const override;
    virtual RoleKey                             add_role(const RoleSettings& role_settings) const override;
    virtual void                                update_role(RoleKey role_key, const RoleSettings& role_settings) const override;
    virtual void                                remove_role(RoleKey role_key) override;

    // source tree access
    virtual std::vector<SourceInfo>             get_sources_info() const override;
    virtual SourceInfo                          get_source_info(SourceKey source_key) const override;
    virtual SourceKey                           add_source(const SourceSettings& source_settings) override;
    virtual void                                update_source(SourceKey source_key, const SourceSettings& source_settings) override;
    virtual void                                remove_source(SourceKey source_key) override;
    virtual SourceConfig                        get_source_config(SourceKey source_key) const override;
    virtual void                                set_source_config(SourceKey source_key, SourceConfig config) override;

    // source link
    virtual std::vector<Link>                   get_links() const override;
    virtual void                                add_link(Link link) override;
    virtual void                                remove_link(Link link) override;

    // report info
    virtual CurrentReportInfo                   get_current_report(SourceKey source_key, ReportTypeUUID report_type_uuid) const override;
    virtual std::shared_ptr<Content>            get_current_content(SourceKey source_key, ReportTypeUUID report_type_uuid) const override;
    //virtual void                              query_current_content(SourceKey source_key, ReportTypeUUID report_type_uuid) const override;

    // archive
    virtual std::vector<ArchivedReportInfo>     get_archive(SourceKey source_key) const override;
    virtual ArchivedReportInfo                  get_archived_report(ArchivedReportKey archived_report_key) const override;
    virtual std::shared_ptr<Content>            get_archived_content(ArchivedReportKey archived_report_key) const override;
    //virtual void                              query_archived_content(SourceKey source_key, ReportTypeUUID report_type_uuid, time_t time) const override;

    // stream
    virtual StreamInfo                          get_stream_info(SourceKey source_key, StreamTypeUUID stream_type_uuid) const;
    //virtual Job                               query_stream_segment(SourceKey source_key, StreamTypeUUID stream_type_uuid, StreamReadyHandler completed_handler, JobFailedHandler failed_handler) const override;
    
    // options
    virtual SubjectReportOptions                get_options(SubjectKey subject_key, ReportTypeUUID report_type_uuid) const override;
    virtual SubjectStreamOptions                get_options(SubjectKey subject_key, StreamTypeUUID stream_type_uuid) const override;    
    virtual SubjectActionOptions                get_options(SubjectKey subject_key, ActionUUID action_uuid, boost::optional<UUID> shortcut_uuid) const override;
    virtual void                                set_options(SubjectKey subject_key, const ReportOptions& report_options) override;
    virtual void                                set_options(SubjectKey subject_key, const StreamOptions& stream_options) override;
    virtual void                                set_options(SubjectKey subject_key, const ActionOptions& shortcut_options) override;

    virtual SubjectOptions                      get_subject_options(SubjectKey subject_key) const override;
    virtual ReportTypeOptions                   get_report_type_options(ReportTypeUUID report_type_uuid) const override;
    virtual StreamTypeOptions                   get_stream_type_options(StreamTypeUUID stream_type_uuid) const override;
    virtual ActionTypeOptions                   get_action_options(ActionUUID action_uuid, boost::optional<UUID> shortcut_uuid) const override;

    // custom query
    virtual CustomQueryInfo                     get_custom_query_info(Key custom_query_key) const override;
    virtual CustomQueryDefinition               get_custom_query_def(Key custom_query_key) const override;
    virtual CustomQueryDefinition               create_custom_query(std::vector<ReportTypeUUID> input_uuids) const override;
    virtual Key                                 add_custom_query(std::wstring name, CustomQueryDefinition custom_query_def) override;
    virtual void                                remove_custom_query(Key custom_query_key) override;

    //custom filter
    virtual CustomFilterInfo                    get_custom_filter_info(Key custom_filter_key) const override;
    virtual Key                                 add_custom_filter(std::wstring name, ReportTypeUUID input_uuid, CustomFilterSetup custom_filter_setup) override;
    virtual CustomFilterSetup                   create_filter_setup(ReportTypeUUID input_uuid) const override;
    virtual CustomFilterSetup                   get_filter_setup(Key custom_filter_key) const override;
    virtual void                                set_filter_setup(Key custom_filter_key, CustomFilterSetup custom_filter_setup) const override;

    //tracking stream
    virtual TrackingStreamInfo                  get_tracking_stream_info(Key tracking_stream_key) const override;
    virtual Key                                 add_tracking_stream(std::wstring name, ReportTypeUUID input_uuid, std::string input_xpath) override;

    //stream filter
    virtual StreamFilterInfo                    get_stream_filter_info(Key stream_filter_key) const override;
    virtual StreamFilterSetup                   create_filter_setup(StreamTypeUUID input_uuid) const override;
    virtual Key                                 add_stream_filter(std::wstring name, StreamTypeUUID  input_uuid, StreamFilterSetup stream_filter_setup) override;
    virtual StreamFilterSetup                   get_stream_filter_setup(Key stream_filter_key) const override;
    virtual void                                set_stream_filter_setup(Key stream_filter_key, StreamFilterSetup stream_filter_setup) override;

    // compliance
    virtual std::vector<ComplianceInfo>         get_compliances_info() const override;
    virtual ComplianceInfo                      get_compliance_info(ComplianceKey compliance_key) const override;
    virtual ComplianceSetup                     get_compliance_setup(SubjectKey subject_key, ComplianceKey compliance_key) const override;
    virtual void                                set_compliance_setup(SubjectKey subject_key, ComplianceKey compliance_key, ComplianceSetup compliance_setup) override;
    
    // validation               
    virtual ValidationInfo                      get_validation_info(ValidationKey validation_key) const override;
    virtual ValidationKey                       add_validation(std::wstring name, ReportTypeUUID input_uuid, ValidationSettings default_settings) const override;
    virtual ValidationSettings                  create_validation_settings(ReportTypeUUID input_uuid) const override;
    virtual ValidationSettings                  get_validation_settings(SubjectKey subject_key, ValidationKey validation_key) const override;
    virtual void                                set_validation_settings(SubjectKey subject_key, ValidationKey validation_key, ValidationSettings validation_settings) override;

    // grouping
    virtual GroupingInfo                        get_grouping_info(Key grouping_key) const override;
    virtual Key                                 add_grouping(std::wstring name, ReportTypeUUID input_uuid) override;

    //custom report 
    virtual void                                remove_custom_report(ReportTypeUUID report_type_uuid, bool recursive) override;

    // integrity checking 
    virtual std::shared_ptr<Content>            create_pattern(ReportTypeUUID report_type_uuid) const override;
    virtual std::shared_ptr<Content>            create_pattern(ReportTypeUUID report_type_uuid, Blob blob) const override;
    virtual std::shared_ptr<Content>            get_pattern(SubjectKey subject_key, ReportTypeUUID report_type_uuid) const override;
    virtual std::shared_ptr<Content>            get_effective_pattern(SubjectKey subject_key, ReportTypeUUID report_type_uuid) const override;
    virtual void                                set_pattern(SubjectKey subject_key, ReportTypeUUID report_type_uuid, Blob blob) override;
    virtual void                                reset_pattern(SubjectKey subject_key, ReportTypeUUID report_type_uuid) override;
        
    // custom actions
    virtual UUID                                add_custom_action(std::wstring name, ActionUUID action_uuid, Params params) override;
    virtual void                                remove_custom_action(UUID custom_action_uuid) override;

    // scheduler
    virtual std::vector<ScheduleInfo>           get_schedules_info() const override;
    virtual ScheduleInfo                        get_schedule_info(Key schedule_key) const override;
    virtual Key                                 add_schedule(const ScheduleSettings& schedule_settings) override;
    virtual void                                update_schedule(Key schedule_key, const ScheduleSettings& schedule_settings) override;
    virtual void                                remove_schedule(Key schedule_key) override;

    // operations
    virtual void                                reload(SourceKey source_key) override;
    //virtual Job                               reload_report(SourceKey source_key, ReportTypeUUID report_type_uuid, ContentReadyHandler completed_handler, JobFailedHandler failed_handler) override;
    //virtual Job                               launch_action(SourceKey source_key, ActionUUID action_uuid, Params params, ActionCompletedHandler completed_handler, JobFailedHandler failed_handler) override;
    //virtual Job                               launch_shortcut(SourceKey source_key, ActionUUID shortcut_uuid, ActionCompletedHandler completed_handler, JobFailedHandler failed_handler) override;

    // features control
    virtual std::vector<FeatureInfo>            get_features_info() const override;
    virtual FeatureInfo                         get_feature_info(UUID feature_uuid) const override;
    virtual void                                install_feature(UUID feature_uuid) override;
    virtual void                                uninstall_feature(UUID feature_uuid) override;

protected:
    virtual void                                do_cancel_job(int id) const override;
    virtual int                                 do_query_current_content(SourceKey source_key, ReportTypeUUID report_type_uuid) const override;
    virtual int                                 do_query_archived_content(SourceKey source_key, ReportTypeUUID report_type_uuid, time_t time) const override;
    virtual int                                 do_query_stream_segment(SourceKey source_key, StreamTypeUUID stream_type_uuid, time_t start, time_t stop) const override;
    virtual int                                 do_reload_report(SourceKey source_key, ReportTypeUUID report_type_uuid) const override;
    virtual int                                 do_rebuild_report(SourceKey source_key, ReportTypeUUID report_type_uuid) const override;
    virtual int                                 do_launch_action(SourceKey source_key, ActionUUID action_uuid, Params params) const override;
    virtual int                                 do_launch_shortcut(SourceKey source_key, UUID shortcut_uuid) const override;
    virtual int                                 do_export_summary() const override;

    Core::Core* m_core;
    Core::Frontend* m_frontend;
};

} //namespace TR {