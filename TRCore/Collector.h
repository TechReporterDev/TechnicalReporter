#pragma once
#include "RegistryDecl.h"
namespace TR { namespace Core {
class CurrentReportDataset;
struct CurrentReportData;
struct CurrentReportContentData;
class Content;

class Report
{
public:
    Report(SourceRef source_ref, ReportTypeRef report_type_ref, stl_tools::lazy_shared_ptr<Content> content, time_t time);

    SourceRef                       get_source_ref() const;
    ReportTypeRef                   get_report_type_ref() const;
    std::shared_ptr<Content>        get_content() const;
    time_t                          get_time() const;

protected:
    SourceRef m_source_ref;
    ReportTypeRef m_report_type_ref;    
    stl_tools::lazy_shared_ptr<Content> m_content;
    time_t m_time;
};

struct TransformationCircumstances
{
    TransformationCircumstances() :
        m_transformation_uuid(stl_tools::null_uuid()),
        m_setup_uuid(stl_tools::null_uuid())
    {
    }

    TransformationCircumstances(UUID transformation_uuid, UUID setup_uuid, std::vector<UUID> input_uuids) :
        m_transformation_uuid(std::move(transformation_uuid)),
        m_setup_uuid(std::move(setup_uuid)),
        m_input_uuids(std::move(input_uuids))
    {
    }

    UUID					m_transformation_uuid;
    UUID					m_setup_uuid;
    std::vector<UUID>		m_input_uuids;
};

inline bool operator == (const TransformationCircumstances& left, const TransformationCircumstances& right)
{
    return left.m_transformation_uuid == right.m_transformation_uuid &&
        left.m_setup_uuid == right.m_setup_uuid &&
        left.m_input_uuids == right.m_input_uuids;
}

class CurrentReport: public Report
{
public: 
    friend class Collector;

    std::shared_ptr<Diff>                   get_diff() const;
    time_t                                  get_check_time() const;
    UUID                                    get_uuid() const;
    const TransformationCircumstances&      get_transformation_circumstances() const;
    CurrentReportRef                        get_ref() const;
    operator CurrentReportRef() const;

private:
    CurrentReport(CurrentReportRef ref, stl_tools::lazy_shared_ptr<Content> content, std::shared_ptr<Diff> diff, time_t time, time_t check_time, UUID uuid, TransformationCircumstances transformation_circumstances = {});

    CurrentReportRef m_ref;
    std::shared_ptr<Diff> m_diff;
    time_t m_check_time;
    UUID m_uuid;
    TransformationCircumstances m_transformation_circumstances;
};

class Collector
{
public:
    static const Priority COLLECTOR_PRIORITY = 500;

    Collector();
    ~Collector();

    void                            set_database(Database* db);
    void                            set_report_types(ReportTypes* report_types);
    void                            set_source_types(SourceTypes* source_types);
    void                            set_sources(Sources* sources);
    void                            run(Executive* executive, Transaction& t);

    void                            collect_report(const Report& report, const TransformationCircumstances& transformation_circumstances = {});
    CurrentReport                   get_current_report(SourceRef source_ref, ReportTypeRef report_type_ref) const;
    std::vector<CurrentReport>      get_current_reports(SourceRef source_ref) const;
    boost::optional<CurrentReport>  find_current_report(SourceRef source_ref, ReportTypeRef report_type_ref) const;
    boost::optional<CurrentReport>  find_current_report(UUID uuid) const;

    template<class T>
    void connect_update_current_report(T slot, Priority priority) { m_update_current_report_sig.connect(slot, priority); }

private:
    friend class CollectingJob;
    class CollectingManager;

    void                            persist_current_report(SourceRef source_ref, ReportTypeRef report_type_ref, std::shared_ptr<Content> content, std::shared_ptr<Diff> diff, time_t time, time_t check_time, const TransformationCircumstances& transformation_circumstances, Transaction& t);
    void                            update_current_report(SourceRef source_ref, ReportTypeRef report_type_ref, std::shared_ptr<Content> content, std::shared_ptr<Diff> diff, time_t time, time_t check_time, const TransformationCircumstances& transformation_circumstances, Transaction& t);
    void                            update_current_report(SourceRef source_ref, ReportTypeRef report_type_ref, time_t check_time, const TransformationCircumstances& transformation_circumstances, Transaction& t);

    CurrentReportData               pack_current_report(const CurrentReport& current_report) const;
    CurrentReportContentData        pack_current_report_content(const CurrentReport& current_report) const;
    CurrentReport                   unpack_current_report(const CurrentReportData& current_report_data) const;
    std::shared_ptr<Content>        unpack_current_report_content(const CurrentReportContentData& current_report_content_data) const;

    void                            on_remove_report_type(UUID source_type_uuid, Transaction& t);
    void                            on_remove_sources(const std::vector<SourceKey>& removed_sources, Transaction& t);
    
    Database* m_db;
    ReportTypes* m_report_types;
    SourceTypes* m_source_types;
    Sources* m_sources;
    Executive* m_executive;

    std::unique_ptr<CurrentReportDataset> m_current_report_dataset;
    std::unique_ptr<CollectingManager> m_collecting_manager;

    OrderedSignal<void(CurrentReportRef current_report_ref, bool content_changed, Transaction& t)> m_update_current_report_sig; 
};

}} //namespace TR { namespace Core {
