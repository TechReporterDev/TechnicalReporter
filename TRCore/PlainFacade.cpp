#include "stdafx.h"
#include "PlainFacade.h"
#include "CoreImpl.h"
#include "DateTime.h"
namespace TR { namespace Core {

namespace {
TR::ReportTypeInfo::Syntax get_report_type_syntax(const ReportType& report_type)
{
    if (dynamic_cast<const ComplianceReportType*>(&report_type))
    {
        return TR::ReportTypeInfo::Syntax::COMPLIANCE_XML;
    }
    if (dynamic_cast<const CheckListReportType*>(&report_type))
    {
        return TR::ReportTypeInfo::Syntax::CHECKLIST_XML;
    }
    else if (dynamic_cast<const ValidatedReportType*>(&report_type))
    {
        return TR::ReportTypeInfo::Syntax::VALIDATED_XML;
    }
    else if (dynamic_cast<const RegularReportType*>(&report_type))
    {
        return TR::ReportTypeInfo::Syntax::REGULAR_XML;
    }
    else if(dynamic_cast<const XmlReportType*>(&report_type))
    {
        return TR::ReportTypeInfo::Syntax::PLAIN_XML;
    }
    else if(dynamic_cast<const TextReportType*>(&report_type))
    {
        return TR::ReportTypeInfo::Syntax::PLAIN_TEXT;
    }
    throw std::logic_error("Invalid report type");
}

NotificationPolicy get_notification_policy(NotificationSeverity notification_severity)
{
    switch (notification_severity)
    {
    case NotificationSeverity::DEFAULT:
        return NotificationPolicy::DEFAULT_POLICY;

    case NotificationSeverity::NONE:
        return NotificationPolicy::DISABLE_NOTIFICATION;

    case NotificationSeverity::INFO:
        return NotificationPolicy::ENABLE_NOTIFICATION;

    default:
        _ASSERT(false);
    }
    return NotificationPolicy::DEFAULT_POLICY; // to supress warning
}

NotificationSeverity get_notification_severity(NotificationPolicy notification_policy)
{
    switch (notification_policy)
    {
    case NotificationPolicy::DEFAULT_POLICY:
        return NotificationSeverity::DEFAULT;

    case NotificationPolicy::ENABLE_NOTIFICATION:
        return NotificationSeverity::INFO;

    case NotificationPolicy::DISABLE_NOTIFICATION:
        return NotificationSeverity::NONE;

    default:
        _ASSERT(false);
    }

    return NotificationSeverity::DEFAULT; // to supress warning
}

} //namespace {

PlainFacade::PlainFacade(CoreImpl* core_impl):
    m_core_impl(core_impl),
    m_db(*core_impl->m_db),
    m_basis(core_impl->m_core_domain.m_basis),
    m_registry(core_impl->m_core_domain.m_registry),
    m_services(core_impl->m_core_domain.m_services),
    m_source_types(m_basis.m_source_types),
    m_report_types(m_basis.m_report_types),
    m_stream_types(m_basis.m_stream_types),
    m_transformations(m_basis.m_transformations),
    m_actions(m_basis.m_actions),
    m_action_shortcuts(m_basis.m_action_shortcuts),
    m_source_type_activities(m_basis.m_source_type_activities), 
    m_roles(m_registry.m_roles),
    m_sources(m_registry.m_sources),
    m_links(m_registry.m_links),
    m_collector(m_registry.m_collector),    
    m_reservoir(m_registry.m_reservoir),
    m_resource_policies(m_registry.m_resource_policies),
    m_source_resources(m_registry.m_source_resources),
    m_custom_queries(m_services.m_custom_queries),
    m_custom_filters(m_services.m_custom_filters),
    m_tracking_streams(m_services.m_tracking_streams),
    m_stream_filters(m_services.m_stream_filters),
    m_validations(m_services.m_validations),
    m_compliances(m_services.m_compliances),
    m_grouping_reports(m_services.m_grouping_reports),
    m_integrity_checking(m_services.m_integrity_checking),
    m_archive(m_services.m_archive),
    m_custom_actions(m_services.m_custom_actions),
    m_security(core_impl->m_security),
    m_scheduler(core_impl->m_scheduler),
    m_notifier(core_impl->m_notifier),
    m_mailing(core_impl->m_mailing)
{   
}

static void increment_counter(SummaryInfo& summary_info, Severity severity)
{
    switch (severity)
    {
    case Severity::SEVERITY_FAIL:
        ++summary_info.m_failed_sources;
        break;

    case Severity::SEVERITY_WARNING:
        ++summary_info.m_warning_sources;
        break;

    case Severity::SEVERITY_INFO:
        break;

    default:
        _ASSERT(false);
    }
}

static Severity max_alert_severity(const SourceInfo& source_info)
{
    auto max_severity = Severity::SEVERITY_INFO;
    for (auto& alert : source_info.m_alerts)
    {
        auto severity = boost::polymorphic_get<Alert>(alert).m_severity;
        if (int(severity) > int(max_severity))
        {
            max_severity = severity;
        }
    }
    return max_severity;
}

SummaryInfo PlainFacade::get_summary_info() const
{
    auto sources_info = get_sources_info();
    SummaryInfo summary_info(sources_info.size(), 0, 0);
    for (auto& source_info : sources_info)
    {
        increment_counter(summary_info, max_alert_severity(source_info));       
    }
    return summary_info;
}

std::vector<UserInfo> PlainFacade::get_users() const
{
    std::vector<UserInfo> users_info;
    for (auto& user : m_security.get_users())
    {
        auto user_email = m_mailing.get_email(user.m_key);
        users_info.push_back({user.m_key, user.m_name, user_email? *user_email: L""});
    }
    return users_info;
}

Key PlainFacade::add_user(const UserSettings& user_settings)
{
    Transaction t(m_db);
    auto user_key = m_security.add_user(user_settings.m_name, t);
    m_mailing.set_email(user_key, boost::make_optional(!user_settings.m_email.empty(), user_settings.m_email), t);
    t.commit();
    return user_key;
}

void PlainFacade::update_user(Key user_key, const UserSettings& user_settings)
{
    Transaction t(m_db);
    m_security.update_user(user_key, user_settings.m_name, t);
    m_mailing.set_email(user_key, boost::make_optional(!user_settings.m_email.empty(), user_settings.m_email), t);
    t.commit();
}

void PlainFacade::remove_user(Key user_key)
{
    Transaction t(m_db);
    m_security.remove_user(user_key, t);
    t.commit();
}

SourceAccessInfo PlainFacade::get_access_info(SourceKey source_key) const
{
    auto source = m_sources.get_source(source_key);     
    std::vector<AccessEntry> access_entries;        
    for (auto& ace : m_security.get_acl(source))
    {
        access_entries.push_back({ace.m_user_key, source_key, TR::Access(ace.m_access_rights.get_value())});
    }

    std::vector<AccessEntry> default_access_entries;
    for (auto& user : m_security.get_users())
    {
        auto default_ace = m_security.get_default_ace(user.m_key, source);
        default_access_entries.push_back({user.m_key, source_key, TR::Access(default_ace.m_access_rights.get_value())});
    }
    return SourceAccessInfo{source_key, std::move(access_entries), std::move(default_access_entries)};
}

std::vector<AccessEntry> PlainFacade::get_access_entries(SourceKey source_key) const
{
    auto source = m_sources.get_source(source_key);
    std::vector<AccessEntry> access_entries;
    for (auto& user : m_security.get_users())
    {
        auto ace = m_security.get_ace(user.m_key, source);
        access_entries.push_back({user.m_key, source_key, TR::Access(ace.m_access_rights.get_value())});
    }
    return access_entries;
}

void PlainFacade::set_access_list(const SourceAccessList& source_access_list) const
{
    auto source = m_sources.get_source(source_access_list.m_source_key);
    ACL acl;
    for (auto& access_entry : source_access_list.m_access_entries)
    {
        acl.push_back({source, access_entry.m_user_key, Access(access_entry.m_access_rights.get_value())});
    }       
    m_security.set_acl(source, acl);
}

std::vector<ReportTypeInfo> PlainFacade::get_report_types_info() const
{
    std::vector<ReportTypeInfo> report_types_info;
    for (auto& report_type : m_report_types.get_report_types())
    {
        report_types_info.push_back(ReportTypeInfo(
            report_type.get_uuid(),
            report_type.get_name(),
            get_report_type_syntax(report_type),
            report_type.get_version(),
            ReportTypeInfo::Trait(report_type.get_traits().get_value()),
            get_compliance_key(report_type),
            get_validation_key(report_type),
            get_custom_query_key(report_type),
            get_custom_filter_key(report_type),
            get_grouping_key(report_type)
        ));
    }
    return report_types_info;
}

ReportTypeInfo PlainFacade::get_report_type_info(ReportTypeUUID report_type_uuid) const
{
    auto& report_type = m_report_types.get_report_type(report_type_uuid);
    return ReportTypeInfo(
        report_type.get_uuid(),
        report_type.get_name(),
        get_report_type_syntax(report_type),
        report_type.get_version(),
        ReportTypeInfo::Trait(report_type.get_traits().get_value()),
        get_compliance_key(report_type),
        get_validation_key(report_type),
        get_custom_query_key(report_type),
        get_custom_filter_key(report_type),
        get_grouping_key(report_type)
    );  
}

std::shared_ptr<const XmlDoc> PlainFacade::get_content_def(ReportTypeUUID report_type_uuid) const
{
    auto& report_type = m_report_types.get_report_type(report_type_uuid);
    if (auto regular_report_type = dynamic_cast<const RegularReportType*>(&report_type))
    {
        return regular_report_type->get_def_doc();
    }   
    return nullptr;
}

std::vector<StreamTypeInfo> PlainFacade::get_stream_types_info() const
{
    std::vector<StreamTypeInfo> stream_types_info;
    for (auto& stream_type : m_stream_types.get_stream_types())
    {
        stream_types_info.push_back(get_stream_type_info(stream_type));
    }
    return stream_types_info;
}

StreamTypeInfo PlainFacade::get_stream_type_info(StreamTypeUUID stream_type_uuid) const
{
    auto& stream_type = m_stream_types.get_stream_type(stream_type_uuid);
    return get_stream_type_info(stream_type);
}

StreamTypeInfo PlainFacade::get_stream_type_info(const StreamType& stream_type) const
{
    auto tracking_stream = m_tracking_streams.find_by_output(stream_type);

    return StreamTypeInfo(
        stream_type.get_uuid(),
        stream_type.get_name(),
        stream_type.get_version(),
        tracking_stream ? tracking_stream->get_key() : 0
    );
}

std::shared_ptr<const XmlDefDoc> PlainFacade::get_message_def(StreamTypeUUID stream_type_uuid) const
{
    auto& stream_type = m_stream_types.get_stream_type(stream_type_uuid);
    return stream_type.get_message_def();
}

std::vector<ActionInfo> PlainFacade::get_actions_info() const
{
    std::vector<ActionInfo> actions_info;
    for (auto& action : m_actions.get_actions())
    {
        actions_info.push_back(get_action_info(action.get_uuid()));     
    }
    return actions_info;
}

ActionInfo PlainFacade::get_action_info(ActionUUID action_uuid) const
{
    auto& action = m_actions.get_action(action_uuid);
    auto action_info = ActionInfo(action.get_uuid(), action.get_name());
    for (auto& action_shortcut : m_action_shortcuts.find_shortcuts(action))
    {       
        auto output_ref = action_shortcut.get_output_ref();
        action_info.m_shortcuts.push_back({
            action_shortcut.get_uuid(),
            action_shortcut.get_name(),
            output_ref ? boost::make_optional(output_ref->get_uuid()): boost::none});
    }   
    return action_info;
}

Params PlainFacade::get_default_params(ActionUUID action_uuid) const
{
    auto& action = m_actions.get_action(action_uuid);
    return {action.get_default_params(), action.get_params_def()};
}

Params PlainFacade::create_params(ActionUUID action_uuid, const std::string& params_xml) const
{
    auto& action = m_actions.get_action(action_uuid);
    return {XML::parse_properties_doc(params_xml, *action.get_params_def()), action.get_params_def()};
}

std::vector<SourceTypeInfo> PlainFacade::get_source_types_info() const
{
    std::vector<SourceTypeInfo> source_types_info;
    for (auto& source_type : m_source_types.get_source_types())
    {
        source_types_info.push_back(get_source_type_info(source_type.get_uuid()));
    }
    return source_types_info;
}

SourceTypeInfo PlainFacade::get_source_type_info(UUID source_type_uuid) const
{
    auto& source_type = m_source_types.get_source_type(source_type_uuid);
    auto source_type_info = SourceTypeInfo(source_type.get_uuid(), source_type.get_name(), source_type.get_family());
    
    for (auto activity : m_source_type_activities.get_activities(source_type))
    {
        if (auto loading_activity = boost::get<LoadingActivity>(&activity))
        {
            source_type_info.m_download_uuids.push_back(loading_activity->m_output_ref.get_uuid());
        }

        if (auto action_activity = boost::get<ActionActivity>(&activity))
        {
            source_type_info.m_action_uuids.push_back(action_activity->m_action_ref.get_uuid());
        }

        if (auto action_delegate_activity = boost::get<ActionDelegateActivity>(&activity))
        {
            source_type_info.m_action_uuids.push_back(action_delegate_activity->m_guest_action_ref.get_uuid());
        }

        if (auto shortcut_activity = boost::get<ShortcutActivity>(&activity))
        {
            source_type_info.m_action_shortcut_uuids.push_back(shortcut_activity->m_shortcut_ref.get_uuid());
        }       
    }

    return source_type_info;
}

std::shared_ptr<const XmlDefDoc> PlainFacade::get_config_def(UUID source_type_uuid) const
{
    auto& source_type = m_source_types.get_source_type(source_type_uuid);
    return source_type.get_config_def();
}

SourceConfig PlainFacade::get_default_config(UUID source_type_uuid) const
{
    auto& source_type = m_source_types.get_source_type(source_type_uuid);
    return SourceConfig(source_type.get_default_config(), source_type.get_config_def());
}

std::vector<RoleInfo> PlainFacade::get_roles_info() const
{
    std::vector<RoleInfo> roles_info;
    for (auto& role : m_roles.get_roles())
    {
        roles_info.push_back({
            role.get_ref().get_key(),
            role.get_name(), 
            role.has_parent() ? role.get_parent().get_key() : RoleKey(0)
        });
    }
    return roles_info;
}

RoleInfo PlainFacade::get_role_info(RoleKey role_key) const
{
    auto role = m_roles.get_role(role_key);
    return RoleInfo(
        role.get_ref().get_key(),
        role.get_name(),
        role.has_parent() ? role.get_parent().get_key() : RoleKey(0)
    );
}

RoleKey PlainFacade::add_role(const RoleSettings& role_settings) const
{
    auto parentRole = m_roles.get_role(role_settings.m_parent_key);
    auto role = m_roles.add_role({role_settings.m_name, parentRole});
    return role.get_key();
}

void PlainFacade::update_role(RoleKey role_key, const RoleSettings& role_settings) const
{
    auto role = m_roles.get_role(role_key);
    role.set_name(role_settings.m_name);
    role.set_parent(m_roles.get_role(role_settings.m_parent_key));
    m_roles.update_role(role);
}

void PlainFacade::remove_role(RoleKey role_key)
{
    m_roles.remove_role(role_key);
}

std::vector<SourceInfo> PlainFacade::get_sources_info() const
{
    ReadOnlyTransaction t(m_db);
    std::vector<SourceInfo> sources_info;
    for (auto& source : m_sources.get_sources())
    {
        auto source_key = source.get_ref().get_key();
        sources_info.emplace_back(get_source_info(source_key));
    }
    return sources_info;
}

SourceInfo PlainFacade::get_source_info(SourceKey source_key) const
{
    ReadOnlyTransaction t(m_db);
    auto& source = m_sources.get_source(source_key);    
    SourceInfo source_info(
        source.get_ref().get_key(),
        source.get_name(),
        source.get_role().get_key(),
        source.has_parent() ? source.get_parent().get_key() : SourceKey(0),
        source.get_source_type().get_uuid()
    );

    for (auto& source_resource : m_source_resources.get_source_resources(source))
    {
        if (!source_resource.m_visible)
        {
            continue;
        }

        auto report_type_ref = boost::get<ReportTypeRef>(&source_resource.m_resource_ref);
        if (!report_type_ref)
        {
            continue;
        }

        if (source_resource.m_deferred)
        {
            source_info.m_current_reports.push_back({source_key, report_type_ref->get_uuid(), 0, 0, true, true});
            continue;
        }

        if (auto collected_report = m_collector.find_current_report(source, *report_type_ref))
        {
            source_info.m_current_reports.push_back({
                source_key,
                collected_report->get_report_type_ref().get_uuid(),
                collected_report->get_time(),
                collected_report->get_check_time(),
                false,
                false
            });
        }
        else
        {
            source_info.m_current_reports.push_back({source_key, report_type_ref->get_uuid(), 0, 0, true, false});
        }

        if (m_integrity_checking.is_checking_on(source, *report_type_ref) &&
            m_integrity_checking.get_checking_result(source, *report_type_ref) == IntegrityCheckingResult::MISMATCHED)
        {
            source_info.m_alerts.push_back(CheckingAlert{
                CheckingAlert::INTEGRITY_CHECKING,
                source_key,
                report_type_ref->get_uuid(),
                Severity::SEVERITY_FAIL,
                L""
            });
        }

        if (auto validation = m_validations.find_by_output(*report_type_ref))
        {
            if (m_validations.get_validation_result(*validation, source) == ValidationResult::VALIDATION_FAILED)
            {
                source_info.m_alerts.push_back(CheckingAlert{
                    CheckingAlert::VALIDATION,
                    source_key,
                    report_type_ref->get_uuid(),
                    Severity::SEVERITY_FAIL,
                    L""
                });
            }
        }

        if (auto compliance = m_compliances.find_by_output(*report_type_ref))
        {
            if (m_compliances.get_checking_result(*compliance, source) == ComplianceCheckingResult::CHECKING_FAILED)
            {
                source_info.m_alerts.push_back(CheckingAlert{
                    CheckingAlert::COMPLIANCE,
                    source_key,
                    report_type_ref->get_uuid(),
                    Severity::SEVERITY_FAIL,
                    L""
                });
            }
        }

        if (m_scheduler.get_effective_task_schedule(source, PeriodicReload(*report_type_ref)).get_key() != NEVER_SCHEDULE_KEY)
        {
            auto scheduler_reload_result = m_scheduler.get_task_result(source, PeriodicReload(*report_type_ref));
            if (!scheduler_reload_result.m_result)
            {
                source_info.m_alerts.push_back(ScheduledReloadAlert{
                    source_key,
                    report_type_ref->get_uuid(),
                    Severity::SEVERITY_FAIL,
                    scheduler_reload_result.m_message
                });
            }
        }        
    }

    for (auto& source_resource : m_source_resources.get_source_resources(source))
    {
        if (!source_resource.m_visible)
        {
            continue;
        }

        auto stream_type_ref = boost::get<StreamTypeRef>(&source_resource.m_resource_ref);
        if (!stream_type_ref)
        {
            continue;
        }

        source_info.m_streams.push_back({
            source_key,
            stream_type_ref->get_uuid(),
            m_reservoir.get_stream_time(source, *stream_type_ref)
        });
    }

    for (auto& shortcut : m_action_shortcuts.get_shortcuts())
    {
        auto scheduler_launch_result = m_scheduler.get_task_result(source, PeriodicLaunch(shortcut));
        if (!scheduler_launch_result.m_result)
        {
            source_info.m_alerts.push_back(ScheduledLaunchAlert{
                SourceKey(source_key),
                shortcut.get_action_ref().get_uuid(),
                shortcut.get_ref().get_uuid(),
                Severity::SEVERITY_FAIL,
                scheduler_launch_result.m_message
            });
        }
    }

    return source_info; 
}

SourceKey PlainFacade::add_source(const SourceSettings& source_settings)
{
    auto& source_type = m_source_types.get_source_type(source_settings.m_source_type_uuid);
    Source source(
        std::move(source_settings.m_name),
        source_settings.m_parent_key ? boost::make_optional(m_sources.get_source(source_settings.m_parent_key).get_ref()) : boost::none,
        m_roles.get_role(source_settings.m_role_key),
        source_type,
        source_type.get_default_config()
    );
    return m_sources.add_source(std::move(source)).get_key();
}

void PlainFacade::update_source(SourceKey source_key, const SourceSettings& source_settings)
{
    auto source = m_sources.get_source(source_key);
    source.set_name(source_settings.m_name);
    
    if (source.get_source_type().get_uuid() != source_settings.m_source_type_uuid)
    {
        auto& source_type = m_source_types.get_source_type(source_settings.m_source_type_uuid);
        source.set_source_type(source_type);
        source.set_config(source_type.get_default_config());
    }

    if (source.get_role().get_key() != source_settings.m_role_key)
    {
        auto role = m_roles.get_role(source_settings.m_role_key);
        source.set_role(role);
    }   

    if (source.has_parent() && source.get_parent().get_key() != source_settings.m_parent_key)
    {
        _ASSERT(source_settings.m_parent_key != 0);
        auto parent = m_sources.get_source(source_settings.m_parent_key);
        source.set_parent(parent);
    }

    m_sources.update_source(std::move(source));
}

void PlainFacade::remove_source(SourceKey source_key)
{
    Transaction t(m_db);
    auto source = m_sources.get_source(source_key);
    unlink_source(source, t);
    m_sources.remove_source(source_key, t);
    t.commit();
}

void PlainFacade::unlink_source(SourceRef source_ref, Transaction& t)
{
    for (auto& link : m_links.find_by_child(source_ref))
    {
        m_links.remove_link(link, t);
    }

    for (auto& child : m_sources.get_sources(source_ref))
    {
        unlink_source(child, t);
    }
}

SourceConfig PlainFacade::get_source_config(SourceKey source_key) const
{
    auto source = m_sources.get_source(source_key);
    auto& source_type = *source.get_source_type();
    return SourceConfig(clone_doc(*source.get_config()), source_type.get_config_def());
}

void PlainFacade::set_source_config(SourceKey source_key, SourceConfig source_config)
{
    auto source = m_sources.get_source(source_key);
    auto& source_type = *source.get_source_type();
    if (source_config.m_config_def.get() != source_type.get_config_def().get())
    {
        throw Exception(L"Invalid source config");
    }
    source.set_config(std::move(source_config.m_config));
    m_sources.update_source(std::move(source));
}

std::vector<TR::Link> PlainFacade::get_links() const
{
    std::vector<TR::Link> links;
    for (auto& link : m_links.get_links())
    {
        links.push_back({link.m_parent_ref.get_key(), link.m_child_ref.get_key()});
    }
    return links;
}

void PlainFacade::add_link(TR::Link link)
{
    Transaction t(m_db);
    auto parent = m_sources.get_source(link.m_parent_key);
    auto child = m_sources.get_source(link.m_child_key);
    m_links.add_link({parent, child}, t);   
    t.commit();
}
void PlainFacade::remove_link(TR::Link link)
{
    Transaction t(m_db);
    auto parent = m_sources.get_source(link.m_parent_key);
    auto child = m_sources.get_source(link.m_child_key);
    m_links.remove_link({parent, child}, t);
    t.commit();
}

CurrentReportInfo PlainFacade::get_current_report_info(SourceKey source_key, ReportTypeUUID report_type_uuid) const
{
    auto current_report = m_collector.get_current_report(m_sources.get_source(source_key), m_report_types.get_report_type(report_type_uuid));
    return CurrentReportInfo(
        SourceKey(source_key),
        current_report.get_report_type_ref().get_uuid(),
        current_report.get_time(),
        current_report.get_check_time(),
        false,
        false
    );
}

std::shared_ptr<Content> PlainFacade::get_current_content(SourceKey source_key, ReportTypeUUID report_type_uuid) const
{
    auto current_report = m_collector.get_current_report(m_sources.get_source(source_key), m_report_types.get_report_type(report_type_uuid));
    return current_report.get_content();    
}

std::vector<ArchivedReportInfo> PlainFacade::get_archive(SourceKey source_key) const
{   
    auto& source = m_sources.get_source(source_key);
    std::vector<ArchivedReportInfo> archived_reports_info;
    for (auto& archived_report : m_archive.get_archived_reports(source))
    {
        if (!m_resource_policies.is_visible(source, archived_report.get_report_type_ref()))
        {
            continue;
        }

        archived_reports_info.push_back({
            archived_report.get_ref().get_key(),
            archived_report.get_source_ref().get_key(),
            archived_report.get_report_type_ref().get_uuid(),
            archived_report.get_diff(),
            archived_report.get_time()
        });     
    }

    return archived_reports_info;
}

ArchivedReportInfo PlainFacade::get_archived_report(ArchivedReportKey archived_report_key) const
{
    auto archived_report = m_archive.get_archived_report(archived_report_key);
    return ArchivedReportInfo(
        archived_report.get_ref().get_key(),
        archived_report.get_source_ref().get_key(),
        archived_report.get_report_type_ref().get_uuid(),
        archived_report.get_diff(),
        archived_report.get_time()
    );
}

std::shared_ptr<Content> PlainFacade::get_archived_content(ArchivedReportKey archived_report_key) const
{
    auto archived_report = m_archive.get_archived_report(archived_report_key);
    return archived_report.get_content();
}

StreamInfo PlainFacade::get_stream_info(SourceKey source_key, StreamTypeUUID stream_type_uuid) const
{
    auto source = m_sources.get_source(source_key);
    auto& stream_type = m_stream_types.get_stream_type(stream_type_uuid);

    return StreamInfo(
        source_key,
        stream_type_uuid,
        m_reservoir.get_stream_time(source, stream_type)
    );
}

SubjectReportGenerating PlainFacade::get_report_generating_options(SubjectRef subject_ref, ReportTypeRef report_type_ref) const
{
    auto base_ref = get_base_ref(subject_ref);
    auto policies = m_resource_policies.get_policies(subject_ref, report_type_ref);
    auto schedule_ref = m_scheduler.get_task_schedule(subject_ref, PeriodicReload(report_type_ref));
    auto default_schedule_ref = base_ref ? m_scheduler.get_effective_task_schedule(*base_ref, PeriodicReload(report_type_ref)) : m_scheduler.get_schedule(NEVER_SCHEDULE_KEY);
    
    return SubjectReportGenerating(
        get_subject_key(subject_ref),
        {
            report_type_ref.get_uuid(),
            TR::EnableReportPolicy(policies.m_enable_policy),
            TR::ReportVisibility(policies.m_visibility),
            TR::DeferReportPolicy(policies.m_defer_policy),
            schedule_ref? boost::make_optional(schedule_ref->get_key()): boost::none
        },
        base_ref ? m_resource_policies.is_enabled(*base_ref, report_type_ref) : true,
        base_ref ? m_resource_policies.is_visible(*base_ref, report_type_ref) : true,
        base_ref ? m_resource_policies.is_deferred(*base_ref, report_type_ref) : false,
        default_schedule_ref.get_key()
    );
}

SubjectIntegrityChecking PlainFacade::get_integrity_checking_options(Key user_key, SubjectRef subject_ref, ReportTypeRef report_type_ref) const
{
    auto base_ref = get_base_ref(subject_ref);
    auto integrity_checking_policy = m_integrity_checking.get_checking_policy(subject_ref, report_type_ref);
    auto pattern_choise = m_integrity_checking.get_pattern_choise(subject_ref, report_type_ref);
    auto notification_severity = m_notifier.get_severity(user_key, NotificationCause::INTEGRITY_CHECKING_FAILED, subject_ref, report_type_ref);
    auto base_notification_severity = base_ref? m_notifier.get_actual_severity(user_key, NotificationCause::INTEGRITY_CHECKING_FAILED, *base_ref, report_type_ref): NotificationSeverity::NONE;
        
    return SubjectIntegrityChecking(
        get_subject_key(subject_ref),
        {
            report_type_ref.get_uuid(),
            TR::IntegrityCheckingPolicy(integrity_checking_policy),
            TR::IntegrityCheckingPatternChoise(pattern_choise),
            get_notification_policy(notification_severity)
        },
        m_integrity_checking.has_pattern(subject_ref, report_type_ref),
        base_ref ? m_integrity_checking.is_checking_on(*base_ref, report_type_ref) : false,
        get_notification_policy(base_notification_severity) == NotificationPolicy::ENABLE_NOTIFICATION
    );
}

SubjectComplianceChecking PlainFacade::get_compliance_options(Key user_key, SubjectRef subject_ref, ComplianceRef compliance_ref) const
{
    auto base_ref = get_base_ref(subject_ref);
    auto checking_policy = m_compliances.get_checking_policy(compliance_ref, subject_ref);
    auto output_ref = (*compliance_ref).get_output_ref();
    auto notification_severity = m_notifier.get_severity(user_key, NotificationCause::COMPLIANCE_CHECKING_FAILED, subject_ref, output_ref);
    auto base_notification_severity = base_ref? m_notifier.get_actual_severity(user_key, NotificationCause::COMPLIANCE_CHECKING_FAILED, *base_ref, output_ref): NotificationSeverity::NONE;

    return SubjectComplianceChecking{
        get_subject_key(subject_ref),
        {
            output_ref.get_uuid(),
            TR::ComplianceCheckingPolicy(checking_policy),
            get_notification_policy(notification_severity)
        },
        m_compliances.has_compliance_setup(compliance_ref, subject_ref),
        base_ref ? m_compliances.is_checking_on(compliance_ref, *base_ref) : true,
        get_notification_policy(base_notification_severity) == NotificationPolicy::ENABLE_NOTIFICATION
    };  
}

SubjectValidating PlainFacade::get_validation_options(Key user_key, SubjectRef subject_ref, ValidationRef validation_ref) const
{
    auto base_ref = get_base_ref(subject_ref);
    auto validation_policy = m_validations.get_validation_policy(validation_ref, subject_ref);
    auto output_ref = (*validation_ref).get_output_ref();
    auto notification_severity = m_notifier.get_severity(user_key, NotificationCause::VALIDATION_FAILED, subject_ref, output_ref);
    auto base_notification_severity = base_ref? m_notifier.get_actual_severity(user_key, NotificationCause::VALIDATION_FAILED, *base_ref, output_ref): NotificationSeverity::NONE;
    
    return SubjectValidating{
        get_subject_key(subject_ref),
        {
            output_ref.get_uuid(),
            TR::ValidationPolicy(validation_policy),
            get_notification_policy(notification_severity)
        },
        m_validations.has_validator(validation_ref, subject_ref),
        base_ref ? m_validations.is_validation_on(validation_ref, *base_ref) : true,
        get_notification_policy(base_notification_severity) == NotificationPolicy::ENABLE_NOTIFICATION
    };
}

SubjectArchiving PlainFacade::get_archiving_options(SubjectRef subject_ref, ReportTypeRef report_type_ref) const
{
    auto base_ref = get_base_ref(subject_ref);
    auto archiving_policy = TR::ArchivingPolicy(m_archive.get_archiving_policy(subject_ref, report_type_ref));
    auto default_archiving_policy = TR::ArchivingPolicy((base_ref && m_archive.is_archiving_enabled(*base_ref, report_type_ref)) ? ArchivingPolicy::ENABLE_ARCHIVING : ArchivingPolicy::DISABLE_ARCHIVING);
    auto archived_expiration = TR::ArchivedExpiration(m_archive.get_archived_expiration(subject_ref, report_type_ref));
    auto default_archived_expiration = TR::ArchivedExpiration(base_ref ? m_archive.get_effective_archived_expiration(*base_ref, report_type_ref) : ArchivedExpiration::EXPIRED_NEVER);

    return SubjectArchiving{
        get_subject_key(subject_ref),
        {
            report_type_ref.get_uuid(),
            archiving_policy,
            archived_expiration
        },
        default_archiving_policy,
        default_archived_expiration
    };
}

SubjectStreamGenerating PlainFacade::get_stream_generating_options(Key user_key, SubjectRef subject_ref, StreamTypeRef stream_type_ref) const
{
    auto base_ref = get_base_ref(subject_ref);
    auto policies = m_resource_policies.get_policies(subject_ref, stream_type_ref);
    auto stream_length = m_reservoir.get_stream_length(subject_ref, stream_type_ref);
    auto notification_severity = m_notifier.get_severity(user_key, NotificationCause::STREAM_MESSAGE_RECIEVED, subject_ref, stream_type_ref);
    auto base_stream_length = base_ref ? m_reservoir.get_effective_stream_length(*base_ref, stream_type_ref) : StreamLength::FOREVER;
    auto base_notification_severity = base_ref? m_notifier.get_actual_severity(user_key, NotificationCause::STREAM_MESSAGE_RECIEVED, *base_ref, stream_type_ref): NotificationSeverity::NONE;
    
    return SubjectStreamGenerating(
        get_subject_key(subject_ref),
        {
            stream_type_ref.get_uuid(),
            TR::ReportVisibility(policies.m_visibility),
            TR::StreamLength(stream_length),
            TR::NotificationSeverity(notification_severity)
        },
        base_ref ? m_resource_policies.is_visible(*base_ref, stream_type_ref) : true,
        TR::StreamLength(base_stream_length),
        TR::NotificationSeverity(base_notification_severity)
    );
}

SubjectReportOptions PlainFacade::get_options(Key user_key, SubjectRef subject_ref, ReportTypeRef report_type_ref) const
{
    SubjectReportOptions subject_report_options(get_subject_key(subject_ref), report_type_ref.get_uuid());
    subject_report_options.m_report_generating = get_report_generating_options(subject_ref, report_type_ref);
    
    if (auto compliance = m_compliances.find_by_output(report_type_ref))
    {
        subject_report_options.m_compliance_checking = get_compliance_options(user_key, subject_ref, *compliance);      
    }
    else if (auto validation = m_validations.find_by_output(report_type_ref))
    {
        subject_report_options.m_validating = get_validation_options(user_key, subject_ref, *validation);       
    }
    else
    {
        subject_report_options.m_integrity_checking = get_integrity_checking_options(user_key, subject_ref, report_type_ref);
        subject_report_options.m_archiving = get_archiving_options(subject_ref, report_type_ref);   
    }

    return subject_report_options;
}

SubjectStreamOptions PlainFacade::get_options(Key user_key, SubjectRef subject_ref, StreamTypeRef stream_type_ref) const
{
    SubjectStreamOptions subject_stream_options(get_subject_key(subject_ref), stream_type_ref.get_uuid());
    subject_stream_options.m_stream_generating = get_stream_generating_options(user_key, subject_ref, stream_type_ref);
    return subject_stream_options;
}

SubjectActionOptions PlainFacade::get_options(SubjectRef subject_ref, ActionShortcutRef shortcut_ref) const
{   
    SubjectActionOptions subject_action_options(get_subject_key(subject_ref), {(*shortcut_ref).get_action_ref().get_uuid(), shortcut_ref.get_uuid(), boost::none});
    auto schedule_ref = m_scheduler.get_task_schedule(subject_ref, PeriodicLaunch(shortcut_ref));
    
    auto base_ref = get_base_ref(subject_ref);
    auto default_schedule_ref = base_ref ? m_scheduler.get_effective_task_schedule(*base_ref, PeriodicLaunch(shortcut_ref)) : m_scheduler.get_schedule(NEVER_SCHEDULE_KEY);

    subject_action_options.m_schedule_key = schedule_ref ? boost::make_optional(schedule_ref->get_key()) : boost::none;
    subject_action_options.m_default_schedule_key = default_schedule_ref.get_key();
    return subject_action_options;
}

SubjectActionOptions PlainFacade::get_options(SubjectRef subject_ref, ActionRef action_ref) const
{
    SubjectActionOptions subject_action_options(get_subject_key(subject_ref), {action_ref.get_uuid(), boost::none, boost::none});
    return subject_action_options;
}

SubjectReportOptions PlainFacade::get_options(Key user_key, SubjectKey subject_key, ReportTypeUUID report_type_uuid) const
{
    auto& report_type = m_report_types.get_report_type(report_type_uuid);
    auto subject_ref = get_subject_ref(m_registry, subject_key);
    return get_options(user_key, subject_ref, report_type);
}

SubjectStreamOptions PlainFacade::get_options(Key user_key, SubjectKey subject_key, StreamTypeUUID stream_type_uuid) const
{
    auto& stream_type = m_stream_types.get_stream_type(stream_type_uuid);
    auto subject_ref = get_subject_ref(m_registry, subject_key);
    return get_options(user_key, subject_ref, stream_type);
}

SubjectActionOptions PlainFacade::get_options(SubjectKey subject_key, ActionUUID action_uuid, boost::optional<UUID> shortcut_uuid) const
{
    auto& action = m_actions.get_action(action_uuid);
    auto subject_ref = get_subject_ref(m_registry, subject_key);
    if (shortcut_uuid)
    {
        auto& shortcut= m_action_shortcuts.get_shortcut(*shortcut_uuid);
        return get_options(subject_ref, shortcut);
    }

    return get_options(subject_ref, action);
}

void PlainFacade::set_options(Key user_key, SubjectKey subject_key, const ReportOptions& report_options)
{
    Transaction t(m_db);
    auto subject_ref = get_subject_ref(m_registry, subject_key);
    auto& report_type = m_report_types.get_report_type(report_options.m_report_type_uuid);

    auto& report_generating_options = report_options.m_report_generating;
    m_resource_policies.set_enable_policy(subject_ref, report_type, EnablePolicy(report_generating_options.m_enable_report_policy), t);
    m_resource_policies.set_visibility(subject_ref, report_type, Visibility(report_generating_options.m_report_visibility), t);
    m_resource_policies.set_defer_policy(subject_ref, report_type, DeferPolicy(report_generating_options.m_defer_report_policy), t);

    boost::optional<ScheduleRef> schedule_ref;
    if (report_generating_options.m_schedule_key)
    {
        schedule_ref = m_scheduler.get_schedule(*report_generating_options.m_schedule_key);
    }
    m_scheduler.set_task_schedule(subject_ref, PeriodicReload(report_type), schedule_ref, t);

    if (report_options.m_compliance_checking)
    {
        auto compliance = m_compliances.find_by_output(report_type);
        m_compliances.set_checking_policy(*compliance, subject_ref, ComplianceCheckingPolicy(report_options.m_compliance_checking->m_compliance_checking_policy), t);
        m_notifier.set_severity(user_key, NotificationCause::COMPLIANCE_CHECKING_FAILED, subject_ref, report_type, get_notification_severity(report_options.m_compliance_checking->m_notification_policy), t);
    }

    if (report_options.m_validating)
    {
        auto validation = m_validations.find_by_output(report_type);
        m_validations.set_validation_policy(*validation, subject_ref, ValidationPolicy(report_options.m_validating->m_validation_policy), t);
        m_notifier.set_severity(user_key, NotificationCause::VALIDATION_FAILED, subject_ref, report_type, get_notification_severity(report_options.m_validating->m_notification_policy), t);
    }

    if (report_options.m_integrity_checking)
    {
        m_integrity_checking.set_checking_policy(subject_ref, report_type, IntegrityCheckingPolicy(report_options.m_integrity_checking->m_integrity_checking_policy), t);
        m_integrity_checking.set_pattern_choise(subject_ref, report_type, IntegrityCheckingPatternChoise(report_options.m_integrity_checking->m_pattern_choise), t);
        m_notifier.set_severity(user_key, NotificationCause::INTEGRITY_CHECKING_FAILED, subject_ref, report_type, get_notification_severity(report_options.m_integrity_checking->m_notification_policy), t);
    }

    if (report_options.m_archiving)
    {
        m_archive.set_archiving_policy(subject_ref, report_type, ArchivingPolicy(report_options.m_archiving->m_archiving_policy), t);
        m_archive.set_archived_expiration(subject_ref, report_type, ArchivedExpiration(report_options.m_archiving->m_archiving_expiration), t);
    }
    t.commit();
}

void PlainFacade::set_options(Key user_key, SubjectKey subject_key, const StreamOptions& stream_options)
{
    Transaction t(m_db);
    auto subject_ref = get_subject_ref(m_registry, subject_key);
    auto& stream_type = m_stream_types.get_stream_type(stream_options.m_stream_type_uuid);

    auto& stream_generating_options = stream_options.m_stream_generating;
    m_resource_policies.set_visibility(subject_ref, stream_type, Visibility(stream_generating_options.m_stream_visibility), t);
    m_reservoir.set_stream_length(subject_ref, stream_type, StreamLength(stream_generating_options.m_stream_length), t);
    m_notifier.set_severity(user_key, NotificationCause::STREAM_MESSAGE_RECIEVED, subject_ref, stream_type, NotificationSeverity(stream_generating_options.m_notification_severity), t);
    t.commit();
}

void PlainFacade::set_options(SubjectKey subject_key, const ActionOptions& action_options)
{
    Transaction t(m_db);
    auto subject_ref = get_subject_ref(m_registry, subject_key);

    if (action_options.m_shortcut_uuid)
    {
        auto& shortcut = m_action_shortcuts.get_shortcut(*action_options.m_shortcut_uuid);
        boost::optional<ScheduleRef> schedule_ref;
        if (action_options.m_schedule_key)
        {
            schedule_ref = m_scheduler.get_schedule(*action_options.m_schedule_key);
        }
        m_scheduler.set_task_schedule(subject_ref, PeriodicLaunch(shortcut), schedule_ref, t);
    }
    t.commit();
}

SubjectOptions PlainFacade::get_role_options(Key user_key, RoleKey role_key) const
{
    auto& role = m_roles.get_role(role_key);
    SubjectOptions role_options(role.get_ref().get_key());
    for (auto& report_type : m_report_types.get_report_types())
    {
        if (!m_resource_policies.is_enabled(role, report_type))
        {
            continue;
        }

        role_options.m_report_options.push_back(get_options(user_key, role, report_type));
    }

    for (auto& stream_type : m_stream_types.get_stream_types())
    {
        if (!m_resource_policies.is_enabled(role, stream_type))
        {
            continue;
        }
        role_options.m_stream_options.push_back(get_options(user_key, role, stream_type));
    }

    for (auto& action : m_actions.get_actions())
    {
        role_options.m_action_options.push_back(get_options(role, action));
        for (auto& shortcut : m_action_shortcuts.find_shortcuts(action))
        {
            role_options.m_action_options.push_back(get_options(role, shortcut));
        }
    }
    return role_options;
}

SubjectOptions PlainFacade::get_source_options(Key user_key, SourceKey source_key) const
{
    ReadOnlyTransaction t(m_db); //database query optimization 

    auto& source = m_sources.get_source(source_key);
    SubjectOptions source_options(source_key);
    for (auto& source_resource : m_source_resources.get_source_resources(source))
    {
        if (!source_resource.m_available)
        {
            continue;
        }

        if (auto report_type_ref = boost::get<ReportTypeRef>(&source_resource.m_resource_ref))
        {
            source_options.m_report_options.push_back(get_options(user_key, source, *report_type_ref));
        }
        else if (auto stream_type_ref = boost::get<StreamTypeRef>(&source_resource.m_resource_ref))
        {
            source_options.m_stream_options.push_back(get_options(user_key, source, *stream_type_ref));
        }
    }

    for (auto& activity : m_source_type_activities.get_activities(source.get_source_type()))
    {
        if (auto action_ref = get_action_ref(activity))
        {
            source_options.m_action_options.push_back(get_options(source, *action_ref));
            for (auto& shortcut : m_action_shortcuts.find_shortcuts(*action_ref))
            {
                source_options.m_action_options.push_back(get_options(source, shortcut));
            }
        }
    }
    return source_options;
}

ReportTypeOptions PlainFacade::get_report_type_options(Key user_key, ReportTypeUUID report_type_uuid) const
{
    auto& report_type = m_report_types.get_report_type(report_type_uuid);
    ReportTypeOptions report_type_options(report_type.get_uuid());
    for (auto subject_ref : get_subject_refs(m_registry))
    {
        if (auto source_ref = boost::get<SourceRef>(&subject_ref))
        {
            auto source_resouce = m_source_resources.find_source_resource(*source_ref, report_type);
            if (source_resouce == boost::none || source_resouce->m_available == false)
            {
                continue;
            }
        }
        report_type_options.m_subject_options.push_back(get_options(user_key, subject_ref, report_type));
    }

    return report_type_options;
}

StreamTypeOptions PlainFacade::get_stream_type_options(Key user_key, StreamTypeUUID stream_type_uuid) const
{
    auto& stream_type = m_stream_types.get_stream_type(stream_type_uuid);
    StreamTypeOptions stream_type_options(stream_type.get_uuid());
    for (auto subject_ref : get_subject_refs(m_registry))
    {
        if (auto source_ref = boost::get<SourceRef>(&subject_ref))
        {
            auto source_resouce = m_source_resources.find_source_resource(*source_ref, stream_type);
            if (source_resouce == boost::none || source_resouce->m_available == false)
            {
                continue;
            }
        }
        stream_type_options.m_subject_options.push_back(get_options(user_key, subject_ref, stream_type));
    }

    return stream_type_options;
}

ActionTypeOptions PlainFacade::get_action_options(ActionUUID action_uuid, boost::optional<UUID> shortcut_uuid) const
{
    auto& action = m_actions.get_action(action_uuid);
    const ActionShortcut* action_shortcut = nullptr;
    if (shortcut_uuid)
    {
        action_shortcut = &m_action_shortcuts.get_shortcut(*shortcut_uuid);
    }
    
    ActionTypeOptions action_type_options(action_uuid, shortcut_uuid);
    for (auto subject_ref : get_subject_refs(m_registry))
    {
        if (auto source_ref = boost::get<SourceRef>(&subject_ref))
        {
            auto& source_type = (**source_ref).get_source_type();
            if (!m_source_type_activities.find_by_action(source_type, action))
            {
                continue;
            }

            if (action_shortcut && !m_source_type_activities.find_by_shortcut(source_type, *action_shortcut))
            {
                continue;
            }           
        }

        if (action_shortcut)
        {
            action_type_options.m_subject_options.push_back(get_options(subject_ref, *action_shortcut));
        }
        else
        {
            action_type_options.m_subject_options.push_back(get_options(subject_ref, action));
        }       
    }

    return action_type_options;
}

Key PlainFacade::get_custom_query_key(ReportTypeRef output_ref) const
{
    if (auto custom_query = m_custom_queries.find_by_output(output_ref))
    {
        return custom_query->get_ref().get_key();
    }
    return Key();
}

CustomQueryInfo PlainFacade::get_custom_query_info(Key custom_query_key) const
{
    auto custom_query = m_custom_queries.get_custom_query(custom_query_key);
    return CustomQueryInfo(
        custom_query.get_ref().get_key(),
        custom_query.get_name(),
        stl_tools::copy_vector(custom_query.get_input_refs() | boost::adaptors::transformed(boost::bind(&ReportTypeRef::get_uuid, _1))),
        custom_query.get_output_ref().get_uuid()
    );
}

CustomQueryDefinition PlainFacade::get_custom_query_def(Key custom_query_key) const
{
    auto custom_query = m_custom_queries.get_custom_query(custom_query_key);
    std::vector<ReportTypeUUID> input_uuids;
    boost::transform(custom_query.get_input_refs(), std::back_inserter(input_uuids), [](ReportTypeRef input_ref){
        return input_ref.get_uuid();
    });

    return CustomQueryDefinition(
        std::move(input_uuids), 
        clone_doc(custom_query.get_query_doc()),
        custom_query.get_query_def()
    );
}

CustomQueryDefinition PlainFacade::create_custom_query(std::vector<ReportTypeUUID> input_uuids) const
{
    std::vector<ReportTypeRef> input_refs;
    for (auto uuid : input_uuids)
    {
        input_refs.push_back(m_report_types.get_report_type(uuid));
    }
    auto custom_query = m_custom_queries.create_custom_query(L"", std::move(input_refs));
    return CustomQueryDefinition(
        std::move(input_uuids), 
        clone_doc(custom_query.get_query_doc()),
        custom_query.get_query_def()
    );
}

Key PlainFacade::add_custom_query(std::wstring name, CustomQueryDefinition custom_query_def)
{
    std::vector<ReportTypeRef> input_refs;
    for (auto uuid : custom_query_def.m_input_uuids)
    {
        input_refs.push_back(m_report_types.get_report_type(uuid));
    }
    auto custom_query = m_custom_queries.create_custom_query(name, std::move(input_refs));
    auto query_doc = clone_doc(*custom_query_def.m_query_doc);
    XML::bind_query_definition(*query_doc, *custom_query.get_query_def());
    custom_query.set_query_doc(std::move(query_doc));

    Transaction t(m_db);
    m_custom_queries.add_custom_query(custom_query, t);
    m_resource_policies.set_enable_policy(m_roles.get_default_role(), custom_query.get_output_ref(), EnablePolicy::DISABLE, t);
    t.commit();

    return custom_query.get_ref().get_key();
}

void PlainFacade::remove_custom_query(Key custom_query_key)
{
    m_custom_queries.remove_custom_query(custom_query_key);
}

Key PlainFacade::get_custom_filter_key(ReportTypeRef output_ref) const
{
    if (auto custom_filter = m_custom_filters.find_by_output(output_ref))
    {
        return custom_filter->get_key();
    }
    return 0;
}

CustomFilterInfo PlainFacade::get_custom_filter_info(Key custom_filter_key) const
{
    auto custom_filter = m_custom_filters.get_custom_filter(custom_filter_key);
    return CustomFilterInfo(
        custom_filter.get_key(),
        custom_filter.get_name(),
        custom_filter.get_input_ref().get_uuid(),
        custom_filter.get_output_ref().get_uuid()
    );
}

Key PlainFacade::add_custom_filter(std::wstring name, ReportTypeUUID input_uuid, CustomFilterSetup custom_filter_setup)
{
    Transaction t(m_db);
    CustomFilter custom_filter(std::move(name), m_report_types.get_report_type(input_uuid));
    custom_filter.set_node_filters(custom_filter_setup.m_xml_node_filters);
    m_custom_filters.add_custom_filter(custom_filter, t);
    t.commit();

    return custom_filter.get_key();
}

CustomFilterSetup PlainFacade::create_filter_setup(ReportTypeUUID input_uuid) const
{
    auto& input = dynamic_cast<const RegularReportType&>(m_report_types.get_report_type(input_uuid));
    return CustomFilterSetup({}, input.get_def_doc());
}

CustomFilterSetup PlainFacade::get_filter_setup(Key custom_filter_key) const
{
    auto custom_filter = m_custom_filters.get_custom_filter(custom_filter_key);
    auto& input = dynamic_cast<const RegularReportType&>(*custom_filter.get_input_ref());
    return CustomFilterSetup(custom_filter.get_node_filters(), input.get_def_doc());
}

void PlainFacade::set_filter_setup(Key custom_filter_key, CustomFilterSetup custom_filter_setup)
{
    Transaction t(m_db);
    auto custom_filter = m_custom_filters.get_custom_filter(custom_filter_key);
    custom_filter.set_node_filters(custom_filter_setup.m_xml_node_filters);
    m_custom_filters.update_custom_filter(custom_filter, t);
    t.commit();
}

Key PlainFacade::get_tracking_stream_key(StreamTypeRef output_ref) const
{
    if (auto tracking_stream = m_tracking_streams.find_by_output(output_ref))
    {
        return tracking_stream->get_key();
    }
    return 0;
}

TrackingStreamInfo PlainFacade::get_tracking_stream_info(Key tracking_stream_key) const
{
    auto tracking_stream = m_tracking_streams.get_tracking_stream(tracking_stream_key);
    return TrackingStreamInfo(
        tracking_stream.get_key(),
        tracking_stream.get_name(),
        tracking_stream.get_input_ref().get_uuid(),
        tracking_stream.get_output_ref().get_uuid()
    );
}

Key PlainFacade::add_tracking_stream(std::wstring name, ReportTypeUUID input_uuid, std::string input_xpath)
{
    Transaction t(m_db);
    TrackingStream tracking_stream(std::move(name), m_report_types.get_report_type(input_uuid), std::move(input_xpath));
    m_tracking_streams.add_tracking_stream(tracking_stream, t);
    t.commit();
    return tracking_stream.get_key();
}

Key PlainFacade::get_stream_filter_key(StreamTypeRef output_ref) const
{
    if (auto stream_filter = m_stream_filters.find_by_output(output_ref))
    {
        return stream_filter->get_key();
    }
    return 0;
}

StreamFilterInfo PlainFacade::get_stream_filter_info(Key stream_filter_key) const
{
    auto stream_filter = m_stream_filters.get_stream_filter(stream_filter_key);
    return StreamFilterInfo(
        stream_filter.get_key(),
        stream_filter.get_name(),
        stream_filter.get_input_ref().get_uuid(),
        stream_filter.get_output_ref().get_uuid()
    );  
}

StreamFilterSetup PlainFacade::create_filter_setup(StreamTypeUUID input_uuid) const
{
    auto& input = m_stream_types.get_stream_type(input_uuid);
    auto stream_filter = m_stream_filters.create_stream_filter(L"", input);
    return StreamFilterSetup(
        xml_tools::clone_doc(stream_filter.get_filter_doc()),
        stream_filter.get_filter_def()
    );
}

Key PlainFacade::add_stream_filter(std::wstring name, StreamTypeUUID input_uuid, StreamFilterSetup stream_filter_setup)
{
    Transaction t(m_db);
    auto& input = m_stream_types.get_stream_type(input_uuid);   
    auto stream_filter = m_stream_filters.create_stream_filter(std::move(name), input);
    auto filter_doc = clone_doc(*stream_filter_setup.m_filter_doc);
    XML::bind_query_definition(*filter_doc, *stream_filter.get_filter_def());
    stream_filter.set_filter_doc(std::move(filter_doc));
    m_stream_filters.add_stream_filter(stream_filter, t);
    t.commit();

    return stream_filter.get_key();
}

StreamFilterSetup PlainFacade::get_stream_filter_setup(Key stream_filter_key) const
{
    auto stream_filter = m_stream_filters.get_stream_filter(stream_filter_key);
    return StreamFilterSetup(
        xml_tools::clone_doc(stream_filter.get_filter_doc()),
        stream_filter.get_filter_def()
    );
}

void PlainFacade::set_stream_filter_setup(Key stream_filter_key, StreamFilterSetup stream_filter_setup)
{
    Transaction t(m_db);
    auto stream_filter = m_stream_filters.get_stream_filter(stream_filter_key);
    auto filter_doc = clone_doc(*stream_filter_setup.m_filter_doc);
    XML::bind_query_definition(*filter_doc, *stream_filter.get_filter_def());
    stream_filter.set_filter_doc(std::move(filter_doc));
    m_stream_filters.update_stream_filter(stream_filter, t);
    t.commit();
}

ComplianceKey PlainFacade::get_compliance_key(ReportTypeRef output_ref) const
{
    if (auto compliance = m_compliances.find_by_output(output_ref))
    {
        return compliance->get_ref().get_key();
    }
    return ComplianceKey();
}

std::vector<ComplianceInfo> PlainFacade::get_compliances_info() const
{
    std::vector<ComplianceInfo> compliances_info;
    for (auto compliance : m_compliances.get_compliances())
    {
        compliances_info.push_back({
            compliance.get_ref().get_key(),
            L"",
            compliance.get_base_report_type_ref().get_uuid(),
            compliance.get_output_ref().get_uuid()
        });
    }
    return compliances_info;
}

ComplianceInfo PlainFacade::get_compliance_info(ComplianceKey compliance_key) const
{
    auto compliance = m_compliances.get_compliance(compliance_key);

    return ComplianceInfo {
        compliance.get_ref().get_key(),
        L"",
        compliance.get_base_report_type_ref().get_uuid(),
        compliance.get_output_ref().get_uuid()
    };
}

ComplianceSetup PlainFacade::get_compliance_setup(SubjectKey subject_key, ComplianceKey compliance_key) const
{
    auto subject_ref = get_subject_ref(m_registry, subject_key);
    auto compliance = m_compliances.get_compliance(compliance_key);
    
    return ComplianceSetup(
        clone_doc(*m_compliances.get_compliance_setup(compliance, subject_ref)),
        m_compliances.get_base_setup(compliance, subject_ref),
        m_compliances.get_compliance_def(compliance)
    );
}

void PlainFacade::set_compliance_setup(SubjectKey subject_key, ComplianceKey compliance_key, ComplianceSetup compliance_setup)
{
    Transaction t(m_db);    
    auto subject_ref = get_subject_ref(m_registry, subject_key);
    auto compliance = m_compliances.get_compliance(compliance_key);
    m_compliances.set_compliance_setup(compliance, subject_ref, std::move(compliance_setup.m_setup), t);
    t.commit();
}

ValidationKey PlainFacade::get_validation_key(ReportTypeRef output_ref) const
{
    if (auto validation = m_validations.find_by_output(output_ref))
    {
        return validation->get_ref().get_key();
    }
    return ValidationKey();
}

ValidationInfo PlainFacade::get_validation_info(ValidationKey validation_key) const
{
    auto validation = m_validations.get_validation(validation_key);

    return ValidationInfo(
        validation.get_ref().get_key(),
        validation.get_name(),
        validation.get_input_ref().get_uuid(),
        validation.get_output_ref().get_uuid()
    );
}

ValidationKey PlainFacade::add_validation(std::wstring name, ReportTypeUUID input_uuid, ValidationSettings default_settings) const
{
    Transaction t(m_db);
    auto& input = m_report_types.get_report_type(input_uuid);
    auto validation_ref = m_validations.add_validation({std::move(name), input, std::move(default_settings.m_validator)}, t);
    t.commit();
    return validation_ref.get_key();
}

ValidationSettings PlainFacade::create_validation_settings(ReportTypeUUID input_uuid) const
{
    auto& input = dynamic_cast<const RegularReportType&>(m_report_types.get_report_type(input_uuid));
    auto validator = XML::create_validator(*input.get_def_doc());
    return ValidationSettings(
        XML::create_validator(*input.get_def_doc()),
        nullptr,
        input.get_def_doc()
    );
}

ValidationSettings PlainFacade::get_validation_settings(SubjectKey subject_key, ValidationKey validation_key) const
{
    auto subject_ref = get_subject_ref(m_registry, subject_key);
    auto validation = m_validations.get_validation(validation_key);

    return ValidationSettings(
        clone_doc(*m_validations.get_validator(validation, subject_ref)),
        m_validations.get_base_validator(validation, subject_ref),
        m_validations.get_validator_def(validation)
    );
}

void PlainFacade::set_validation_settings(SubjectKey subject_key, ValidationKey validation_key, ValidationSettings validation_settings)
{
    Transaction t(m_db);
    auto subject_ref = get_subject_ref(m_registry, subject_key);
    auto validation = m_validations.get_validation(validation_key);
    m_validations.set_validator(validation, subject_ref, std::move(validation_settings.m_validator), t);
    t.commit();
}

Key PlainFacade::get_grouping_key(ReportTypeRef output_ref) const
{
    if (auto grouping_report = m_grouping_reports.find_by_output(output_ref))
    {
        return grouping_report->get_ref().get_key();
    }
    return Key();
}

GroupingInfo PlainFacade::get_grouping_info(Key grouping_key) const
{
    auto grouping_report = m_grouping_reports.get_grouping_report(grouping_key);
    return GroupingInfo(
        grouping_report.get_ref().get_key(),
        grouping_report.get_name(),
        grouping_report.get_input_ref().get_uuid(),
        grouping_report.get_output_ref().get_uuid()
    );
}

Key PlainFacade::add_grouping(std::wstring name, ReportTypeUUID input_uuid)
{
    Transaction t(m_db);
    auto& input = m_report_types.get_report_type(input_uuid);
    auto grouping_report = m_grouping_reports.create_grouping_report(std::move(name), input);
    m_grouping_reports.add_grouping_report(grouping_report, ReportTypeUUID(stl_tools::gen_uuid()), t);
    t.commit();
    return grouping_report.get_ref().get_key();
}

void PlainFacade::remove_custom_report(ReportTypeRef report_type_ref, bool recursive, Transaction& t)
{
    if (recursive)
    {
        for (auto selection : m_transformations.m_selections.find_by_input(report_type_ref))
        {
            remove_custom_report(selection->get_plan().m_output_ref, recursive, t);
        }
    }

    if (auto custom_query = m_custom_queries.find_by_output(report_type_ref))
    {
        m_custom_queries.remove_custom_query(custom_query->get_ref().get_key(), t);
    }
    else if (auto custom_filter = m_custom_filters.find_by_output(report_type_ref))
    {
        m_custom_filters.remove_custom_filter(custom_filter->get_key(), t);
    }
    else if (auto grouping = m_grouping_reports.find_by_output(report_type_ref))
    {
        m_grouping_reports.remove_grouping_report(grouping->get_ref().get_key(), t);
    }
    else if (auto validation = m_validations.find_by_output(report_type_ref))
    {
        m_validations.remove_validation(validation->get_ref().get_key(), t);
    }
    else
    {
        throw std::logic_error("Invalid report type");
    }
}

void PlainFacade::remove_custom_report(ReportTypeUUID report_type_uuid, bool recursive)
{
    Transaction t(m_db);
    auto& report_type = m_report_types.get_report_type(report_type_uuid);
    remove_custom_report(report_type, recursive, t);
    t.commit();
}

std::shared_ptr<Content> PlainFacade::create_pattern(ReportTypeUUID report_type_uuid) const
{
    auto& report_type = m_report_types.get_report_type(report_type_uuid);
    return report_type.make_empty_content();
}

std::shared_ptr<Content> PlainFacade::create_pattern(ReportTypeUUID report_type_uuid, Blob blob) const
{
    auto& report_type = m_report_types.get_report_type(report_type_uuid);
    return report_type.make_content(std::move(blob));
}

std::shared_ptr<Content> PlainFacade::get_pattern(SubjectKey subject_key, ReportTypeUUID report_type_uuid) const
{
    auto subject_ref = get_subject_ref(m_registry, subject_key);
    auto& report_type = m_report_types.get_report_type(report_type_uuid);
    return m_integrity_checking.get_pattern(subject_ref, report_type);
}

std::shared_ptr<Content> PlainFacade::get_effective_pattern(SubjectKey subject_key, ReportTypeUUID report_type_uuid) const
{
    auto subject_ref = get_subject_ref(m_registry, subject_key);
    auto& report_type = m_report_types.get_report_type(report_type_uuid);
    return m_integrity_checking.get_effective_pattern(subject_ref, report_type);
}

void PlainFacade::set_pattern(SubjectKey subject_key, ReportTypeUUID report_type_uuid, Blob blob)
{
    Transaction t(m_db);
    auto subject_ref = get_subject_ref(m_registry, subject_key);
    auto& report_type = m_report_types.get_report_type(report_type_uuid);
    m_integrity_checking.set_pattern(subject_ref, report_type, report_type.make_content(std::move(blob)), t);
    t.commit();
}

void PlainFacade::reset_pattern(SubjectKey subject_key, ReportTypeUUID report_type_uuid)
{
    Transaction t(m_db);
    auto subject_ref = get_subject_ref(m_registry, subject_key);
    auto& report_type = m_report_types.get_report_type(report_type_uuid);
    m_integrity_checking.set_pattern(subject_ref, report_type, nullptr, t);
    t.commit();
}

UUID PlainFacade::add_custom_action(std::wstring name, ActionUUID action_uuid, Params params)
{
    Transaction t(m_db);
    auto& action = m_actions.get_action(action_uuid);
    auto custom_action = m_custom_actions.create_custom_action(name, action);
    custom_action.set_params_doc(std::move(params.m_params_doc));
    m_custom_actions.add_custom_action(custom_action, t);
    t.commit();
    return custom_action.get_ref().get_uuid();
}

void PlainFacade::remove_custom_action(UUID custom_action_uuid)
{
    Transaction t(m_db);
    m_custom_actions.remove_custom_action(custom_action_uuid, t);
    t.commit();
}

std::vector<ScheduleInfo> PlainFacade::get_schedules_info() const
{
    std::vector<ScheduleInfo> result;
    for (auto& schedule : m_scheduler.get_schedules())
    {
        result.emplace_back(
            schedule.get_ref().get_key(),
            schedule.get_name(), 
            ScheduleSettings::Period(schedule.get_period()),
            schedule.get_day(),
            schedule.get_week_day(),
            schedule.get_day_time(),
            schedule.get_scheduled_time()
        );
    }
    return result;
}

ScheduleInfo PlainFacade::get_schedule_info(Key schedule_key) const
{
    auto schedule = m_scheduler.get_schedule(schedule_key);
    return ScheduleInfo(
        schedule.get_ref().get_key(),
        schedule.get_name(),
        ScheduleSettings::Period(schedule.get_period()),
        schedule.get_day(),
        schedule.get_week_day(),
        schedule.get_day_time(),
        schedule.get_scheduled_time()
    );
}

Key PlainFacade::add_schedule(const ScheduleSettings& schedule_settings)
{
    Schedule schedule(
        schedule_settings.m_name,
        Schedule::Period(schedule_settings.m_period),
        schedule_settings.m_day,
        schedule_settings.m_week_day,
        schedule_settings.m_day_time
    );

    Transaction t(m_db);
    m_scheduler.add_schedule(schedule, t);
    t.commit();
    return schedule.get_ref().get_key();
}

void PlainFacade::update_schedule(Key schedule_key, const ScheduleSettings& schedule_settings)
{
    Transaction t(m_db);
    auto schedule = m_scheduler.get_schedule(schedule_key);
    schedule.set_name(schedule_settings.m_name);
    schedule.set_period(Schedule::Period(schedule_settings.m_period));
    schedule.set_day(schedule_settings.m_day);
    schedule.set_week_day(schedule_settings.m_week_day);
    schedule.set_day_time(schedule_settings.m_day_time);
    m_scheduler.update_schedule(schedule, t);
    t.commit();
}

void PlainFacade::remove_schedule(Key schedule_key)
{
    Transaction t(m_db);
    m_scheduler.remove_schedule(schedule_key, t);   
    t.commit();
}

}} //namespace TR { namespace Core {