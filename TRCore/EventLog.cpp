#include "stdafx.h"
#include "EventLog.h"
#include "CoreDomain.h"
#include "Security.h"
#include "JobManager.h"
#include "Transaction.h"
#include "ServicesPacking.h"
#include "EventRecord-odb.hxx"
namespace TR { namespace Core {

EventLog::EventLog(Database& db, CoreDomain& core_domain):
    m_db(db),
    m_core_domain(core_domain),
    m_report_types(core_domain.m_basis.m_report_types),
    m_stream_types(core_domain.m_basis.m_stream_types),
    m_sources(core_domain.m_registry.m_sources),
    m_roles(core_domain.m_registry.m_roles),
    m_collector(core_domain.m_registry.m_collector),
    m_reservoir(core_domain.m_registry.m_reservoir),
    m_integrity_checking(core_domain.m_services.m_integrity_checking),
    m_compliances(core_domain.m_services.m_compliances),
    m_validations(core_domain.m_services.m_validations),
    m_security(nullptr)
{
    m_report_types.connect_remove_report_type([this](ReportTypeUUID report_type_uuid, Transaction& t){
        on_remove_report_type(report_type_uuid, t);
    }, EVENT_LOG_PRIORITY);

    m_stream_types.connect_remove_stream_type([this](StreamTypeUUID stream_type_uuid, Transaction& t){
        on_remove_stream_type(stream_type_uuid, t);
    }, EVENT_LOG_PRIORITY);

    m_sources.connect_remove_source([this](SourceKey source_key, const std::vector<SourceKey>& removed_sources, Transaction& t){
        on_remove_source(removed_sources, t);
    }, EVENT_LOG_PRIORITY);

    m_integrity_checking.connect_update_checking_result([this](SourceRef source_ref, ReportTypeRef report_type_ref, IntegrityCheckingResult checking_result, Transaction& t){
        if (checking_result == IntegrityCheckingResult::MISMATCHED)
        {
            on_integrity_checking_failed(source_ref, report_type_ref, t);
        }
    }, EVENT_LOG_PRIORITY);

    m_compliances.connect_update_checking_result([this](ComplianceRef compliance_ref, SourceRef source_ref, ComplianceCheckingResult checking_result, Transaction& t){
        if (checking_result == ComplianceCheckingResult::CHECKING_FAILED)
        {           
            on_compliance_checking_failed(source_ref, (*compliance_ref).get_output_ref(), t);
        }
    }, EVENT_LOG_PRIORITY);

    m_validations.connect_update_validation_result([this](ValidationRef validation_ref, SourceRef source_ref, ValidationResult validation_result, Transaction& t){
        if (validation_result == ValidationResult::VALIDATION_FAILED)
        {           
            on_validation_failed(source_ref, (*validation_ref).get_output_ref(), t);
        }
    }, EVENT_LOG_PRIORITY);

    m_reservoir.connect_append_record([this](const StreamRecord& stream_record, Transaction& t){
        on_stream_record_recieved(stream_record, t);
    }, EVENT_LOG_PRIORITY); 
}

EventLog::~EventLog()
{
}

void EventLog::set_security(Security* security)
{
    _ASSERT(!m_security);
    m_security = security;
}

std::vector<EventRecord> EventLog::get_event_log(Key user_key, EventSeverity min_severity)
{
    ReadOnlyTransaction t(m_db);

    std::vector<EventRecord> event_records;
    for (auto& event_record_data : m_db.query<EventRecordData>(odb::query<EventRecordData>::severity > min_severity))
    {
        event_records.push_back({
            unpack(event_record_data.m_source_key, &m_sources),
            unpack(event_record_data.m_resource_uuid, &m_core_domain.m_basis.m_report_types, &m_core_domain.m_basis.m_stream_types),
            event_record_data.m_event_type,
            event_record_data.m_severity,
            event_record_data.m_time
        });
    }
    return event_records;
}

/*void Notifier::on_remove_report_type(ReportTypeUUID report_type_uuid, Transaction& t)
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
}*/

}} //namespace TR { namespace Core {