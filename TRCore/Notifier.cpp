#include "stdafx.h"
#include "Notifier.h"
#include "CoreDomain.h"
#include "Security.h"
#include "JobManager.h"
#include "Transaction.h"
#include "ServicesPacking.h"
#include "NotificationData-odb.hxx"
namespace TR { namespace Core {

Notifier::Notifier(Database& db, CoreDomain& core_domain):
    m_db(db),
    m_core_domain(core_domain),
    m_report_types(core_domain.m_basis.m_report_types),
    m_stream_types(core_domain.m_basis.m_stream_types),
    m_sources(core_domain.m_registry.m_sources),
    m_roles(core_domain.m_registry.m_roles),
    m_reservoir(core_domain.m_registry.m_reservoir),
    m_integrity_checking(core_domain.m_services.m_integrity_checking),
    m_compliances(core_domain.m_services.m_compliances),
    m_validations(core_domain.m_services.m_validations),
    m_security(nullptr)
{
    m_report_types.connect_remove_report_type([this](ReportTypeUUID report_type_uuid, Transaction& t){
        on_remove_report_type(report_type_uuid, t);
    }, NOTIFIER_PRIORITY);

    m_stream_types.connect_remove_stream_type([this](StreamTypeUUID stream_type_uuid, Transaction& t){
        on_remove_stream_type(stream_type_uuid, t);
    }, NOTIFIER_PRIORITY);

    m_sources.connect_remove_source([this](SourceKey source_key, const std::vector<SourceKey>& removed_sources, Transaction& t){
        on_remove_source(removed_sources, t);
    }, NOTIFIER_PRIORITY);

    m_roles.connect_remove_role([this](RoleKey role_key, Transaction& t){
        on_remove_role(role_key, t);
    }, NOTIFIER_PRIORITY);

    m_integrity_checking.connect_update_checking_result([this](SourceRef source_ref, ReportTypeRef report_type_ref, IntegrityCheckingResult checking_result, Transaction& t){
        if (checking_result == IntegrityCheckingResult::MISMATCHED)
        {
            on_integrity_checking_failed(source_ref, report_type_ref, t);
        }
    }, NOTIFIER_PRIORITY);

    m_compliances.connect_update_checking_result([this](ComplianceRef compliance_ref, SourceRef source_ref, ComplianceCheckingResult checking_result, Transaction& t){
        if (checking_result == ComplianceCheckingResult::CHECKING_FAILED)
        {           
            on_compliance_checking_failed(source_ref, (*compliance_ref).get_output_ref(), t);
        }
    }, NOTIFIER_PRIORITY);

    m_validations.connect_update_validation_result([this](ValidationRef validation_ref, SourceRef source_ref, ValidationResult validation_result, Transaction& t){
        if (validation_result == ValidationResult::VALIDATION_FAILED)
        {           
            on_validation_failed(source_ref, (*validation_ref).get_output_ref(), t);
        }
    }, NOTIFIER_PRIORITY);

    m_reservoir.connect_append_record([this](const StreamRecord& stream_record, Transaction& t){
        on_stream_record_recieved(stream_record, t);
    }, NOTIFIER_PRIORITY);
 
}

Notifier::~Notifier()
{
}

void Notifier::set_security(Security* security)
{
    _ASSERT(!m_security);
    m_security = security;
    m_security->connect_remove_user([this](Key user_key, Transaction& t){
        on_remove_user(user_key, t);
    }, NOTIFIER_PRIORITY);
}

void Notifier::add_sender(std::unique_ptr<NotificatonSender> notification_sender)
{
    m_notification_senders.push_back(std::move(notification_sender));
}

NotificationSeverity Notifier::get_severity(Key user_key, NotificationCause cause, SubjectRef subject_ref, ResourceRef resource_ref)
{
    ReadOnlyTransaction t(m_db);
    NotificationData notification_data({user_key, cause, pack(subject_ref), pack(resource_ref)});
    m_db.find(notification_data.m_identity, notification_data);
    return notification_data.m_severity;
}

NotificationSeverity Notifier::get_actual_severity(Key user_key, NotificationCause cause, SubjectRef subject_ref, ResourceRef resource_ref)
{
    ReadOnlyTransaction t(m_db);
    auto severity = get_severity(user_key, cause, subject_ref, resource_ref);

    if (severity != NotificationSeverity::DEFAULT)
    {
        return severity;
    }

    if (refer_default_role(subject_ref))
    {
        return NotificationSeverity::NONE;
    }
    
    return get_actual_severity(user_key, cause, *get_base_ref(subject_ref), resource_ref);
}

void Notifier::set_severity(Key user_key, NotificationCause cause, SubjectRef subject_ref, ResourceRef resource_ref, NotificationSeverity severity, Transaction& t)
{
    NotificationData notification_data({user_key, cause, pack(subject_ref), pack(resource_ref)});
    auto exist = m_db.find(notification_data.m_identity, notification_data);
    notification_data.m_severity = severity;    
    exist? m_db.update(notification_data): m_db.persist(notification_data); 
}

std::vector<User> Notifier::get_recipients(NotificationCause cause, SourceRef source_ref, ResourceRef resource_ref, NotificationSeverity severity)
{
    std::vector<User> recipients;
    for (auto& user : m_security->get_users())
    {
        if (!m_security->check_access_rights(user.m_key, source_ref, Access::READ_ACCESS))
        {
            continue;
        }

        if (get_actual_severity(user.m_key, cause, source_ref, resource_ref) > severity)
        {
            continue;
        }

        recipients.push_back(user);
    }
    return recipients;
}

void Notifier::on_remove_report_type(ReportTypeUUID report_type_uuid, Transaction& t)
{
    m_db.erase_query<NotificationData>(odb::query<NotificationData>::identity.resource.value1.value == report_type_uuid);
}

void Notifier::on_remove_stream_type(StreamTypeUUID stream_type_uuid, Transaction& t)
{
    m_db.erase_query<NotificationData>(odb::query<NotificationData>::identity.resource.value2.value == stream_type_uuid);
}

void Notifier::on_remove_source(const std::vector<SourceKey>& source_keys, Transaction& t)
{
    m_db.erase_query<NotificationData>(odb::query<NotificationData>::identity.subject.value1.value.in_range(source_keys.begin(), source_keys.end()));
}

void Notifier::on_remove_role(RoleKey role_key, Transaction& t)
{
    m_db.erase_query<NotificationData>(odb::query<NotificationData>::identity.subject.value2.value == role_key);
}

void Notifier::on_remove_user(Key user_key, Transaction& t)
{
    m_db.erase_query<NotificationData>(odb::query<NotificationData>::identity.user_key == user_key);
}

void Notifier::on_integrity_checking_failed(SourceRef source_ref, ReportTypeRef report_type_ref, Transaction& t)
{
    t.connect_commit([this, source_ref, report_type_ref](){
        for (auto& recipient : get_recipients(NotificationCause::INTEGRITY_CHECKING_FAILED, source_ref, report_type_ref, NotificationSeverity::INFO))
        {
            for (auto& sender : m_notification_senders | boost::adaptors::indirected)
            {
                sender.send_integrity_checking_failed(recipient.m_key, source_ref, report_type_ref);
            }
        }
    }); 
}

void Notifier::on_compliance_checking_failed(SourceRef source_ref, ReportTypeRef report_type_ref, Transaction& t)
{
    t.connect_commit([this, source_ref, report_type_ref](){
        for (auto& recipient : get_recipients(NotificationCause::COMPLIANCE_CHECKING_FAILED, source_ref, report_type_ref, NotificationSeverity::INFO))
        {
            for (auto& sender : m_notification_senders | boost::adaptors::indirected)
            {
                sender.send_compliance_checking_failed(recipient.m_key, source_ref, report_type_ref);
            }
        }
    });
}

void Notifier::on_validation_failed(SourceRef source_ref, ReportTypeRef report_type_ref, Transaction& t)
{
    t.connect_commit([this, source_ref, report_type_ref](){
        for (auto& recipient : get_recipients(NotificationCause::VALIDATION_FAILED, source_ref, report_type_ref, NotificationSeverity::INFO))
        {
            for (auto& sender : m_notification_senders | boost::adaptors::indirected)
            {
                sender.send_validation_failed(recipient.m_key, source_ref, report_type_ref);
            }
        }
    });
}

void Notifier::on_stream_record_recieved(const StreamRecord& stream_record, Transaction& t)
{
    t.connect_commit([this, stream_record](){
        for (auto& recipient : get_recipients(NotificationCause::STREAM_MESSAGE_RECIEVED, stream_record.m_source_ref, stream_record.m_stream_type_ref, NotificationSeverity::INFO))
        {
            for (auto& sender : m_notification_senders | boost::adaptors::indirected)
            {
                sender.send_stream_record_recieved(recipient.m_key, stream_record);
            }
        }
    });
}

}} //namespace TR { namespace Core {