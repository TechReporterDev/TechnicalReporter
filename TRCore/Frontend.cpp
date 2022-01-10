#include "stdafx.h"
#include "Frontend.h"
#include "PlainFacade.h"
#include "CoreImpl.h"
#include "Security.h"
#include "Scheduler.h"
#include "Features.h"
#include "Async.h"
namespace TR { namespace Core {

Frontend::Frontend(CoreImpl& core_impl):
    m_core_impl(core_impl),
    m_executive(*core_impl.m_executive),
    m_facade(std::make_unique<PlainFacade>(&core_impl)),
    m_security(&core_impl.m_security),
    m_features(nullptr),
    m_async(nullptr)
{
}

Frontend::~Frontend()
{
}

void Frontend::set_features(Features* features)
{
    _ASSERT(!m_features);
    m_features = features;
}

void Frontend::set_async(Async* async)
{
    _ASSERT(!m_async);
    m_async = async;
}

Key Frontend::open_session(const std::wstring& user_name)
{
    return m_executive.exec([&]{
        return m_security->open_session(user_name);
    });
}

void Frontend::close_session(Key session_key)
{
    m_executive.exec([&]{
        m_security->close_session(session_key);
    });
}

void Frontend::cancel_job(Key session_key, int id){
    m_executive.exec([&]{
        m_async->cancel(session_key, id);
    });
}

std::vector<UserInfo> Frontend::get_users(Key session_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_users();
    });
}

Key Frontend::add_user(Key session_key, const UserSettings& user_settings)
{
    return m_executive.exec([&]{
        return m_facade->add_user(user_settings);
    });
}

void Frontend::update_user(Key session_key, Key user_key, const UserSettings& user_settings)
{
    m_executive.exec([&]{
        m_facade->update_user(user_key, user_settings);
    });
}

void Frontend::remove_user(Key session_key, Key user_key)
{
    m_executive.exec([&]{
        m_facade->remove_user(user_key);
    });
}

SourceAccessInfo Frontend::get_access_info(Key session_key, SourceKey source_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_access_info(source_key);
    });
}

std::vector<AccessEntry> Frontend::get_access_entries(Key session_key, SourceKey source_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_access_entries(source_key);
    });
}

void Frontend::set_access_list(Key session_key, const SourceAccessList& source_access_list) const
{
    m_executive.exec([&]{
        return m_facade->set_access_list(source_access_list);
    });
}

bool Frontend::check_access_rights(Key session_key, SourceKey source_key, TR::AccessRights access_rights) const
{
    auto session = m_security->get_session(session_key);
    auto source = m_core_impl.m_core_domain.m_registry.m_sources.get_source(source_key);
    return m_security->check_access_rights(session.m_user_key, source, Access(access_rights.get_value()));
}

SummaryInfo Frontend::get_summary_info(Key session_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_summary_info();
    });
}

std::vector<ReportTypeInfo> Frontend::get_report_types_info(Key session_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_report_types_info();
    });
}

ReportTypeInfo Frontend::get_report_type_info(Key session_key, ReportTypeUUID report_type_uuid) const
{
    return m_executive.exec([&]{
        return m_facade->get_report_type_info(report_type_uuid);
    });
}

std::shared_ptr<const XmlDoc> Frontend::get_content_def(Key session_key, ReportTypeUUID report_type_uuid) const
{
    return m_executive.exec([&]{
        return m_facade->get_content_def(report_type_uuid);
    });
}

std::vector<StreamTypeInfo> Frontend::get_stream_types_info(Key session_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_stream_types_info();
    });
}

StreamTypeInfo Frontend::get_stream_type_info(Key session_key, StreamTypeUUID stream_type_uuid) const
{
    return m_executive.exec([&]{
        return m_facade->get_stream_type_info(stream_type_uuid);
    });
}

std::shared_ptr<const XmlDefDoc> Frontend::get_message_def(Key session_key, StreamTypeUUID stream_type_uuid) const
{
    return m_executive.exec([&]{
        return m_facade->get_message_def(stream_type_uuid);
    });
}

std::vector<ActionInfo> Frontend::get_actions_info(Key session_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_actions_info();
    });
}

ActionInfo Frontend::get_action_info(Key session_key, ActionUUID action_uuid) const
{
    return m_executive.exec([&]{
        return m_facade->get_action_info(action_uuid);
    });
}

Params Frontend::get_default_params(Key session_key, ActionUUID action_uuid) const
{
    return m_executive.exec([&]{
        return m_facade->get_default_params(action_uuid);
    });
}

Params Frontend::create_params(Key session_key, ActionUUID action_uuid, const std::string& params_xml) const
{
    return m_executive.exec([&]{
        return m_facade->create_params(action_uuid, params_xml);
    });
}

std::vector<SourceTypeInfo> Frontend::get_source_types_info(Key session_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_source_types_info();
    });
}

SourceTypeInfo Frontend::get_source_type_info(Key session_key, UUID source_type_uuid) const
{
    return m_executive.exec([&]{
        return m_facade->get_source_type_info(source_type_uuid);
    });
}

std::shared_ptr<const XmlDefDoc> Frontend::get_config_def(Key session_key, UUID source_type_uuid) const
{
    return m_executive.exec([&]{
        return m_facade->get_config_def(source_type_uuid);
    });
}

SourceConfig Frontend::get_default_config(Key session_key, UUID source_type_uuid) const
{
    return m_executive.exec([&]{
        return m_facade->get_default_config(source_type_uuid);
    });
}

std::vector<RoleInfo> Frontend::get_roles_info(Key session_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_roles_info();
    });
}

RoleInfo Frontend::get_role_info(Key session_key, RoleKey role_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_role_info(role_key);
    });
}

RoleKey Frontend::add_role(Key session_key, const RoleSettings& role_settings) const
{
    return m_executive.exec([&]{
        return m_facade->add_role(role_settings);
    });
}

void Frontend::update_role(Key session_key, RoleKey role_key, const RoleSettings& role_settings) const
{
    return m_executive.exec([&]{
        return m_facade->update_role(role_key, role_settings);
    });
}

void Frontend::remove_role(Key session_key, RoleKey role_key)
{
    return m_executive.exec([&]{
        return m_facade->remove_role(role_key);
    });
}

std::vector<SourceInfo> Frontend::get_sources_info(Key session_key) const
{
    return m_executive.exec([&]{
        std::vector<SourceInfo> sources_info;
        for (auto& source_info : m_facade->get_sources_info())
        {
            if (!check_access_rights(session_key, source_info.m_key, TR::Access::READ_ACCESS))
            {
                continue;
            }
            sources_info.push_back(std::move(source_info));
        }
        return sources_info;
    });
}

SourceInfo Frontend::get_source_info(Key session_key, SourceKey source_key) const
{
    return m_executive.exec([&]{
        if (!check_access_rights(session_key, source_key, TR::Access::READ_ACCESS))
        {
            throw Exception(L"Access denied");
        }

        return m_facade->get_source_info(source_key);
    });
}

SourceKey Frontend::add_source(Key session_key, const SourceSettings& source_settings)
{
    return m_executive.exec([&]{
        if (!check_access_rights(session_key, source_settings.m_parent_key, TR::Access::WRITE_ACCESS))
        {
            throw Exception(L"Access denied");
        }
        return m_facade->add_source(source_settings);
    });
}

void Frontend::update_source(Key session_key, SourceKey source_key, const SourceSettings& source_settings)
{
    return m_executive.exec([&]{
        if (!check_access_rights(session_key, source_key, TR::Access::WRITE_ACCESS))
        {
            throw Exception(L"Access denied");
        }

        m_facade->update_source(source_key, source_settings);
    });
}

void Frontend::remove_source(Key session_key, SourceKey source_key)
{
    return m_executive.exec([&]{
        if (!check_access_rights(session_key, source_key, TR::Access::WRITE_ACCESS))
        {
            throw Exception(L"Access denied");
        }

        m_facade->remove_source(source_key);
    });
}

SourceConfig Frontend::get_source_config(Key session_key, SourceKey source_key) const
{
    return m_executive.exec([&]{
        if (!check_access_rights(session_key, source_key, TR::Access::READ_ACCESS))
        {
            throw Exception(L"Access denied");
        }

        return m_facade->get_source_config(source_key);
    });
}

void Frontend::set_source_config(Key session_key, SourceKey source_key, SourceConfig config)
{
    return m_executive.exec([&]{
        if (!check_access_rights(session_key, source_key, TR::Access::WRITE_ACCESS))
        {
            throw Exception(L"Access denied");
        }

        m_facade->set_source_config(source_key, std::move(config));
    });
}

std::vector<TR::Link> Frontend::get_links(Key session_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_links();
    });
}

void Frontend::add_link(Key session_key, TR::Link link)
{
    m_executive.exec([&]{
        m_facade->add_link(link);
    });
}

void Frontend::remove_link(Key session_key, TR::Link link)
{
    m_executive.exec([&]{
        m_facade->remove_link(link);
    });
}

CurrentReportInfo Frontend::get_current_report(Key session_key, SourceKey source_key, ReportTypeUUID report_type_uuid) const
{
    return m_executive.exec([&]{
        if (!check_access_rights(session_key, source_key, TR::Access::READ_ACCESS))
        {
            throw Exception(L"Access denied");
        }

        return m_facade->get_current_report_info(source_key, report_type_uuid);
    });
}

std::shared_ptr<Content> Frontend::get_current_content(Key session_key, SourceKey source_key, ReportTypeUUID report_type_uuid) const
{
    return m_executive.exec([&]{
        if (!check_access_rights(session_key, source_key, TR::Access::READ_ACCESS))
        {
            throw Exception(L"Access denied");
        }

        return m_facade->get_current_content(source_key, report_type_uuid);
    });
}

int Frontend::query_current_content(Key session_key, SourceKey source_key, ReportTypeUUID report_type_uuid) const
{
    return m_executive.exec([&]{
        if (!check_access_rights(session_key, source_key, TR::Access::READ_ACCESS))
        {
            throw Exception(L"Access denied");
        }
        return m_async->query_current_content(session_key, source_key, report_type_uuid);
    });
}

std::vector<ArchivedReportInfo> Frontend::get_archive(Key session_key, SourceKey source_key) const
{
    return m_executive.exec([&]{
        if (!check_access_rights(session_key, source_key, TR::Access::READ_ACCESS))
        {
            throw Exception(L"Access denied");
        }

        return m_facade->get_archive(source_key);
    });
}

ArchivedReportInfo Frontend::get_archived_report(Key session_key, ArchivedReportKey archived_report_key) const
{
    return m_executive.exec([&]{
        auto archived_report = m_facade->get_archived_report(archived_report_key);
        if (!check_access_rights(session_key, archived_report.m_source_key, TR::Access::READ_ACCESS))
        {
            throw Exception(L"Access denied");
        }

        return archived_report;
    });
}

std::shared_ptr<Content> Frontend::get_archived_content(Key session_key, ArchivedReportKey archived_report_key) const
{
    return m_executive.exec([&]{
        auto archived_report = m_facade->get_archived_report(archived_report_key);
        if (!check_access_rights(session_key, archived_report.m_source_key, TR::Access::READ_ACCESS))
        {
            throw Exception(L"Access denied");
        }

        return m_facade->get_archived_content(archived_report_key);
    });
}

int Frontend::query_archived_content(Key session_key, SourceKey source_key, UUID report_type_uuid, time_t time) const
{
    return m_executive.exec([&]{
        if (!check_access_rights(session_key, source_key, TR::Access::READ_ACCESS))
        {
            throw Exception(L"Access denied");
        }

        return m_async->query_archived_content(session_key, source_key, ReportTypeUUID(report_type_uuid), time);
    });
}

StreamInfo Frontend::get_stream_info(Key session_key, SourceKey source_key, StreamTypeUUID stream_type_uuid) const
{
    return m_executive.exec([&]{
        if (!check_access_rights(session_key, source_key, TR::Access::READ_ACCESS))
        {
            throw Exception(L"Access denied");
        }

        return m_facade->get_stream_info(source_key, stream_type_uuid);
    });
}

int Frontend::query_stream_segment(Key session_key, SourceKey source_key, StreamTypeUUID stream_type_uuid, time_t start, time_t stop) const
{
    return m_executive.exec([&]{
        if (!check_access_rights(session_key, source_key, TR::Access::READ_ACCESS))
        {
            throw Exception(L"Access denied");
        }
        return m_async->query_stream_segment(session_key, source_key, stream_type_uuid, start, stop);
    });
}

SubjectReportOptions Frontend::get_options(Key session_key, SubjectKey subject_key, ReportTypeUUID report_type_uuid) const
{
    return m_executive.exec([&]{    
        if (auto source_key = boost::get<SourceKey>(&subject_key))
        {
            if (!check_access_rights(session_key, *source_key, TR::Access::READ_ACCESS))
            {
                throw Exception(L"Access denied");
            }
        }

        auto session = m_security->get_session(session_key);
        return m_facade->get_options(session.m_user_key, subject_key, report_type_uuid);
    });
}

SubjectStreamOptions Frontend::get_options(Key session_key, SubjectKey subject_key, StreamTypeUUID stream_type_uuid) const
{
    return m_executive.exec([&]{
        if (auto source_key = boost::get<SourceKey>(&subject_key))
        {
            if (!check_access_rights(session_key, *source_key, TR::Access::READ_ACCESS))
            {
                throw Exception(L"Access denied");
            }
        }
        auto session = m_security->get_session(session_key);
        return m_facade->get_options(session.m_user_key, subject_key, stream_type_uuid);
    });
}

SubjectActionOptions Frontend::get_options(Key session_key, SubjectKey subject_key, ActionUUID action_uuid, boost::optional<UUID> shortcut_uuid) const
{
    return m_executive.exec([&]{
        if (auto source_key = boost::get<SourceKey>(&subject_key))
        {
            if (!check_access_rights(session_key, *source_key, TR::Access::READ_ACCESS))
            {
                throw Exception(L"Access denied");
            }
        }
        return m_facade->get_options(subject_key, action_uuid, shortcut_uuid);
    });
}

void Frontend::set_options(Key session_key, SubjectKey subject_key, const ReportOptions& report_options)
{
    m_executive.exec([&]{
        if (auto source_key = boost::get<SourceKey>(&subject_key))
        {
            if (!check_access_rights(session_key, *source_key, TR::Access::WRITE_ACCESS))
            {
                throw Exception(L"Access denied");
            }
        }

        auto session = m_security->get_session(session_key);
        return m_facade->set_options(session.m_user_key, subject_key, report_options);
    });
}

void Frontend::set_options(Key session_key, SubjectKey subject_key, const StreamOptions& stream_options)
{
    m_executive.exec([&]{
        if (auto source_key = boost::get<SourceKey>(&subject_key))
        {
            if (!check_access_rights(session_key, *source_key, TR::Access::WRITE_ACCESS))
            {
                throw Exception(L"Access denied");
            }
        }

        auto session = m_security->get_session(session_key);
        return m_facade->set_options(session.m_user_key, subject_key, stream_options);
    });
}

void Frontend::set_options(Key session_key, SubjectKey subject_key, const ActionOptions& action_options)
{
    m_executive.exec([&]{
        if (auto source_key = boost::get<SourceKey>(&subject_key))
        {
            if (!check_access_rights(session_key, *source_key, TR::Access::WRITE_ACCESS))
            {
                throw Exception(L"Access denied");
            }
        }

        return m_facade->set_options(subject_key, action_options);
    });
}

SubjectOptions Frontend::get_role_options(Key session_key, RoleKey role_key) const
{
    return m_executive.exec([&]{
        auto session = m_security->get_session(session_key);
        return m_facade->get_role_options(session.m_user_key, role_key);
    });
}

SubjectOptions Frontend::get_source_options(Key session_key, SourceKey source_key) const
{
    return m_executive.exec([&]{
        if (!check_access_rights(session_key, source_key, TR::Access::READ_ACCESS))
        {
            throw Exception(L"Access denied");
        }

        auto session = m_security->get_session(session_key);
        return m_facade->get_source_options(session.m_user_key, source_key);
    });
}

ReportTypeOptions Frontend::get_report_type_options(Key session_key, ReportTypeUUID report_type_uuid) const
{
    return m_executive.exec([&]{
        auto session = m_security->get_session(session_key);
        return m_facade->get_report_type_options(session.m_user_key, report_type_uuid);
    });
}

StreamTypeOptions Frontend::get_stream_type_options(Key session_key, StreamTypeUUID stream_type_uuid) const
{
    return m_executive.exec([&]{
        auto session = m_security->get_session(session_key);
        return m_facade->get_stream_type_options(session.m_user_key, stream_type_uuid);
    });
}

ActionTypeOptions Frontend::get_action_options(Key session_key, ActionUUID action_uuid, boost::optional<UUID> shortcut_uuid) const
{
    return m_executive.exec([&]{
        return m_facade->get_action_options(action_uuid, shortcut_uuid);
    });
}

CustomQueryInfo Frontend::get_custom_query_info(Key session_key, Key custom_query_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_custom_query_info(custom_query_key);
    });
}

CustomQueryDefinition Frontend::get_custom_query_def(Key session_key, Key custom_query_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_custom_query_def(custom_query_key);
    });
}

CustomQueryDefinition Frontend::create_custom_query(Key session_key, std::vector<ReportTypeUUID> input_uuids) const
{
    return m_executive.exec([&]{
        return m_facade->create_custom_query(std::move(input_uuids));
    });
}

Key Frontend::add_custom_query(Key session_key, std::wstring name, CustomQueryDefinition custom_query_def)
{
    return m_executive.exec([&]{
        return m_facade->add_custom_query(std::move(name), std::move(custom_query_def));
    });
}

void Frontend::remove_custom_query(Key session_key, Key custom_query_key)
{
    return m_executive.exec([&]{
        return m_facade->remove_custom_query(custom_query_key);
    });
}

CustomFilterInfo Frontend::get_custom_filter_info(Key session_key, Key custom_filter_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_custom_filter_info(custom_filter_key);
    });
}

Key Frontend::add_custom_filter(Key session_key, std::wstring name, ReportTypeUUID input_uuid, CustomFilterSetup custom_filter_setup)
{
    return m_executive.exec([&]{
        return m_facade->add_custom_filter(std::move(name), input_uuid, std::move(custom_filter_setup));
    });
}

CustomFilterSetup Frontend::create_filter_setup(Key session_key, ReportTypeUUID input_uuid) const
{
    return m_executive.exec([&]{
        return m_facade->create_filter_setup(input_uuid);
    });
}

CustomFilterSetup Frontend::get_filter_setup(Key session_key, Key custom_filter_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_filter_setup(custom_filter_key);
    });
}

void Frontend::set_filter_setup(Key session_key, Key custom_filter_key, CustomFilterSetup custom_filter_setup)
{
    return m_executive.exec([&]{
        return m_facade->set_filter_setup(custom_filter_key, std::move(custom_filter_setup));
    });
}

TrackingStreamInfo Frontend::get_tracking_stream_info(Key session_key, Key tracking_stream_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_tracking_stream_info(tracking_stream_key);
    });
}

Key Frontend::add_tracking_stream(Key session_key, std::wstring name, ReportTypeUUID input_uuid, std::string input_xpath)
{
    return m_executive.exec([&]{
        return m_facade->add_tracking_stream(std::move(name), input_uuid, std::move(input_xpath));
    });
}

StreamFilterInfo Frontend::get_stream_filter_info(Key session_key, Key stream_filter_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_stream_filter_info(stream_filter_key);
    });
}

StreamFilterSetup Frontend::create_filter_setup(Key session_key, StreamTypeUUID input_uuid) const
{
    return m_executive.exec([&]{
        return m_facade->create_filter_setup(input_uuid);
    });
}

Key Frontend::add_stream_filter(Key session_key, std::wstring name, StreamTypeUUID input_uuid, StreamFilterSetup stream_filter_setup)
{
    return m_executive.exec([&]{
        return m_facade->add_stream_filter(std::move(name), input_uuid, stream_filter_setup);
    });
}

StreamFilterSetup Frontend::get_stream_filter_setup(Key session_key, Key stream_filter_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_stream_filter_setup(stream_filter_key);
    });
}

void Frontend::set_stream_filter_setup(Key session_key, Key stream_filter_key, StreamFilterSetup stream_filter_setup)
{
    return m_executive.exec([&]{
        return m_facade->set_stream_filter_setup(stream_filter_key, stream_filter_setup);
    });
}

std::vector<ComplianceInfo> Frontend::get_compliances_info(Key session_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_compliances_info();
    });
}

ComplianceInfo Frontend::get_compliance_info(Key session_key, ComplianceKey compliance_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_compliance_info(compliance_key);
    });
}

ComplianceSetup Frontend::get_compliance_setup(Key session_key, SubjectKey subject_key, ComplianceKey compliance_key) const
{
    return m_executive.exec([&]{
        if (auto source_key = boost::get<SourceKey>(&subject_key))
        {
            if (!check_access_rights(session_key, *source_key, TR::Access::READ_ACCESS))
            {
                throw Exception(L"Access denied");
            }
        }

        return m_facade->get_compliance_setup(subject_key, compliance_key);
    });
}

void Frontend::set_compliance_setup(Key session_key, SubjectKey subject_key, ComplianceKey compliance_key, ComplianceSetup compliance_setup)
{
    m_executive.exec([&]{
        if (auto source_key = boost::get<SourceKey>(&subject_key))
        {
            if (!check_access_rights(session_key, *source_key, TR::Access::WRITE_ACCESS))
            {
                throw Exception(L"Access denied");
            }
        }       

        m_facade->set_compliance_setup(subject_key, compliance_key, std::move(compliance_setup));
    });
}

ValidationInfo Frontend::get_validation_info(Key session_key, ValidationKey validation_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_validation_info(validation_key);
    });
}

ValidationKey Frontend::add_validation(Key session_key, std::wstring name, ReportTypeUUID input_uuid, ValidationSettings default_settings) const
{
    return m_executive.exec([&]{
        return m_facade->add_validation(std::move(name), input_uuid, std::move(default_settings));
    });
}

ValidationSettings Frontend::create_validation_settings(Key session_key, ReportTypeUUID input_uuid) const
{
    return m_executive.exec([&]{
        return m_facade->create_validation_settings(input_uuid);
    });
}

ValidationSettings Frontend::get_validation_settings(Key session_key, SubjectKey subject_key, ValidationKey validation_key) const
{
    return m_executive.exec([&]{
        if (auto source_key = boost::get<SourceKey>(&subject_key))
        {
            if (!check_access_rights(session_key, *source_key, TR::Access::READ_ACCESS))
            {
                throw Exception(L"Access denied");
            }
        }
        return m_facade->get_validation_settings(subject_key, validation_key);
    });
}

void Frontend::set_validation_settings(Key session_key, SubjectKey subject_key, ValidationKey validation_key, ValidationSettings validation_settings)
{
    m_executive.exec([&]{
        if (auto source_key = boost::get<SourceKey>(&subject_key))
        {
            if (!check_access_rights(session_key, *source_key, TR::Access::WRITE_ACCESS))
            {
                throw Exception(L"Access denied");
            }
        }

        m_facade->set_validation_settings(subject_key, validation_key, std::move(validation_settings));
    });
}

GroupingInfo Frontend::get_grouping_info(Key session_key, Key grouping_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_grouping_info(grouping_key);
    });
}

Key Frontend::add_grouping(Key session_key, std::wstring name, ReportTypeUUID input_uuid)
{
    return m_executive.exec([&]{
        return m_facade->add_grouping(std::move(name), input_uuid);
    });
}

void Frontend::remove_custom_report(Key session_key, ReportTypeUUID report_type_uuid, bool recursive)
{
    return m_executive.exec([&]{
        return m_facade->remove_custom_report(report_type_uuid, recursive);
    });
}

std::shared_ptr<Content> Frontend::create_pattern(Key session_key, ReportTypeUUID report_type_uuid) const
{
    return m_executive.exec([&]{
        return m_facade->create_pattern(report_type_uuid);
    });
}

std::shared_ptr<Content> Frontend::create_pattern(Key session_key, ReportTypeUUID report_type_uuid, Blob blob) const
{
    return m_executive.exec([&]{
        return m_facade->create_pattern(report_type_uuid, std::move(blob));
    });
}

std::shared_ptr<Content> Frontend::get_pattern(Key session_key, SubjectKey subject_key, ReportTypeUUID report_type_uuid) const
{
    return m_executive.exec([&]{
        if (auto source_key = boost::get<SourceKey>(&subject_key))
        {
            if (!check_access_rights(session_key, *source_key, TR::Access::READ_ACCESS))
            {
                throw Exception(L"Access denied");
            }
        }
        return m_facade->get_pattern(subject_key, report_type_uuid);
    });
}

std::shared_ptr<Content> Frontend::get_effective_pattern(Key session_key, SubjectKey subject_key, ReportTypeUUID report_type_uuid) const
{
    return m_executive.exec([&]{
        if (auto source_key = boost::get<SourceKey>(&subject_key))
        {
            if (!check_access_rights(session_key, *source_key, TR::Access::READ_ACCESS))
            {
                throw Exception(L"Access denied");
            }
        }
        return m_facade->get_effective_pattern(subject_key, report_type_uuid);
    });
}

void Frontend::set_pattern(Key session_key, SubjectKey subject_key, ReportTypeUUID report_type_uuid, Blob blob)
{
    m_executive.exec([&]{
        if (auto source_key = boost::get<SourceKey>(&subject_key))
        {
            if (!check_access_rights(session_key, *source_key, TR::Access::WRITE_ACCESS))
            {
                throw Exception(L"Access denied");
            }
        }
        return m_facade->set_pattern(subject_key, report_type_uuid, std::move(blob));
    });
}

void Frontend::reset_pattern(Key session_key, SubjectKey subject_key, ReportTypeUUID report_type_uuid)
{
    m_executive.exec([&]{
        if (auto source_key = boost::get<SourceKey>(&subject_key))
        {
            if (!check_access_rights(session_key, *source_key, TR::Access::WRITE_ACCESS))
            {
                throw Exception(L"Access denied");
            }
        }
        return m_facade->reset_pattern(subject_key, report_type_uuid);
    });
}

UUID Frontend::add_custom_action(Key session_key, std::wstring name, ActionUUID action_uuid, Params params)
{
    return m_executive.exec([&]{
        return m_facade->add_custom_action(std::move(name), action_uuid, std::move(params));
    });
}

void Frontend::remove_custom_action(Key session_key, UUID custom_action_uuid)
{
    m_executive.exec([&]{
        m_facade->remove_custom_action(custom_action_uuid);
    });
}

std::vector<ScheduleInfo> Frontend::get_schedules_info(Key session_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_schedules_info();
    });
}

ScheduleInfo    Frontend::get_schedule_info(Key session_key, Key schedule_key) const
{
    return m_executive.exec([&]{
        return m_facade->get_schedule_info(schedule_key);
    });
}

Key Frontend::add_schedule(Key session_key, const ScheduleSettings& schedule_settings)
{
    return m_executive.exec([&]{        
        return m_facade->add_schedule(schedule_settings);
    });
}

void Frontend::update_schedule(Key session_key, Key schedule_key, const ScheduleSettings& schedule_settings)
{
    return m_executive.exec([&]{        
        return m_facade->update_schedule(schedule_key, schedule_settings);
    });
}

void Frontend::remove_schedule(Key session_key, Key schedule_key)
{
    return m_executive.exec([&]{        
        return m_facade->remove_schedule(schedule_key);
    });
}

void Frontend::reload(Key session_key, SourceKey source_key)
{
    m_executive.exec([&]{
        if (!check_access_rights(session_key, source_key, TR::Access::EXEC_ACCESS))
        {
            throw Exception(L"Access denied");
        }

        return m_async->reload(session_key, source_key);
    });
}

int Frontend::reload_report(Key session_key, SourceKey source_key, ReportTypeUUID report_type_uuid)
{
    return m_executive.exec([&]{
        if (!check_access_rights(session_key, source_key, TR::Access::EXEC_ACCESS))
        {
            throw Exception(L"Access denied");
        }

        return m_async->reload_report(session_key, source_key, report_type_uuid);
    });
}

int Frontend::rebuild_report(Key session_key, SourceKey source_key, ReportTypeUUID report_type_uuid)
{
    return m_executive.exec([&]{
        if (!check_access_rights(session_key, source_key, TR::Access::EXEC_ACCESS))
        {
            throw Exception(L"Access denied");
        }

        return m_async->rebuild_report(session_key, source_key, report_type_uuid);
    });
}

int Frontend::launch_action(Key session_key, SourceKey source_key, ActionUUID action_uuid, Params params)
{
    return m_executive.exec([&]{
        if (!check_access_rights(session_key, source_key, TR::Access::EXEC_ACCESS))
        {
            throw Exception(L"Access denied");
        }

        return m_async->launch_action(session_key, source_key, action_uuid, std::move(params.m_params_doc));
    });
}

int Frontend::launch_shortcut(Key session_key, SourceKey source_key, UUID shortcut_uuid)
{
    return m_executive.exec([&]{
        if (!check_access_rights(session_key, source_key, TR::Access::EXEC_ACCESS))
        {
            throw Exception(L"Access denied");
        }

        return m_async->launch_shortcut(session_key, source_key, shortcut_uuid);
    });
}

int Frontend::export_summary(Key session_key)
{
    return m_executive.exec([&] {
        return m_async->export_summary(session_key);
    });
}

std::vector<FeatureInfo> Frontend::get_features_info(Key session_key) const
{
    return m_executive.exec([&]{
        std::vector<FeatureInfo> features_info;
        for (auto feature : m_features->get_features())
        {
            bool installed = m_features->is_installed(feature->get_uuid());
            features_info.push_back({feature->get_uuid(), feature->get_name(), feature->get_dependencies(), installed});
        }
        return features_info;
    });
}

FeatureInfo Frontend::get_feature_info(Key session_key, UUID feature_uuid) const
{
    return m_executive.exec([&]{
        auto& feature = m_features->get_feature(feature_uuid);
        bool installed = m_features->is_installed(feature.get_uuid());
        return FeatureInfo(feature.get_uuid(), feature.get_name(), feature.get_dependencies(), installed);
    });
}

void Frontend::install_feature(Key session_key, UUID feature_uuid)
{
    m_executive.exec([&]{
        m_features->install_feature(feature_uuid);
    });
}

void Frontend::uninstall_feature(Key session_key, UUID feature_uuid)
{
    m_executive.exec([&]{
        m_features->uninstall_feature(feature_uuid);
    });
}

}} //namespace TR { namespace Core {