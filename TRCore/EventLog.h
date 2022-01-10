#pragma once
#include "AppDecl.h"
namespace TR { namespace Core {
struct EventRecordData;
enum class EventSeverity { NONE = 0, INFO, WARN, FAIL };
enum class EventType { REPORT_CHANGED = 0, INTEGRITY_CHANGED, COMPLIANCE_CHANGED, VALIDATION_CHANGED, STREAM_MESSAGE_RECIEVED, DOWNLOAD_FAILED };

struct EventRecord
{   
    EventRecord(SourceRef source_ref, ResourceRef resource_ref, EventType event_type, EventSeverity severity, time_t time):
        m_source_ref(source_ref),
        m_resource_ref(resource_ref),
        m_event_type(event_type),
        m_severity(severity),
        m_time(time)
    {
    }
        
    SourceRef       m_source_ref;
    ResourceRef     m_resource_ref;
    EventType       m_event_type;
    EventSeverity   m_severity;
    time_t          m_time;
};

class EventLog
{
public:
    static const Priority EVENT_LOG_PRIORITY = 2500;

    EventLog(Database& db, CoreDomain& core_domain);
    ~EventLog();

    void                            set_security(Security* security);
    std::vector<EventRecord>        get_event_log(Key user_key, EventSeverity min_severity = EventSeverity::NONE);

private:
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
    Collector& m_collector;
    Reservoir& m_reservoir;
    IntegrityChecking& m_integrity_checking;
    Compliances& m_compliances;
    Validations& m_validations;
    Security* m_security;
};

}} //namespace TR { namespace Core {