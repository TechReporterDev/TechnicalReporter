#pragma once
#include "AppDecl.h"
namespace TR {  namespace Core {
enum class NotificationSeverity {DEFAULT = 0, INFO, WARN, FAIL, NONE};
enum class NotificationCause {INTEGRITY_CHECKING_FAILED = 0, COMPLIANCE_CHECKING_FAILED, VALIDATION_FAILED, STREAM_MESSAGE_RECIEVED};

class NotificatonSender
{
public:
    virtual ~NotificatonSender() = default;

    virtual void send_integrity_checking_failed(Key user_key, SourceRef source_ref, ReportTypeRef report_type_ref) = 0;
    virtual void send_compliance_checking_failed(Key user_key, SourceRef source_ref, ReportTypeRef report_type_ref) = 0;
    virtual void send_validation_failed(Key user_key, SourceRef source_ref, ReportTypeRef report_type_ref) = 0;
    virtual void send_stream_record_recieved(Key user_key, const StreamRecord& stream_record) = 0;
};

class Notifier
{
public:
    static const Priority NOTIFIER_PRIORITY = 2000;

    Notifier(Database& db, CoreDomain& core_domain);
    ~Notifier();

    void                            set_security(Security* security);
    void                            add_sender(std::unique_ptr<NotificatonSender> notification_sender);

    NotificationSeverity            get_severity(Key user_key, NotificationCause cause, SubjectRef subject_ref, ResourceRef resource_ref);
    NotificationSeverity            get_actual_severity(Key user_key, NotificationCause cause, SubjectRef subject_ref, ResourceRef resource_ref);
    void                            set_severity(Key user_key, NotificationCause cause, SubjectRef subject_ref, ResourceRef resource_ref, NotificationSeverity severity, Transaction& t);

private:
    std::vector<User>               get_recipients(NotificationCause cause, SourceRef source_ref, ResourceRef resource_ref, NotificationSeverity severity);

    void                            on_remove_report_type(ReportTypeUUID report_type_uuid, Transaction& t);
    void                            on_remove_stream_type(StreamTypeUUID stream_type_uuid, Transaction& t);
    void                            on_remove_source(const std::vector<SourceKey>& source_keys, Transaction& t);
    void                            on_remove_role(RoleKey role_key, Transaction& t);
    void                            on_remove_user(Key user_key, Transaction& t);
    
    void                            on_integrity_checking_failed(SourceRef source_ref, ReportTypeRef report_type_ref, Transaction& t);
    void                            on_compliance_checking_failed(SourceRef source_ref, ReportTypeRef report_type_ref, Transaction& t);
    void                            on_validation_failed(SourceRef source_ref, ReportTypeRef report_type_ref, Transaction& t);
    void                            on_stream_record_recieved(const StreamRecord& stream_record, Transaction& t);
    
    CoreDomain& m_core_domain;
    Database& m_db; 
    ReportTypes& m_report_types;
    StreamTypes& m_stream_types;    
    Sources& m_sources;
    Roles& m_roles;
    Reservoir& m_reservoir;
    IntegrityChecking& m_integrity_checking;
    Compliances& m_compliances;
    Validations& m_validations;
    Security* m_security;

    std::vector<std::unique_ptr<NotificatonSender>> m_notification_senders;
};

}} //namespace TR { namespace Core {