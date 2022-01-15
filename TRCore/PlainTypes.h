#pragma once
#include "BasisTrivialTypes.h"
#include "RegistryTrivialTypes.h"
#include "ServicesTrivialTypes.h"
#include "AppTrivialTypes.h"
#include "StreamMessage.h"
#include "Content.h"
#include "Diff.h"
#include "XmlNodeFilter.h"
#include "TRXML\TRXML.h"
#include <boost\range\algorithm.hpp>

namespace TR {
using Core::Blob;
using Core::UUID;
using Core::Key;
using Core::SourceKey;
using Core::RoleKey;
using Core::SourceTypeUUID;
using Core::ReportTypeUUID;
using Core::StreamTypeUUID;
using Core::ActionUUID;
using Core::ComplianceKey;
using Core::ValidationKey;
using Core::ArchivedReportKey;
using Core::Content;
using Core::TextContent;
using Core::XmlContent;
using Core::RegularContent;
using Core::ValidatedContent;
using Core::CheckListContent;
using Core::ComplianceContent;
using Core::Diff;
using Core::PlainTextDiff;
using Core::PlainXmlDiff;
using Core::RegularDiff;
using Core::StreamMessage;
using Core::XmlNodeFilter;

using SubjectKey = boost::variant<SourceKey, RoleKey>;

enum class DeferReportPolicy { DEFAULT_POLICY = 0, DEFER_REPORT, NOT_DEFER_REPORT };
enum class ReportVisibility { DEFAULT_VISIBILITY = 0, VISIBLE_REPORT, INVISIBLE_REPORT };
enum class EnableReportPolicy { DEFAULT_POLICY = 0, ENABLE_REPORT, DISABLE_REPORT };
enum class NotificationSeverity {DEFAULT = 0, INFO, WARN, FAIL, NONE};
enum class NotificationPolicy { DEFAULT_POLICY = 0, ENABLE_NOTIFICATION, DISABLE_NOTIFICATION };

enum class IntegrityCheckingPolicy { DEFAULT_POLICY = 0, ENABLE_CHECKING, DISABLE_CHECKING };
enum class IntegrityCheckingPatternChoise { DEFAULT_PATTERN = 0, CUSTOM_PATTERN };
enum class IntegrityCheckingResult { UNDEFINED = 0, MATCHED, MISMATCHED };

static UUID COMPLIANCE_NOTIFICATION = stl_tools::gen_uuid(L"D47862BF-CBE7-417E-B54B-8457BA327AAC");
enum class ComplianceCheckingPolicy { DEFAULT_POLICY = 0, ENABLE_CHECKING, DISABLE_CHECKING };
enum class ComplianceCheckingResult { UNDEFINED = 0, CHECKING_SUCCEEDED, CHECKING_FAILED };

enum class ValidationPolicy { DEFAULT_POLICY = 0, ENABLE_VALIDATION, DISABLE_VALIDATION };
enum class ValidationResult { UNDEFINED = 0, VALIDATION_SUCCEEDED, VALIDATION_FAILED };

enum class ArchivingPolicy { DEFAULT_ARCHIVING = 0, DISABLE_ARCHIVING, ENABLE_ARCHIVING };
enum class ArchivedExpiration { EXPIRED_DEFAULT = 0, EXPIRED_NEVER, EXPIRED_IN_HOUR, EXPIRED_IN_DAY, EXPIRED_IN_MONTH, EXPIRED_IN_YEAR };

enum class Access { EMPTY_ACCESS = 0x00, READ_ACCESS = 0x01, WRITE_ACCESS = 0x02, EXEC_ACCESS = 0x04, FULL_ACCESS = 0x07 };
using AccessRights = stl_tools::flag_type<Access>;

enum class StreamLength { DEFAULT = 0, FOREVER, HOUR, DAY, MONTH, YEAR, NONE };

using XML::XmlDoc;
using XML::XmlDefDoc;
using XML::XmlRegularDoc;
using XML::XmlQueryDoc;
using XML::XmlComplianceDoc;
using XML::XmlComplianceSetup;
using XML::XmlComplianceDef;
using XML::XmlPropertiesDef;
using XML::XmlPropertiesDoc;

const auto GROUP_SOURCE_TYPE_UUID = SourceTypeUUID(stl_tools::gen_uuid(L"E82F3179-49DB-463C-83D1-A36D110C5A43"));
const auto DEFAULT_ROLE_KEY = RoleKey(1);
const auto ROOT_GROUP_KEY = SourceKey(1);
const auto INVALID_SOURCE_KEY = SourceKey(0);

enum class Severity { SEVERITY_INFO = 0, SEVERITY_WARNING, SEVERITY_FAIL };
struct Alert
{
    Alert() = default; //required by std::promise
    Alert(SourceKey source_key, Severity severity, std::wstring message):
        m_source_key(source_key),
        m_severity(severity),
        m_message(std::move(message))
    {
    }

    SourceKey m_source_key;
    Severity m_severity;
    std::wstring m_message;
};

struct CheckingAlert: Alert
{
    enum CheckingType { INTEGRITY_CHECKING = 0, COMPLIANCE, VALIDATION};
    CheckingAlert() = default; //required by std::promise
    CheckingAlert(CheckingType checking_type, SourceKey source_key, ReportTypeUUID report_type, Severity severity, std::wstring message):
        Alert(source_key, severity, std::move(message)),
        m_checking_type(checking_type),
        m_report_type(report_type)
    {
    }

    CheckingType m_checking_type;
    ReportTypeUUID m_report_type;
};

struct ScheduledReloadAlert: Alert
{
    ScheduledReloadAlert() = default; //required by std::promise
    ScheduledReloadAlert(SourceKey source_key, ReportTypeUUID report_type, Severity severity, std::wstring message):
        Alert(source_key, severity, std::move(message)),
        m_report_type(report_type)
    {
    }

    ReportTypeUUID m_report_type;
};

struct ScheduledLaunchAlert: Alert
{
    ScheduledLaunchAlert() = default; //required by std::promise
    ScheduledLaunchAlert(SourceKey source_key, ActionUUID action_uuid, UUID shortcut_uuid, Severity severity, std::wstring message):
        Alert(source_key, severity, std::move(message)),
        m_action_uuid(action_uuid),
        m_shortcut_uuid(shortcut_uuid)
    {
    }

    ActionUUID m_action_uuid;
    UUID m_shortcut_uuid;
};

using VariantAlert = boost::variant<CheckingAlert, ScheduledReloadAlert, ScheduledLaunchAlert>;

struct ReportTypeInfo
{
    enum class Syntax { PLAIN_TEXT = 1, PLAIN_XML, REGULAR_XML, CHECKLIST_XML, VALIDATED_XML, COMPLIANCE_XML };
    enum class Trait {VIEW = 0x01 };
    using Traits = stl_tools::flag_type<Trait>;

    ReportTypeInfo() = default; //required by std::promise
    ReportTypeInfo(ReportTypeUUID uuid, std::wstring name, Syntax syntax, int version, Traits traits, ComplianceKey compliance_key, ValidationKey validation_key, Key custom_query_key, Key custom_filter_key, Key grouping_key):
        m_uuid(uuid),
        m_name(std::move(name)),
        m_syntax(syntax),
        m_version(version),
        m_traits(traits),
        m_compliance_key(compliance_key),
        m_validation_key(validation_key),
        m_custom_query_key(custom_query_key),
        m_custom_filter_key(custom_filter_key),
        m_grouping_key(grouping_key)
    {
    }

    ReportTypeUUID m_uuid;
    std::wstring m_name;
    Syntax m_syntax;
    int m_version;
    Traits m_traits;
    ComplianceKey m_compliance_key;
    ValidationKey m_validation_key;
    Key m_custom_query_key;
    Key m_custom_filter_key;
    Key m_grouping_key;
};

struct StreamTypeInfo
{
    StreamTypeInfo() = default; //required by std::promise
    StreamTypeInfo(StreamTypeUUID uuid, std::wstring name, int version, Key tracking_stream_key):
        m_uuid(uuid),
        m_name(std::move(name)),    
        m_version(version),
        m_tracking_stream_key(tracking_stream_key)
    {
    }

    StreamTypeUUID m_uuid;
    std::wstring m_name;
    int m_version;
    Key m_tracking_stream_key;
};

struct ActionShortcutInfo
{
    ActionShortcutInfo() = default; //required by std::promise
    ActionShortcutInfo(UUID uuid, std::wstring name, boost::optional<ReportTypeUUID> output_uuid):
        m_uuid(uuid),
        m_name(std::move(name)),
        m_output_uuid(output_uuid)
    {
    }

    UUID m_uuid;
    std::wstring m_name;
    boost::optional<ReportTypeUUID> m_output_uuid;
};

struct ActionInfo
{
    ActionInfo() = default; //required by std::promise
    ActionInfo(ActionUUID uuid, std::wstring name):
        m_uuid(uuid),
        m_name(std::move(name))
    {
    }

    ActionUUID m_uuid;
    std::wstring m_name;
    std::vector<ActionShortcutInfo> m_shortcuts;
};

struct Params
{
    Params() = default; //required by std::promise
    Params(std::unique_ptr<XmlPropertiesDoc> params_doc, std::shared_ptr<const XmlPropertiesDef> params_def):
        m_params_doc(std::move(params_doc)),
        m_params_def(params_def)
    {
    }

    Params(const Params&) = delete;
    Params& operator = (const Params& params) = delete;

    // because VC++2013 does not generate move constructor
    Params(Params&& params):
        m_params_doc(std::move(params.m_params_doc)),
        m_params_def(params.m_params_def)
    {
    }

    // because VC++2013 does not generate move assign operator
    Params& operator = (Params&& params)
    {
        m_params_doc = std::move(params.m_params_doc);
        m_params_def = params.m_params_def;
        return *this;
    }

    Params clone() const
    {
        return Params(xml_tools::clone_doc(*m_params_doc), m_params_def);
    }

    std::unique_ptr<XmlPropertiesDoc> m_params_doc;
    std::shared_ptr<const XmlPropertiesDef> m_params_def;
};

struct SourceTypeInfo
{   
    SourceTypeInfo() = default; //required by std::promise
    SourceTypeInfo(SourceTypeUUID uuid, std::wstring name, UUID family_uuid):
        m_uuid(uuid),
        m_name(std::move(name)),
        m_family_uuid(family_uuid)
    {
    }

    SourceTypeUUID m_uuid;
    std::wstring m_name;    
    UUID m_family_uuid;
    std::vector<ReportTypeUUID> m_download_uuids;
    std::vector<UUID> m_action_uuids;
    std::vector<UUID> m_action_shortcut_uuids;
};

struct CurrentReportInfo
{
    CurrentReportInfo() = default; //required by std::promise
    CurrentReportInfo(SourceKey source_key, ReportTypeUUID report_type_uuid, time_t modified_time, time_t update_time, bool empty, bool deferred):
        m_source_key(source_key),
        m_report_type_uuid(report_type_uuid),
        m_modified_time(modified_time),
        m_update_time(update_time),
        m_empty(empty),
        m_deferred(deferred)
    {
    }

    SourceKey m_source_key;
    ReportTypeUUID m_report_type_uuid;
    time_t m_modified_time;
    time_t m_update_time;
    bool m_empty;
    bool m_deferred;
};

struct ArchivedReportInfo
{
    ArchivedReportInfo() = default; //required by std::promise
    ArchivedReportInfo(ArchivedReportKey key, SourceKey source_key, ReportTypeUUID report_type_uuid, std::shared_ptr<Diff> diff, time_t time):
        m_key(key),
        m_source_key(source_key),
        m_report_type_uuid(report_type_uuid),
        m_diff(diff),
        m_time(time)
    {
    }

    ArchivedReportKey m_key;
    SourceKey m_source_key;
    ReportTypeUUID m_report_type_uuid;
    std::shared_ptr<Diff> m_diff;
    time_t m_time;
};

struct StreamInfo
{
    StreamInfo() = default; //required by std::promise
    StreamInfo(SourceKey source_key, StreamTypeUUID stream_type_uuid, time_t time):
        m_source_key(source_key),
        m_stream_type_uuid(stream_type_uuid),
        m_time(time)
    {
    }

    SourceKey m_source_key;
    StreamTypeUUID m_stream_type_uuid;
    time_t m_time;
};

struct StreamSegment
{
    StreamSegment() = default; //required by std::promise
    StreamSegment(SourceKey source_key, StreamTypeUUID stream_type_uuid):
        m_source_key(source_key),
        m_stream_type_uuid(stream_type_uuid)
    {
    }

    SourceKey m_source_key;
    StreamTypeUUID m_stream_type_uuid;
    std::vector<StreamMessage> m_messages;
};

struct RoleSettings
{
    RoleSettings() = default; //required by std::promise
    RoleSettings(std::wstring name, RoleKey parent_key):
        m_name(name),
        m_parent_key(parent_key)
    {
    }

    std::wstring m_name;
    RoleKey m_parent_key;
};

struct RoleInfo: RoleSettings
{
    RoleInfo() = default; //required by std::promise
    RoleInfo(RoleKey key, std::wstring name, RoleKey parent_key):
        RoleSettings(std::move(name), parent_key),
        m_key(key)
    {
    }

    RoleKey m_key;
};

struct SourceSettings
{
    SourceSettings() = default; //required by std::promise
    SourceSettings(std::wstring name, RoleKey role_key, SourceKey parent_key, SourceTypeUUID source_type_uuid):
        m_name(std::move(name)),
        m_role_key(role_key),
        m_parent_key(parent_key),
        m_source_type_uuid(source_type_uuid)
    {
    }

    std::wstring m_name;
    RoleKey m_role_key;
    SourceKey m_parent_key;
    SourceTypeUUID m_source_type_uuid;
};

struct SourceInfo: SourceSettings
{   
    SourceInfo() = default; //required by std::promise
    SourceInfo(SourceKey key, std::wstring name, RoleKey role_key, SourceKey parent_key, SourceTypeUUID source_type_uuid):
        SourceSettings(std::move(name), role_key, parent_key, source_type_uuid),
        m_key(key)
    {
    }

    SourceKey m_key;    
    std::vector<CurrentReportInfo> m_current_reports;
    std::vector<StreamInfo> m_streams;
    std::vector<VariantAlert> m_alerts;
};

struct SourceConfig
{
    SourceConfig() = default; //required by std::promise
    SourceConfig(std::unique_ptr<XmlPropertiesDoc> config, std::shared_ptr<const XmlPropertiesDef> config_def):
        m_config(std::move(config)),
        m_config_def(config_def)
    {
    }

    // because VC++2013 does not generate move constructor
    SourceConfig(SourceConfig&& source_config):
        m_config(std::move(source_config.m_config)),
        m_config_def(source_config.m_config_def)
    {
    }

    // because VC++2013 does not generate move assign operator
    SourceConfig& operator = (SourceConfig&& source_config)
    {
        m_config = std::move(source_config.m_config);
        m_config_def = source_config.m_config_def;
        return *this;
    }

    std::unique_ptr<XmlPropertiesDoc> m_config;
    std::shared_ptr<const XmlPropertiesDef> m_config_def;
};

struct Link
{
    Link() = default; //required by std::promise
    Link(SourceKey parent_key, SourceKey child_key):
        m_parent_key(parent_key),
        m_child_key(child_key)
    {
    }

    SourceKey m_parent_key;
    SourceKey m_child_key;
};

struct ScheduleSettings
{
    enum class Period { Never = 0, Hourly, Daily, Weekly, Mounthly, Often };

    ScheduleSettings() = default; //required by std::promise
    ScheduleSettings(std::wstring name, Period period, int day, int week_day, time_t day_time):
        m_name(std::move(name)),
        m_period(period),
        m_day(day),
        m_week_day(week_day),
        m_day_time(day_time)
    {
    }

    std::wstring m_name;
    Period m_period;
    int m_day;
    int m_week_day;
    time_t m_day_time;
};

struct ScheduleInfo: ScheduleSettings
{
    ScheduleInfo() = default; //required by std::promise
    ScheduleInfo(Key schedule_key, std::wstring name, Period period, int day, int week_day, time_t day_time, time_t scheduled_time):
        ScheduleSettings(std::move(name), period, day, week_day, day_time),
        m_schedule_key(schedule_key),
        m_scheduled_time(scheduled_time)
    {
    }
    Key m_schedule_key;
    time_t m_scheduled_time;
};

struct ReportGeneratingOptions
{
    ReportGeneratingOptions() = default; //required by std::promise
    ReportGeneratingOptions(ReportTypeUUID report_type_uuid, EnableReportPolicy enable_report_policy, ReportVisibility report_visibility, DeferReportPolicy defer_report_policy, boost::optional<Key> schedule_key):
        m_report_type_uuid(report_type_uuid),
        m_enable_report_policy(enable_report_policy),
        m_report_visibility(report_visibility),
        m_defer_report_policy(defer_report_policy),
        m_schedule_key(schedule_key)
    {
    }

    ReportTypeUUID m_report_type_uuid;
    EnableReportPolicy m_enable_report_policy;
    ReportVisibility m_report_visibility;
    DeferReportPolicy m_defer_report_policy;
    boost::optional<Key> m_schedule_key;
};

struct SubjectReportGenerating: ReportGeneratingOptions
{
    SubjectReportGenerating() = default; //required by std::promise
    SubjectReportGenerating(SubjectKey subject_key, ReportGeneratingOptions options, bool default_enabled, bool default_visibile, bool default_deferred, Key default_schedule_key):
        ReportGeneratingOptions(options),   
        m_subject_key(subject_key),
        m_default_enabled(default_enabled),
        m_default_visibile(default_visibile),
        m_default_deferred(default_deferred),
        m_default_schedule_key(default_schedule_key)
    {
    }

    bool modified() const
    {
        return m_enable_report_policy != EnableReportPolicy::DEFAULT_POLICY ||
            m_report_visibility != ReportVisibility::DEFAULT_VISIBILITY ||
            m_defer_report_policy != DeferReportPolicy::DEFAULT_POLICY ||
            m_schedule_key != boost::none;
    }
    
    SubjectKey m_subject_key;
    bool m_default_enabled;
    bool m_default_visibile;
    bool m_default_deferred;
    Key m_default_schedule_key;
};

struct IntegrityCheckingOptions
{
    IntegrityCheckingOptions() = default; //required by std::promise
    IntegrityCheckingOptions(ReportTypeUUID report_type_uuid, IntegrityCheckingPolicy integrity_checking_policy, IntegrityCheckingPatternChoise pattern_choise, NotificationPolicy notification_policy):
        m_report_type_uuid(report_type_uuid),
        m_integrity_checking_policy(integrity_checking_policy),
        m_pattern_choise(pattern_choise),
        m_notification_policy(notification_policy)
    {
    }

    ReportTypeUUID m_report_type_uuid;
    IntegrityCheckingPolicy m_integrity_checking_policy;
    IntegrityCheckingPatternChoise m_pattern_choise;
    NotificationPolicy m_notification_policy;
};

struct SubjectIntegrityChecking: IntegrityCheckingOptions
{
    SubjectIntegrityChecking() = default; //required by std::promise
    SubjectIntegrityChecking(SubjectKey subject_key, IntegrityCheckingOptions options, bool has_pattern, bool default_checking_on, bool default_mailing_enabled):
        IntegrityCheckingOptions(options),      
        m_subject_key(subject_key),
        m_has_pattern(has_pattern),
        m_default_checking_on(default_checking_on),
        m_default_mailing_enabled(default_mailing_enabled)
    {
    }

    bool modified() const
    {
        return m_integrity_checking_policy != IntegrityCheckingPolicy::DEFAULT_POLICY || m_pattern_choise != IntegrityCheckingPatternChoise::DEFAULT_PATTERN || m_has_pattern == true;
    }

    SubjectKey m_subject_key;
    bool m_has_pattern;
    bool m_default_checking_on;
    bool m_default_mailing_enabled;
};

struct ComplianceCheckingOptions
{
    ComplianceCheckingOptions() = default; //required by std::promise
    ComplianceCheckingOptions(ReportTypeUUID report_type_uuid, ComplianceCheckingPolicy compliance_checking_policy, NotificationPolicy notification_policy):
        m_report_type_uuid(report_type_uuid),
        m_compliance_checking_policy(compliance_checking_policy),
        m_notification_policy(notification_policy)
    {
    }

    ReportTypeUUID m_report_type_uuid;
    ComplianceCheckingPolicy m_compliance_checking_policy;
    NotificationPolicy m_notification_policy;
};

struct SubjectComplianceChecking: ComplianceCheckingOptions
{
    SubjectComplianceChecking() = default; //required by std::promise
    SubjectComplianceChecking(SubjectKey subject_key, ComplianceCheckingOptions options, bool customized, bool default_checking_on, bool default_notification_enabled):
        ComplianceCheckingOptions(options),
        m_subject_key(subject_key),
        m_customized(customized),
        m_default_checking_on(default_checking_on),
        m_default_notification_enabled(default_notification_enabled)
    {
    }

    bool modified() const
    {
        return m_compliance_checking_policy != ComplianceCheckingPolicy::DEFAULT_POLICY || m_customized == true;
    }
    
    SubjectKey m_subject_key;
    bool m_customized;
    bool m_default_checking_on;
    bool m_default_notification_enabled;
};

struct ValidatingOptions
{
    ValidatingOptions() = default; //required by std::promise
    ValidatingOptions(ReportTypeUUID report_type_uuid, ValidationPolicy validation_policy, NotificationPolicy notification_policy):
        m_report_type_uuid(report_type_uuid),
        m_validation_policy(validation_policy),
        m_notification_policy(notification_policy)
    {
    }

    ReportTypeUUID m_report_type_uuid;
    ValidationPolicy m_validation_policy;
    NotificationPolicy m_notification_policy;
};

struct SubjectValidating: ValidatingOptions
{
    SubjectValidating() = default; //required by std::promise
    SubjectValidating(SubjectKey subject_key, ValidatingOptions options, bool customized, bool default_validation_on, bool default_notification_enabled):
        ValidatingOptions(options),
        m_subject_key(subject_key),
        m_customized(customized),
        m_default_validation_on(default_validation_on),
        m_default_notification_enabled(default_notification_enabled)
    {
    }

    bool modified() const
    {
        return m_validation_policy != ValidationPolicy::DEFAULT_POLICY || m_customized == true;
    }

    SubjectKey m_subject_key;
    bool m_customized;
    bool m_default_validation_on;
    bool m_default_notification_enabled;
};

struct ArchivingOptions
{
    ArchivingOptions() = default; //required by std::promise
    ArchivingOptions(ReportTypeUUID report_type_uuid, ArchivingPolicy archiving_policy, ArchivedExpiration archiving_expiration):
        m_report_type_uuid(report_type_uuid),
        m_archiving_policy(archiving_policy),
        m_archiving_expiration(archiving_expiration)
    {
    }

    ReportTypeUUID m_report_type_uuid;
    ArchivingPolicy m_archiving_policy;
    ArchivedExpiration m_archiving_expiration;
};

struct SubjectArchiving: ArchivingOptions
{
    SubjectArchiving() = default; //required by std::promise
    SubjectArchiving(SubjectKey subject_key, ArchivingOptions options, ArchivingPolicy default_archiving_policy, ArchivedExpiration default_archiving_expiration):
        ArchivingOptions(options),
        m_subject_key(subject_key),
        m_default_archiving_policy(default_archiving_policy),
        m_default_archiving_expiration(default_archiving_expiration)
    {
    }

    bool modified() const
    {
        return m_archiving_policy != ArchivingPolicy::DEFAULT_ARCHIVING || m_archiving_expiration != ArchivedExpiration::EXPIRED_DEFAULT;
    }
    

    SubjectKey m_subject_key;
    ArchivingPolicy m_default_archiving_policy;
    ArchivedExpiration m_default_archiving_expiration;
};

struct ReportOptions
{
    ReportOptions() = default; //required by std::promise
    ReportOptions(ReportTypeUUID report_type_uuid):
        m_report_type_uuid(report_type_uuid),
        m_report_generating(report_type_uuid, TR::EnableReportPolicy::DEFAULT_POLICY, TR::ReportVisibility::DEFAULT_VISIBILITY, TR::DeferReportPolicy::DEFAULT_POLICY, boost::none)
    {
    }

    ReportTypeUUID m_report_type_uuid;
    ReportGeneratingOptions m_report_generating;
    boost::optional<IntegrityCheckingOptions> m_integrity_checking;
    boost::optional<ComplianceCheckingOptions> m_compliance_checking;
    boost::optional<ValidatingOptions> m_validating;
    boost::optional<ArchivingOptions> m_archiving;
};

struct SubjectReportOptions
{
    SubjectReportOptions() = default; //required by std::promise
    SubjectReportOptions(SubjectKey subject_key, ReportTypeUUID report_type_uuid):
        m_subject_key(subject_key),
        m_report_type_uuid(report_type_uuid)
    {
    }

    operator ReportOptions() const
    {
        ReportOptions report_options(m_report_type_uuid);
        report_options.m_report_generating = m_report_generating;

        if (m_integrity_checking)
        {
            report_options.m_integrity_checking = *m_integrity_checking;
        }

        if (m_compliance_checking)
        {
            report_options.m_compliance_checking = *m_compliance_checking;
        }

        if (m_validating)
        {
            report_options.m_validating = *m_validating;
        }

        if (m_archiving)
        {
            report_options.m_archiving = *m_archiving;
        }

        return report_options;
    }

    SubjectKey m_subject_key;
    ReportTypeUUID m_report_type_uuid;
    SubjectReportGenerating m_report_generating;
    boost::optional<SubjectIntegrityChecking> m_integrity_checking;
    boost::optional<SubjectComplianceChecking> m_compliance_checking;
    boost::optional<SubjectValidating> m_validating;
    boost::optional<SubjectArchiving> m_archiving;
};

struct StreamGeneratingOptions
{
    StreamGeneratingOptions() = default; //required by std::promise
    StreamGeneratingOptions(StreamTypeUUID stream_type_uuid, ReportVisibility stream_visibility, StreamLength stream_length, NotificationSeverity notification_severity):
        m_stream_type_uuid(stream_type_uuid),
        m_stream_visibility(stream_visibility),
        m_stream_length(stream_length),
        m_notification_severity(notification_severity)
    {
    }

    StreamTypeUUID m_stream_type_uuid;  
    ReportVisibility m_stream_visibility;
    StreamLength m_stream_length;
    NotificationSeverity m_notification_severity;
};

struct SubjectStreamGenerating: StreamGeneratingOptions
{
    SubjectStreamGenerating() = default; //required by std::promise
    SubjectStreamGenerating(SubjectKey subject_key, StreamGeneratingOptions options, bool default_visibile, StreamLength default_stream_length, NotificationSeverity default_notification_severity):
        StreamGeneratingOptions(options),
        m_subject_key(subject_key),
        m_default_visibile(default_visibile),
        m_default_stream_length(default_stream_length),
        m_default_notification_severity(default_notification_severity)
    {
    }

    bool modified() const
    {
        return m_stream_visibility != ReportVisibility::DEFAULT_VISIBILITY;
    }

    SubjectKey m_subject_key;
    bool m_default_visibile;
    StreamLength m_default_stream_length;
    NotificationSeverity m_default_notification_severity;
};

struct StreamOptions
{
    StreamOptions() = default; //required by std::promise
    StreamOptions(SubjectKey subject_key, StreamTypeUUID stream_type_uuid):
        m_subject_key(subject_key),
        m_stream_type_uuid(stream_type_uuid)
    {
    }

    SubjectKey m_subject_key;
    StreamTypeUUID m_stream_type_uuid;
    StreamGeneratingOptions m_stream_generating;
};

struct SubjectStreamOptions
{
    SubjectStreamOptions() = default; //required by std::promise
    SubjectStreamOptions(SubjectKey subject_key, StreamTypeUUID stream_type_uuid):
        m_subject_key(subject_key),
        m_stream_type_uuid(stream_type_uuid)
    {
    }

    operator StreamOptions() const
    {
        StreamOptions stream_options(m_subject_key, m_stream_type_uuid);
        stream_options.m_stream_generating = m_stream_generating;
        return stream_options;
    }

    SubjectKey m_subject_key;
    StreamTypeUUID m_stream_type_uuid;
    SubjectStreamGenerating m_stream_generating;
};

struct ActionOptions
{
    ActionOptions() = default; //required by std::promise
    ActionOptions(ActionUUID action_uuid, boost::optional<UUID> shortcut_uuid, boost::optional<Key> schedule_key):
        m_action_uuid(action_uuid),
        m_shortcut_uuid(shortcut_uuid),
        m_schedule_key(schedule_key)
    {
    }

    ActionUUID m_action_uuid;
    boost::optional<UUID> m_shortcut_uuid;
    boost::optional<Key> m_schedule_key;
};

struct SubjectActionOptions: ActionOptions
{
    SubjectActionOptions() = default; //required by std::promise
    SubjectActionOptions(SubjectKey subject_key, ActionOptions action_options):
        ActionOptions(action_options),
        m_subject_key(subject_key)
    {
    }

    bool modified() const
    {
        return m_schedule_key != boost::none;
    }

    SubjectKey m_subject_key;
    boost::optional<Key> m_default_schedule_key;
};

using VariantOptions = boost::variant<
    SubjectReportGenerating,    
    SubjectIntegrityChecking,
    SubjectComplianceChecking,
    SubjectValidating,
    SubjectArchiving,
    SubjectStreamGenerating,
    SubjectActionOptions>;

using ModifiedOptions = std::vector<VariantOptions>;

inline void get_modified_options(const SubjectReportOptions& subject_report_options, ModifiedOptions& modified_options)
{
    if (subject_report_options.m_report_generating.modified())
    {
        modified_options.push_back(subject_report_options.m_report_generating);
    }

    if (subject_report_options.m_integrity_checking && subject_report_options.m_integrity_checking->modified())
    {
        modified_options.push_back(*subject_report_options.m_integrity_checking);
    }

    if (subject_report_options.m_compliance_checking && subject_report_options.m_compliance_checking->modified())
    {
        modified_options.push_back(*subject_report_options.m_compliance_checking);
    }

    if (subject_report_options.m_validating && subject_report_options.m_validating->modified())
    {
        modified_options.push_back(*subject_report_options.m_validating);
    }

    if (subject_report_options.m_archiving && subject_report_options.m_archiving->modified())
    {
        modified_options.push_back(*subject_report_options.m_archiving);
    }
}

inline void get_modified_options(const SubjectStreamOptions& subject_stream_options, ModifiedOptions& modified_options)
{
    if (subject_stream_options.m_stream_generating.modified())
    {
        modified_options.push_back(subject_stream_options.m_stream_generating);
    }
}

inline void get_modified_options(const SubjectActionOptions& subject_action_options, ModifiedOptions& modified_options)
{
    if (subject_action_options.modified())
    {
        modified_options.push_back(subject_action_options);
    }
}

struct ReportTypeOptions
{
    ReportTypeOptions() = default; //required by std::promise
    ReportTypeOptions(ReportTypeUUID report_type_uuid):
        m_report_type_uuid(report_type_uuid)
    {
    }

    ReportTypeUUID m_report_type_uuid;
    std::vector<SubjectReportOptions> m_subject_options;
};

inline ModifiedOptions get_modified_options(const ReportTypeOptions& report_type_options)
{
    ModifiedOptions modified_options;
    for (auto& subject_options : report_type_options.m_subject_options)
    {
        get_modified_options(subject_options, modified_options);    
    }
    return modified_options;
}

struct StreamTypeOptions
{
    StreamTypeOptions() = default; //required by std::promise
    StreamTypeOptions(StreamTypeUUID stream_type_uuid):
        m_stream_type_uuid(stream_type_uuid)
    {
    }

    StreamTypeUUID m_stream_type_uuid;
    std::vector<SubjectStreamOptions> m_subject_options;
};

inline ModifiedOptions get_modified_options(const StreamTypeOptions& stream_type_options)
{
    ModifiedOptions modified_options;
    for (auto& subject_options : stream_type_options.m_subject_options)
    {
        get_modified_options(subject_options, modified_options);
    }
    return modified_options;
}

struct ActionTypeOptions
{
    ActionTypeOptions() = default; //required by std::promise
    ActionTypeOptions(ActionUUID action_uuid,   boost::optional<UUID> shortcut_uuid):
        m_action_uuid(action_uuid),
        m_shortcut_uuid(shortcut_uuid)
    {
    }

    ActionUUID m_action_uuid;
    boost::optional<UUID> m_shortcut_uuid;
    std::vector<SubjectActionOptions> m_subject_options;
};

inline ModifiedOptions get_modified_options(const ActionTypeOptions& action_options)
{
    ModifiedOptions modified_options;
    for (auto& subject_options : action_options.m_subject_options)
    {
        get_modified_options(subject_options, modified_options);
    }
    return modified_options;
}

struct SubjectOptions
{
    SubjectOptions() = default; //required by std::promise
    SubjectOptions(SubjectKey subject_key):
        m_subject_key(subject_key)
    {
    }

    SubjectKey m_subject_key;
    std::vector<SubjectReportOptions> m_report_options;
    std::vector<SubjectStreamOptions> m_stream_options;
    std::vector<SubjectActionOptions> m_action_options;
};

inline ModifiedOptions get_modified_options(const SubjectOptions& subject_options)
{
    ModifiedOptions modified_options;
    for (auto& report_options : subject_options.m_report_options)
    {
        get_modified_options(report_options, modified_options);
    }

    for (auto& stream_options : subject_options.m_stream_options)
    {
        get_modified_options(stream_options, modified_options);
    }

    for (auto& action_options : subject_options.m_action_options)
    {
        get_modified_options(action_options, modified_options);
    }

    return modified_options;
}

struct CustomQueryInfo
{
    CustomQueryInfo() = default;
    CustomQueryInfo(Key key, std::wstring name, std::vector<ReportTypeUUID> input_uuids, ReportTypeUUID output_uuid):
        m_key(key),
        m_name(std::move(name)),
        m_input_uuids(std::move(input_uuids)),
        m_output_uuid(output_uuid)
    {
    }

    Key m_key;
    std::wstring m_name;
    std::vector<ReportTypeUUID> m_input_uuids;
    ReportTypeUUID m_output_uuid;
};

struct CustomQueryDefinition
{
    CustomQueryDefinition() = default; //required by std::promise
    CustomQueryDefinition(std::vector<ReportTypeUUID> input_uuids, std::unique_ptr<XmlQueryDoc> query_doc, std::shared_ptr<const XmlDefDoc> query_def):
        m_input_uuids(std::move(input_uuids)),
        m_query_doc(std::move(query_doc)),
        m_query_def(query_def)
    {
    }   

    CustomQueryDefinition clone() const
    {
        return{m_input_uuids, clone_doc(*m_query_doc), m_query_def};
    }

    std::unique_ptr<RegularContent> apply_to(std::shared_ptr<RegularContent> input)
    {
        auto output_doc = do_query(*m_query_doc, input->as_regular_doc());
        if (!output_doc)
        {
            return nullptr;
        }

        auto output_def = get_output_def(*m_query_doc);
        bind_definition(*output_doc, *output_def);
        return std::make_unique<TR::Core::GeneralRegularContent>(std::move(output_doc), std::move(output_def));
    }

    std::vector<ReportTypeUUID> m_input_uuids;
    std::shared_ptr<XmlQueryDoc> m_query_doc;
    std::shared_ptr<const XmlDefDoc> m_query_def;
};

struct CustomFilterInfo
{
    CustomFilterInfo() = default;
    CustomFilterInfo(Key key, std::wstring name, ReportTypeUUID input_uuid, ReportTypeUUID output_uuid):
        m_key(key),
        m_name(std::move(name)),
        m_input_uuid(input_uuid),
        m_output_uuid(output_uuid)
    {
    }

    Key m_key;
    std::wstring m_name;
    ReportTypeUUID m_input_uuid;
    ReportTypeUUID m_output_uuid;
};

struct CustomFilterSetup
{
    CustomFilterSetup() = default; //required by std::promise
    CustomFilterSetup(std::vector<XmlNodeFilter> xml_node_filters, std::shared_ptr<const XmlDefDoc> filter_def):
        m_xml_node_filters(std::move(xml_node_filters)),
        m_filter_def(filter_def)
    {
    }

    CustomFilterSetup(const CustomFilterSetup& custom_filter_setup) = default;

    // because VC++2013 does not generate move constructor
    CustomFilterSetup(CustomFilterSetup&& custom_filter_setup):
        m_xml_node_filters(std::move(custom_filter_setup.m_xml_node_filters)),
        m_filter_def(custom_filter_setup.m_filter_def)
    {
    }

    CustomFilterSetup& operator = (const CustomFilterSetup& custom_filter_setup) = default;

    // because VC++2013 does not generate move assign operator
    CustomFilterSetup& operator = (CustomFilterSetup&& custom_filter_setup)
    {
        m_xml_node_filters = std::move(custom_filter_setup.m_xml_node_filters);
        m_filter_def = custom_filter_setup.m_filter_def;
        return *this;
    }

    std::unique_ptr<RegularContent> apply_to(std::shared_ptr<RegularContent> input)
    {
        auto filter_doc = XML::create_query(*m_filter_def);
        for (auto& xml_node_filter : m_xml_node_filters)
        {
            add_node_filter(*filter_doc, xml_node_filter);
        }

        auto output_doc = do_query(*filter_doc, input->as_regular_doc());

        if (!output_doc)
        {
            return nullptr;
        }
        return std::make_unique<TR::Core::GeneralRegularContent>(std::move(output_doc), m_filter_def);
    }

    std::vector<XmlNodeFilter> m_xml_node_filters;
    std::shared_ptr<const XmlDefDoc> m_filter_def;
};

struct TrackingStreamInfo
{
    TrackingStreamInfo() = default;
    TrackingStreamInfo(Key key, std::wstring name, ReportTypeUUID input_uuid, StreamTypeUUID output_uuid):
        m_key(key),
        m_name(std::move(name)),
        m_input_uuid(input_uuid),
        m_output_uuid(output_uuid)
    {
    }

    Key m_key;
    std::wstring m_name;
    ReportTypeUUID m_input_uuid;
    StreamTypeUUID m_output_uuid;
};

struct StreamFilterInfo
{
    StreamFilterInfo() = default;
    StreamFilterInfo(Key key, std::wstring name, StreamTypeUUID input_uuid, StreamTypeUUID output_uuid):
        m_key(key),
        m_name(std::move(name)),
        m_input_uuid(input_uuid),
        m_output_uuid(output_uuid)
    {
    }

    Key m_key;
    std::wstring m_name;
    StreamTypeUUID m_input_uuid;
    StreamTypeUUID m_output_uuid;
};

struct StreamFilterSetup
{
    StreamFilterSetup() = default; //required by std::promise
    StreamFilterSetup(std::shared_ptr<XmlQueryDoc> filter_doc, std::shared_ptr<const XmlDefDoc> filter_def):
        m_filter_doc(std::move(filter_doc)),
        m_filter_def(filter_def)
    {
    }

    std::vector<TR::StreamMessage> apply_to(const std::vector<TR::StreamMessage>& istream) const
    {
        std::vector<TR::StreamMessage> ostream;
        for (auto& input_message : istream)
        {
            if (auto output = do_query(*m_filter_doc, input_message.m_body->as_regular_doc()))
            {
                ostream.push_back({
                    input_message.m_severety,
                    input_message.m_event,
                    input_message.m_time,
                    std::make_unique<TR::Core::GeneralRegularContent>(std::move(output), input_message.m_body->get_def_doc())
                });
            }
        }
        return ostream;

    }

    std::shared_ptr<XmlQueryDoc> m_filter_doc;
    std::shared_ptr<const XmlDefDoc> m_filter_def;
};

struct ValidationInfo
{
    ValidationInfo() = default;
    ValidationInfo(ValidationKey key, std::wstring name, ReportTypeUUID base_report_type_uuid, ReportTypeUUID output_uuid):
        m_key(key),
        m_name(std::move(name)),
        m_base_report_type_uuid(base_report_type_uuid),
        m_output_uuid(output_uuid)
    {
    }

    ValidationKey m_key;
    std::wstring m_name;
    ReportTypeUUID m_base_report_type_uuid;
    ReportTypeUUID m_output_uuid;
};

struct ValidationSettings
{
    ValidationSettings() = default; //required by std::promise
    ValidationSettings(std::unique_ptr<XmlQueryDoc> validator, std::shared_ptr<const XmlQueryDoc> base_validator, std::shared_ptr<const XmlDefDoc> validator_def):
        m_validator(std::move(validator)),
        m_base_validator(base_validator),
        m_validator_def(validator_def)
    {
    }

    // because VC++2013 does not generate move constructor
    ValidationSettings(ValidationSettings&& validation_settings):
        m_validator(std::move(validation_settings.m_validator)),
        m_base_validator(validation_settings.m_base_validator),
        m_validator_def(validation_settings.m_validator_def)
    {
    }

    // because VC++2013 does not generate move assign operator
    ValidationSettings& operator = (ValidationSettings&& validation_settings)
    {
        m_validator = std::move(validation_settings.m_validator);
        m_base_validator = validation_settings.m_base_validator;
        m_validator_def = validation_settings.m_validator_def;
        return *this;
    }

    ValidationSettings clone() const
    {
        return{clone_doc(*m_validator), m_base_validator, m_validator_def};
    }

    std::unique_ptr<ValidatedContent> apply_to(std::shared_ptr<RegularContent> content)
    {
        std::unique_ptr<XmlRegularDoc> output_doc;
        if (m_base_validator)
        {
            output_doc = do_query(*XML::join_queries(*m_base_validator, *m_validator), content->as_regular_doc());
        }
        else
        {
            output_doc = do_query(*m_validator, content->as_regular_doc());
        }

        return std::make_unique<Core::ValidatedContent>(std::move(output_doc), content->get_def_doc());
    }

    std::unique_ptr<XmlQueryDoc> m_validator;
    std::shared_ptr<const XmlQueryDoc> m_base_validator;
    std::shared_ptr<const XmlDefDoc> m_validator_def;
};

struct ComplianceInfo
{
    ComplianceInfo() = default;
    ComplianceInfo(ComplianceKey key, std::wstring name, ReportTypeUUID base_report_type_uuid, ReportTypeUUID output_uuid):
        m_key(key),
        m_name(std::move(name)),
        m_base_report_type_uuid(base_report_type_uuid),
        m_output_uuid(output_uuid)
    {
    }

    ComplianceKey m_key;
    std::wstring m_name;
    ReportTypeUUID m_base_report_type_uuid;
    ReportTypeUUID m_output_uuid;
};

struct ComplianceSetup
{
    ComplianceSetup() = default; //required by std::promise
    ComplianceSetup(std::unique_ptr<XmlComplianceSetup> setup,
        std::shared_ptr<const XmlComplianceSetup> base_setup, std::shared_ptr<const XmlComplianceDef> compliance_def):
        m_setup(std::move(setup)),
        m_base_setup(base_setup),
        m_compliance_def(compliance_def)
    {
    }

    // because VC++2013 does not generate move constructor
    ComplianceSetup(ComplianceSetup&& compliance_setup):
        m_setup(std::move(compliance_setup.m_setup)),
        m_base_setup(compliance_setup.m_base_setup),
        m_compliance_def(compliance_setup.m_compliance_def)
    {
    }

    // because VC++2013 does not generate move assign operator
    ComplianceSetup& operator = (ComplianceSetup&& compliance_setup)
    {
        m_setup = std::move(compliance_setup.m_setup);
        m_base_setup = compliance_setup.m_base_setup;
        m_compliance_def = compliance_setup.m_compliance_def;
        return *this;
    }

    ComplianceSetup clone()
    {
        return{clone_doc(*m_setup), clone_doc(*m_base_setup), m_compliance_def};
    }

    std::unique_ptr<ComplianceContent> apply_to(std::shared_ptr<CheckListContent> check_list_content)
    {
        auto setup = stl_tools::static_pointer_cast<XmlComplianceSetup>(XML::join_queries(*m_base_setup, *m_setup));
        auto output_doc = stl_tools::static_pointer_cast<XmlComplianceDoc>(do_query(*setup, check_list_content->get_compliance_doc()));
        return std::make_unique<ComplianceContent>(std::move(output_doc), check_list_content->get_compliance_def());
    }

    std::unique_ptr<XmlComplianceSetup> m_setup;
    std::shared_ptr<const XmlComplianceSetup> m_base_setup;
    std::shared_ptr<const XmlComplianceDef> m_compliance_def;
};

struct GroupingInfo
{
    GroupingInfo() = default;
    GroupingInfo(Key key, std::wstring name, ReportTypeUUID input_uuid, ReportTypeUUID output_uuid):
        m_key(key),
        m_name(std::move(name)),
        m_input_uuid(input_uuid),
        m_output_uuid(output_uuid)
    {
    }

    ComplianceKey m_key;
    std::wstring m_name;
    ReportTypeUUID m_input_uuid;
    ReportTypeUUID m_output_uuid;
};

struct FeatureInfo
{
    FeatureInfo() = default; //required by std::promise
    FeatureInfo(UUID uuid, std::wstring name, std::vector<UUID> dependencies, bool installed):
        m_uuid(uuid),
        m_name(std::move(name)),
        m_dependencies(std::move(dependencies)),
        m_installed(installed)
    {
    }

    UUID m_uuid;
    std::vector<UUID> m_dependencies;
    std::wstring m_name;
    bool m_installed;
};

struct UserSettings
{
    UserSettings() = default; //required by std::promise
    UserSettings(std::wstring name, std::wstring email):
        m_name(std::move(name)),
        m_email(std::move(email))
    {
    }

    std::wstring m_name;
    std::wstring m_email;
};

struct UserInfo: UserSettings
{
    UserInfo() = default; //required by std::promise
    UserInfo(Key key, std::wstring name, std::wstring email):
        UserSettings(std::move(name), std::move(email)),
        m_key(key)
    {
    }

    Key m_key;
};

struct AccessEntry
{
    AccessEntry() = default; //required by std::promise
    AccessEntry(Key user_key, SourceKey source_key, AccessRights access_rights):
        m_user_key(user_key),
        m_source_key(source_key),
        m_access_rights(access_rights)
    {
    }

    Key m_user_key;
    SourceKey m_source_key;
    AccessRights m_access_rights;
};

struct SourceAccessList
{
    SourceAccessList() = default; //required by std::promise
    SourceAccessList(SourceKey source_key, std::vector<AccessEntry> access_entries):
        m_source_key(source_key),
        m_access_entries(std::move(access_entries))
    {
    }

    SourceKey m_source_key;
    std::vector<AccessEntry> m_access_entries;
};

struct SourceAccessInfo: SourceAccessList
{
    SourceAccessInfo() = default; //required by std::promise
    SourceAccessInfo(SourceKey source_key, std::vector<AccessEntry> access_entries, std::vector<AccessEntry> default_access_entries):
        SourceAccessList(source_key, std::move(access_entries)),
        m_default_access_entries(std::move(default_access_entries))
    {
    }

    std::vector<AccessEntry> m_default_access_entries;
};

struct SummaryInfo
{
    SummaryInfo() = default; //required by std::promise
    SummaryInfo(int sources_count, int warning_sources, int failed_sources):
        m_sources_count(sources_count),     
        m_warning_sources(warning_sources),
        m_failed_sources(failed_sources)
    {
    }

    int m_sources_count;    
    int m_warning_sources;
    int m_failed_sources;
};

} //namespace TR {