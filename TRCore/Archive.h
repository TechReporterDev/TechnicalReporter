#pragma once
#include "Collector.h"
#include "ServicesDecl.h"
namespace TR {namespace Core {
class ArchivingDataset;
struct ArchivedReportData;
struct ArchivedReportContentData;

class ArchivedReport: public Report
{
public:
    friend class Archive;

    std::shared_ptr<Diff>   get_diff() const;
    ArchivedReportRef       get_ref() const;
    operator ArchivedReportRef() const;

private:
    ArchivedReport(boost::optional<ArchivedReportRef> ref, SourceRef source_ref, ReportTypeRef report_type_ref, stl_tools::lazy_shared_ptr<Content> content, std::shared_ptr<Diff> diff, time_t time, time_t off_time);

    boost::optional<ArchivedReportRef> m_ref;
    std::shared_ptr<Diff> m_diff;
    time_t m_off_time;
};

enum class ArchivingPolicy { DEFAULT_ARCHIVING = 0, DISABLE_ARCHIVING, ENABLE_ARCHIVING };
enum class ArchivedExpiration { EXPIRED_DEFAULT = 0, EXPIRED_NEVER, EXPIRED_IN_HOUR, EXPIRED_IN_DAY, EXPIRED_IN_MONTH, EXPIRED_IN_YEAR };
enum class ArchivingFlags { FLAGS_NONE = 0x00, NO_DIFF = 0x01 };

class Archive
{
public:
    static const Priority ARCHIVE_PRIORITY = 1500;

    Archive(Database& db, Basis& basis, Registry& registry);
    ~Archive();
    void                            run(Executive* executive, Transaction& t);

    // archiving policy
    void                            set_archiving_policy(SubjectRef subject_ref, ReportTypeRef report_type_ref, ArchivingPolicy archiving_policy, Transaction& t);
    ArchivingPolicy                 get_archiving_policy(SubjectRef subject_ref, ReportTypeRef report_type_ref) const;
    bool                            is_archiving_enabled(SubjectRef subject_ref, ReportTypeRef report_type_ref) const;

    // expiration policy
    void                            set_archived_expiration(SubjectRef subject_ref, ReportTypeRef report_type_ref, ArchivedExpiration archived_expiration, Transaction& t);
    ArchivedExpiration              get_archived_expiration(SubjectRef subject_ref, ReportTypeRef report_type_ref) const;
    ArchivedExpiration              get_effective_archived_expiration(SubjectRef subject_ref, ReportTypeRef report_type_ref) const;

    // archive access
    ArchivedReport                  get_archived_report(ArchivedReportKey archived_report_key) const;   
    std::vector<ArchivedReport>     get_archived_reports(SourceRef source_ref) const;
    boost::optional<ArchivedReport> last_archived_report(SourceRef source_ref, ReportTypeRef report_type_ref) const;
    boost::optional<ArchivedReport> find_archived_report(SourceRef source_ref, ReportTypeRef report_type_ref, time_t time) const;

    template<class T>
    void connect_add_archive_report(T slot, Priority priority) { m_add_archive_report_sig.connect(slot, priority); }

private:
    void                            forward_archiving_policy(SubjectRef subject_ref, ReportTypeRef report_type_ref, Transaction& t);
    
    ArchivedReportData              pack_archived_report(const ArchivedReport& archved_report) const;
    ArchivedReport                  unpack_archived_report(const ArchivedReportData& archived_report_data) const;
    
    bool                            check_archivate_condition(const CurrentReport& current_report) const;
    void                            archivate_current_report(const CurrentReport& current_report, Transaction& t);

    void                            archivate_one_by_one();
    void                            archivate_next(std::vector<std::pair<SourceKey, ReportTypeUUID>> targets);
    void                            archivate_one(SourceKey source_key, ReportTypeUUID report_type_uuid);

    void                            remove_expired_one_by_one();
    void                            remove_next_expired(std::vector<std::pair<SourceKey, ReportTypeUUID>> targets);
    void                            remove_expired(SourceKey source_key, ReportTypeUUID report_type_uuid);

    void                            on_update_current_report(CurrentReportRef current_report_ref, Transaction& t);
    void                            on_remove_report_type(UUID report_type_uuid, Transaction& t);
    void                            on_remove_sources(const std::vector<SourceKey>& removed_sources, Transaction& t);

    Database& m_db;
    Basis& m_basis;
    Registry& m_registry;
    ReportTypes& m_report_types;
    SourceTypes& m_source_types;
    Sources& m_sources;
    Collector& m_collector;
    SourceResources& m_source_resources;
    std::unique_ptr<ArchivingDataset> m_archiving_dataset;
    Executive* m_executive;

    OrderedSignal<void(ArchivedReportRef archived_report_ref, Transaction& t)> m_add_archived_report_sig;
};

}} //namespace TR { namespace Core {