#include "stdafx.h"
#include "DirectClient.h"
#include "Core.h"
namespace TR {

class DirectClientDispatcher: public Core::EventDispatcher
{
public:
    void            add_direct_client(DirectClient* client);
    void            remove_direct_client(DirectClient* client);
    DirectClient*   find(Key session_key);

    // override EventDispatcher
    virtual void    on_add_source(const std::vector<Key>& target_sessions, const SourceInfo& source_info) override;
    virtual void    on_update_source(const std::vector<Key>& target_sessions, const SourceInfo& source_info) override;
    virtual void    on_remove_source(const std::vector<Key>& target_sessions, Key source_key, const std::vector<SourceKey>& removed_sources) override;
    virtual void    on_add_link(const std::vector<Key>& target_sessions, TR::Link link) override;
    virtual void    on_remove_link(const std::vector<Key>& target_sessions, TR::Link link) override;
    virtual void    on_update_current_report(const std::vector<Key>& target_sessions, const CurrentReportInfo& current_report_info, bool content_changed) override;
    virtual void    on_query_content_completed(Key session_key, int query_id, std::shared_ptr<Content> content) override;
    virtual void    on_query_stream_completed(Key session_key, int query_id, const StreamSegment& stream_segment) override;
    virtual void    on_action_completed(Key session_key, int id, std::shared_ptr<Content> content) override;
    virtual void    on_reload_completed(Key session_key, int id, std::shared_ptr<Content> content) override;
    virtual void    on_rebuild_completed(Key session_key, int id) override;
    virtual void    on_export_completed(Key session_key, int id, std::shared_ptr<Content> output) override;
    virtual void    on_job_failed(Key session_key, int id, const std::exception& err) override;
    
private:
    DirectClient*   find_no_lock(Key session_key);

    std::vector<DirectClient*> m_clients;
    std::mutex m_mutex;
};

void DirectClientDispatcher::add_direct_client(DirectClient* client)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    _ASSERT(boost::find(m_clients, client) == m_clients.end());
    m_clients.push_back(client);
}

void DirectClientDispatcher::remove_direct_client(DirectClient* client)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto found = boost::find(m_clients, client);
    if (found == m_clients.end())
    {
        throw std::logic_error("Client not found");
    }
    m_clients.erase(found);
}

DirectClient* DirectClientDispatcher::find(Key session_key)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return find_no_lock(session_key);   
}

void DirectClientDispatcher::on_add_source(const std::vector<Key>& target_sessions, const SourceInfo& source_info)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto session_key : target_sessions)
    {
        if (auto direct_client = find_no_lock(session_key))
        {
            direct_client->forward_add_source_event(source_info);
        }
    }
}

void DirectClientDispatcher::on_update_source(const std::vector<Key>& target_sessions, const SourceInfo& source_info)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto session_key : target_sessions)
    {
        if (auto direct_client = find_no_lock(session_key))
        {
            direct_client->forward_update_source_event(source_info);
        }
    }
}

void DirectClientDispatcher::on_remove_source(const std::vector<Key>& target_sessions, Key source_key, const std::vector<SourceKey>& removed_sources)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto session_key : target_sessions)
    {
        if (auto direct_client = find_no_lock(session_key))
        {
            direct_client->forward_remove_source_event(source_key, removed_sources);
        }
    }
}

void DirectClientDispatcher::on_add_link(const std::vector<Key>& target_sessions, TR::Link link)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto session_key : target_sessions)
    {
        if (auto direct_client = find_no_lock(session_key))
        {
            direct_client->forward_add_link_event(link);
        }
    }
}

void DirectClientDispatcher::on_remove_link(const std::vector<Key>& target_sessions, TR::Link link)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto session_key : target_sessions)
    {
        if (auto direct_client = find_no_lock(session_key))
        {
            direct_client->forward_remove_link_event(link);
        }
    }
}

void DirectClientDispatcher::on_update_current_report(const std::vector<Key>& target_sessions, const CurrentReportInfo& current_report_info, bool content_changed)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto session_key : target_sessions)
    {
        if (auto direct_client = find_no_lock(session_key))
        {
            direct_client->forward_update_current_report(current_report_info, content_changed);
        }
    }
}

void DirectClientDispatcher::on_query_content_completed(Key session_key, int id, std::shared_ptr<Content> content)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (auto direct_client = find_no_lock(session_key))
    {
        direct_client->forward_job_completed(id, content);
    }
}

void DirectClientDispatcher::on_query_stream_completed(Key session_key, int id, const StreamSegment& stream_segment)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (auto direct_client = find_no_lock(session_key))
    {
        direct_client->forward_job_completed(id, stream_segment);
    }
}

void DirectClientDispatcher::on_action_completed(Key session_key, int id, std::shared_ptr<Content> content)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (auto direct_client = find_no_lock(session_key))
    {
        direct_client->forward_job_completed(id, content);
    }
}

void DirectClientDispatcher::on_reload_completed(Key session_key, int id, std::shared_ptr<Content> content)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (auto direct_client = find_no_lock(session_key))
    {
        direct_client->forward_job_completed(id, content);
    }
}

void DirectClientDispatcher::on_rebuild_completed(Key session_key, int id)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (auto direct_client = find_no_lock(session_key))
    {
        direct_client->forward_job_completed(id, nullptr);
    }
}

void DirectClientDispatcher::on_export_completed(Key session_key, int id, std::shared_ptr<Content> output)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (auto direct_client = find_no_lock(session_key))
    {
        direct_client->forward_job_completed(id, output);
    }
}

void DirectClientDispatcher::on_job_failed(Key session_key, int id, const std::exception& err)
{
    const char* msg = err.what();
    if (!msg)
    {
        msg = "Unknown error";
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    if (auto direct_client = find_no_lock(session_key))
    {
        direct_client->forward_job_failed(id, msg);
    }
}

DirectClient* DirectClientDispatcher::find_no_lock(Key session_key)
{
    auto found = boost::find_if(m_clients, [&](DirectClient* instance)
    {
        return instance->get_session_key() == session_key;
    });

    if (found == m_clients.end())
    {
        return nullptr;
    }
    return *found;
}

DirectClientDispatcher* find_direct_client_dispatcher(Core::Core& core)
{
    return static_cast<DirectClientDispatcher*>(core.find_event_dispatcher([](Core::EventDispatcher& dispatcher)
    {
        return typeid(dispatcher) == typeid(DirectClientDispatcher);
    }));    
}

DirectClientDispatcher& get_direct_client_dispatcher(Core::Core& core)
{
    auto direct_client_dispatcher = find_direct_client_dispatcher(core);
    if (!direct_client_dispatcher)
    { 
        throw std::logic_error("Direct client dispatcher not connected.");
    }
    return *direct_client_dispatcher;
}

void connect_direct_client_dispatcher(Core::Core& core)
{
    if (find_direct_client_dispatcher(core))
    {
        throw std::logic_error("Direct client dispatcher already connected.");
    }
    core.connect_event_dispatcher(std::make_unique<DirectClientDispatcher>());
}

DirectClient::DirectClient():
    m_core(nullptr),
    m_frontend(nullptr)
{   
}

DirectClient::DirectClient(EventTransporter event_transporter):
    Client(event_transporter),
    m_core(nullptr),
    m_frontend(nullptr) 
{       
}

DirectClient::~DirectClient()
{
    if (is_connected())
    {
        auto& dispatcher = get_direct_client_dispatcher(*m_core);
        dispatcher.remove_direct_client(this);
        
        m_frontend->close_session(m_session_key);
    }
}

void DirectClient::connect(Core::Core& core, const std::wstring& user)
{
    _ASSERT(!is_connected()); //please, call connect once only

    m_session_key = core.get_frontend().open_session(user);
    m_core = &core;
    m_frontend = &core.get_frontend();
    
    auto& dispatcher = get_direct_client_dispatcher(core);
    dispatcher.add_direct_client(this);
}

bool DirectClient::is_connected() const
{
    return m_core != nullptr;
}

std::vector<UserInfo> DirectClient::get_users() const
{
    return m_frontend->get_users(m_session_key);
}

Key DirectClient::add_user(const UserSettings& user_settings)
{
    return m_frontend->add_user(m_session_key, user_settings);
}

void DirectClient::update_user(Key user_key, const UserSettings& user_settings)
{
    m_frontend->update_user(m_session_key, user_key, user_settings);
}

void DirectClient::remove_user(Key user_key)
{
    m_frontend->remove_user(m_session_key, user_key);
}

SourceAccessInfo DirectClient::get_access_info(SourceKey source_key) const
{
    return m_frontend->get_access_info(m_session_key, source_key);
}

std::vector<AccessEntry> DirectClient::get_access_entries(SourceKey source_key) const
{
    return m_frontend->get_access_entries(m_session_key, source_key);
}

void DirectClient::set_access_list(const SourceAccessList& source_access_list) const
{
    m_frontend->set_access_list(m_session_key, source_access_list);
}

SummaryInfo DirectClient::get_summary_info() const
{
    return m_frontend->get_summary_info(m_session_key);
}

std::vector<ReportTypeInfo> DirectClient::get_report_types_info() const
{
    return m_frontend->get_report_types_info(m_session_key);
}

ReportTypeInfo DirectClient::get_report_type_info(ReportTypeUUID report_type_uuid) const
{
    return m_frontend->get_report_type_info(m_session_key, report_type_uuid);
}

std::shared_ptr<const XmlDoc> DirectClient::get_content_def(ReportTypeUUID report_type_uuid) const
{
    return m_frontend->get_content_def(m_session_key, report_type_uuid);
}

std::vector<StreamTypeInfo> DirectClient::get_stream_types_info() const
{
    return m_frontend->get_stream_types_info(m_session_key);
}

StreamTypeInfo DirectClient::get_stream_type_info(StreamTypeUUID stream_type_uuid) const
{
    return m_frontend->get_stream_type_info(m_session_key, stream_type_uuid);
}

std::shared_ptr<const XmlDefDoc> DirectClient::get_message_def(StreamTypeUUID stream_type_uuid) const
{
    return m_frontend->get_message_def(m_session_key, stream_type_uuid);
}

std::vector<ActionInfo> DirectClient::get_actions_info() const
{
    return m_frontend->get_actions_info(m_session_key);
}

ActionInfo DirectClient::get_action_info(ActionUUID action_uuid) const
{
    return m_frontend->get_action_info(m_session_key, action_uuid);
}

Params DirectClient::get_default_params(ActionUUID action_uuid) const
{
    return m_frontend->get_default_params(m_session_key, action_uuid);
}

Params DirectClient::create_params(ActionUUID action_uuid, const std::string& params_xml) const
{
    return m_frontend->create_params(m_session_key, action_uuid, params_xml);
}

std::vector<SourceTypeInfo> DirectClient::get_source_types_info() const
{
    return m_frontend->get_source_types_info(m_session_key);
}

SourceTypeInfo DirectClient::get_source_type_info(UUID source_type_uuid) const
{
    return m_frontend->get_source_type_info(m_session_key, source_type_uuid);
}

std::shared_ptr<const XmlDefDoc> DirectClient::get_config_def(UUID source_type_uuid) const
{
    return m_frontend->get_config_def(m_session_key, source_type_uuid);
}

SourceConfig DirectClient::get_default_config(UUID source_type_uuid) const
{
    return m_frontend->get_default_config(m_session_key, source_type_uuid);
}

// roles info
std::vector<RoleInfo> DirectClient::get_roles_info() const
{
    return m_frontend->get_roles_info(m_session_key);
}

RoleInfo DirectClient::get_role_info(RoleKey role_key) const
{
    return m_frontend->get_role_info(m_session_key, role_key);
}

RoleKey DirectClient::add_role(const RoleSettings& role_settings) const
{
    return m_frontend->add_role(m_session_key, role_settings);
}

void DirectClient::update_role(RoleKey role_key, const RoleSettings& role_settings) const
{
    m_frontend->update_role(m_session_key, role_key, role_settings);
}

void DirectClient::remove_role(RoleKey role_key)
{
    m_frontend->remove_role(m_session_key, role_key);
}

std::vector<SourceInfo> DirectClient::get_sources_info() const
{
    return m_frontend->get_sources_info(m_session_key);
}

SourceInfo DirectClient::get_source_info(SourceKey source_key) const
{
    return m_frontend->get_source_info(m_session_key, source_key);
}

SourceKey DirectClient::add_source(const SourceSettings& source_settings)
{
    return m_frontend->add_source(m_session_key, source_settings);
}

void DirectClient::update_source(SourceKey source_key, const SourceSettings& source_settings)
{
    m_frontend->update_source(m_session_key, source_key, source_settings);
}

void DirectClient::remove_source(SourceKey source_key)
{
    m_frontend->remove_source(m_session_key, source_key);
}

SourceConfig DirectClient::get_source_config(SourceKey source_key) const
{
    return m_frontend->get_source_config(m_session_key, source_key);
}

void DirectClient::set_source_config(SourceKey source_key, SourceConfig config)
{
    m_frontend->set_source_config(m_session_key, source_key, std::move(config));
}

std::vector<Link> DirectClient::get_links() const
{
    return m_frontend->get_links(m_session_key);
}

void DirectClient::add_link(Link link)
{
    m_frontend->add_link(m_session_key, link);
}

void DirectClient::remove_link(Link link)
{
    m_frontend->remove_link(m_session_key, link);
}

CurrentReportInfo DirectClient::get_current_report(SourceKey source_key, ReportTypeUUID report_type_uuid) const
{
    return m_frontend->get_current_report(m_session_key, source_key, report_type_uuid);
}

std::shared_ptr<Content> DirectClient::get_current_content(SourceKey source_key, ReportTypeUUID report_type_uuid) const
{
    return m_frontend->get_current_content(m_session_key, source_key, report_type_uuid);
}

std::vector<ArchivedReportInfo> DirectClient::get_archive(SourceKey source_key) const
{
    return m_frontend->get_archive(m_session_key, source_key);
}

ArchivedReportInfo DirectClient::get_archived_report(ArchivedReportKey archived_report_key) const
{
    return m_frontend->get_archived_report(m_session_key, archived_report_key);
}

std::shared_ptr<Content> DirectClient::get_archived_content(ArchivedReportKey archived_report_key) const
{
    return m_frontend->get_archived_content(m_session_key, archived_report_key);
}

StreamInfo DirectClient::get_stream_info(SourceKey source_key, StreamTypeUUID stream_type_uuid) const
{
    return m_frontend->get_stream_info(m_session_key, source_key, stream_type_uuid);
}

SubjectReportOptions DirectClient::get_options(SubjectKey subject_key, ReportTypeUUID report_type_uuid) const
{
    return m_frontend->get_options(m_session_key, subject_key, report_type_uuid);
}

SubjectStreamOptions DirectClient::get_options(SubjectKey subject_key, StreamTypeUUID stream_type_uuid) const
{
    return m_frontend->get_options(m_session_key, subject_key, stream_type_uuid);
}

SubjectActionOptions DirectClient::get_options(SubjectKey subject_key, ActionUUID action_uuid, boost::optional<UUID> shortcut_uuid) const
{
    return m_frontend->get_options(m_session_key, subject_key, action_uuid, shortcut_uuid);
}

void DirectClient::set_options(SubjectKey subject_key, const ReportOptions& report_options)
{
    m_frontend->set_options(m_session_key, subject_key, report_options);
}

void DirectClient::set_options(SubjectKey subject_key, const StreamOptions& stream_options)
{
    m_frontend->set_options(m_session_key, subject_key, stream_options);
}

void DirectClient::set_options(SubjectKey subject_key, const ActionOptions& shortcut_options)
{
    m_frontend->set_options(m_session_key, subject_key, shortcut_options);
}

SubjectOptions DirectClient::get_subject_options(SubjectKey subject_key) const
{
    if (auto source_key = boost::get<SourceKey>(&subject_key))
    {
        return m_frontend->get_source_options(m_session_key, *source_key);
    }

    auto role_key = boost::get<RoleKey>(subject_key);
    return m_frontend->get_role_options(m_session_key, role_key);
}

ReportTypeOptions DirectClient::get_report_type_options(ReportTypeUUID report_type_uuid) const
{
    return m_frontend->get_report_type_options(m_session_key, report_type_uuid);
}

StreamTypeOptions DirectClient::get_stream_type_options(StreamTypeUUID stream_type_uuid) const
{
    return m_frontend->get_stream_type_options(m_session_key, stream_type_uuid);
}

ActionTypeOptions DirectClient::get_action_options(ActionUUID action_uuid, boost::optional<UUID> shortcut_uuid) const
{
    return m_frontend->get_action_options(m_session_key, action_uuid, shortcut_uuid);
}

CustomQueryInfo DirectClient::get_custom_query_info(Key custom_query_key) const
{
    return m_frontend->get_custom_query_info(m_session_key, custom_query_key);
}

CustomQueryDefinition   DirectClient::get_custom_query_def(Key custom_query_key) const
{
    return m_frontend->get_custom_query_def(m_session_key, custom_query_key);
}

CustomQueryDefinition DirectClient::create_custom_query(std::vector<ReportTypeUUID> input_uuids) const
{
    return m_frontend->create_custom_query(m_session_key, std::move(input_uuids));
}

Key DirectClient::add_custom_query(std::wstring name, CustomQueryDefinition custom_query_def)
{
    return m_frontend->add_custom_query(m_session_key, std::move(name), std::move(custom_query_def));
}

void DirectClient::remove_custom_query(Key custom_query_key)
{
    m_frontend->remove_custom_query(m_session_key, custom_query_key);
}

CustomFilterInfo DirectClient::get_custom_filter_info(Key custom_filter_key) const
{
    return m_frontend->get_custom_filter_info(m_session_key, custom_filter_key);
}

Key DirectClient::add_custom_filter(std::wstring name, ReportTypeUUID input_uuid, CustomFilterSetup custom_filter_setup)
{
    return m_frontend->add_custom_filter(m_session_key, std::move(name), input_uuid, std::move(custom_filter_setup));
}

CustomFilterSetup DirectClient::create_filter_setup(ReportTypeUUID input_uuid) const
{
    return m_frontend->create_filter_setup(m_session_key, input_uuid);
}

CustomFilterSetup DirectClient::get_filter_setup(Key custom_filter_key) const
{
    return m_frontend->get_filter_setup(m_session_key, custom_filter_key);
}

void DirectClient::set_filter_setup(Key custom_filter_key, CustomFilterSetup custom_filter_setup) const
{
    m_frontend->set_filter_setup(m_session_key, custom_filter_key, std::move(custom_filter_setup));
}

TrackingStreamInfo DirectClient::get_tracking_stream_info(Key tracking_stream_key) const
{
    return m_frontend->get_tracking_stream_info(m_session_key, tracking_stream_key);
}

Key DirectClient::add_tracking_stream(std::wstring name, ReportTypeUUID input_uuid, std::string input_xpath)
{
    return m_frontend->add_tracking_stream(m_session_key, std::move(name), input_uuid, std::move(input_xpath));
}

StreamFilterInfo DirectClient::get_stream_filter_info(Key stream_filter_key) const
{
    return m_frontend->get_stream_filter_info(m_session_key, stream_filter_key);
}

StreamFilterSetup DirectClient::create_filter_setup(StreamTypeUUID input_uuid) const
{
    return m_frontend->create_filter_setup(m_session_key, input_uuid);
}

Key DirectClient::add_stream_filter(std::wstring name, StreamTypeUUID  input_uuid, StreamFilterSetup stream_filter_setup)
{
    return m_frontend->add_stream_filter(m_session_key, std::move(name), input_uuid, stream_filter_setup);
}

StreamFilterSetup DirectClient::get_stream_filter_setup(Key stream_filter_key) const
{
    return m_frontend->get_stream_filter_setup(m_session_key, stream_filter_key);
}

void DirectClient::set_stream_filter_setup(Key stream_filter_key, StreamFilterSetup stream_filter_setup)
{
    return m_frontend->set_stream_filter_setup(m_session_key, stream_filter_key, stream_filter_setup);
}

std::vector<ComplianceInfo> DirectClient::get_compliances_info() const
{
    return m_frontend->get_compliances_info(m_session_key);
}

ComplianceInfo DirectClient::get_compliance_info(ComplianceKey compliance_key) const
{
    return m_frontend->get_compliance_info(m_session_key, compliance_key);
}

ComplianceSetup DirectClient::get_compliance_setup(SubjectKey subject_key, ComplianceKey compliance_key) const
{
    return m_frontend->get_compliance_setup(m_session_key, subject_key, compliance_key);
}

void DirectClient::set_compliance_setup(SubjectKey subject_key, ComplianceKey compliance_key, ComplianceSetup compliance_setup)
{
    m_frontend->set_compliance_setup(m_session_key, subject_key, compliance_key, std::move(compliance_setup));
}
        
ValidationInfo DirectClient::get_validation_info(ValidationKey validation_key) const
{
    return m_frontend->get_validation_info(m_session_key, validation_key);
}

ValidationKey DirectClient::add_validation(std::wstring name, ReportTypeUUID input_uuid, ValidationSettings default_settings) const
{
    return m_frontend->add_validation(m_session_key, std::move(name), input_uuid, std::move(default_settings));
}

ValidationSettings DirectClient::create_validation_settings(ReportTypeUUID input_uuid) const
{
    return m_frontend->create_validation_settings(m_session_key, input_uuid);
}

ValidationSettings DirectClient::get_validation_settings(SubjectKey subject_key, ValidationKey validation_key) const
{
    return m_frontend->get_validation_settings(m_session_key, subject_key, validation_key);
}

void DirectClient::set_validation_settings(SubjectKey subject_key, ValidationKey validation_key, ValidationSettings validation_settings)
{
    m_frontend->set_validation_settings(m_session_key, subject_key, validation_key, std::move(validation_settings));
}

GroupingInfo DirectClient::get_grouping_info(Key grouping_key) const
{
    return m_frontend->get_grouping_info(m_session_key, grouping_key);
}

Key DirectClient::add_grouping(std::wstring name, ReportTypeUUID input_uuid)
{
    return m_frontend->add_grouping(m_session_key, std::move(name), input_uuid);
}

void DirectClient::remove_custom_report(ReportTypeUUID report_type_uuid, bool recursive)
{
    m_frontend->remove_custom_report(m_session_key, report_type_uuid, recursive);
}

std::shared_ptr<Content> DirectClient::create_pattern(ReportTypeUUID report_type_uuid) const
{
    return m_frontend->create_pattern(m_session_key, report_type_uuid);
}

std::shared_ptr<Content> DirectClient::create_pattern(ReportTypeUUID report_type_uuid, Blob blob) const
{
    return m_frontend->create_pattern(m_session_key, report_type_uuid, std::move(blob));
}

std::shared_ptr<Content> DirectClient::get_pattern(SubjectKey subject_key, ReportTypeUUID report_type_uuid) const
{
    return m_frontend->get_pattern(m_session_key, subject_key, report_type_uuid);
}

std::shared_ptr<Content> DirectClient::get_effective_pattern(SubjectKey subject_key, ReportTypeUUID report_type_uuid) const
{
    return m_frontend->get_effective_pattern(m_session_key, subject_key, report_type_uuid);
}

void DirectClient::set_pattern(SubjectKey subject_key, ReportTypeUUID report_type_uuid, Blob blob)
{
    m_frontend->set_pattern(m_session_key, subject_key, report_type_uuid, std::move(blob));
}

void DirectClient::reset_pattern(SubjectKey subject_key, ReportTypeUUID report_type_uuid)
{
    m_frontend->reset_pattern(m_session_key, subject_key, report_type_uuid);
}

UUID DirectClient::add_custom_action(std::wstring name, ActionUUID action_uuid, Params params)
{
    return m_frontend->add_custom_action(m_session_key, std::move(name), action_uuid, std::move(params));
}

void DirectClient::remove_custom_action(UUID custom_action_uuid)
{
    m_frontend->remove_custom_action(m_session_key, custom_action_uuid);
}

std::vector<ScheduleInfo> DirectClient::get_schedules_info() const
{
    return m_frontend->get_schedules_info(m_session_key);
}

ScheduleInfo    DirectClient::get_schedule_info(Key schedule_key) const
{
    return m_frontend->get_schedule_info(m_session_key, schedule_key);
}

Key DirectClient::add_schedule(const ScheduleSettings& schedule_settings)
{
    return m_frontend->add_schedule(m_session_key, schedule_settings);
}

void DirectClient::update_schedule(Key schedule_key, const ScheduleSettings& schedule_settings)
{
    m_frontend->update_schedule(m_session_key, schedule_key, schedule_settings);
}

void DirectClient::remove_schedule(Key schedule_key)
{
    m_frontend->remove_schedule(m_session_key, schedule_key);
}

void DirectClient::reload(SourceKey source_key)
{
    m_frontend->reload(m_session_key, source_key);
}

std::vector<FeatureInfo> DirectClient::get_features_info() const
{
    return m_frontend->get_features_info(m_session_key);
}

FeatureInfo DirectClient::get_feature_info(UUID feature_uuid) const
{
    return m_frontend->get_feature_info(m_session_key, feature_uuid);
}

void DirectClient::install_feature(UUID feature_uuid)
{
    m_frontend->install_feature(m_session_key, feature_uuid);
}

void DirectClient::uninstall_feature(UUID feature_uuid)
{
    m_frontend->uninstall_feature(m_session_key, feature_uuid);
}

void DirectClient::do_cancel_job(int id) const
{
    m_frontend->cancel_job(m_session_key, id);
}

int DirectClient::do_query_current_content(SourceKey source_key, ReportTypeUUID report_type_uuid) const
{
    return m_frontend->query_current_content(m_session_key, source_key, report_type_uuid);
}

int DirectClient::do_query_archived_content(SourceKey source_key, ReportTypeUUID report_type_uuid, time_t time) const
{
    return m_frontend->query_archived_content(m_session_key, source_key, report_type_uuid, time);
}

int DirectClient::do_query_stream_segment(SourceKey source_key, StreamTypeUUID stream_type_uuid, time_t start, time_t stop) const
{
    return m_frontend->query_stream_segment(m_session_key, source_key, stream_type_uuid, start, stop);
}

int DirectClient::do_reload_report(SourceKey source_key, ReportTypeUUID report_type_uuid) const
{
    return m_frontend->reload_report(m_session_key, source_key, report_type_uuid);
}   

int DirectClient::do_rebuild_report(SourceKey source_key, ReportTypeUUID report_type_uuid) const
{
    return m_frontend->rebuild_report(m_session_key, source_key, report_type_uuid);
}

int DirectClient::do_launch_action(SourceKey source_key, ActionUUID action_uuid, Params params) const
{
    return m_frontend->launch_action(m_session_key, source_key, action_uuid, std::move(params));
}

int DirectClient::do_launch_shortcut(SourceKey source_key, UUID shortcut_uuid) const
{
    return m_frontend->launch_shortcut(m_session_key, source_key, shortcut_uuid);
}

int DirectClient::do_export_summary() const
{
    return m_frontend->export_summary(m_session_key);
}

} //namespace TR {