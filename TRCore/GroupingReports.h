#pragma once
#include "ServicesDecl.h"
#include "ReportTypes.h"
namespace TR { namespace Core {
struct GroupingReportData;

class GroupingReport
{
public:
    friend class GroupingReports;   
    
    std::wstring                        get_name() const;
    ReportTypeRef                       get_input_ref() const;
    ReportTypeRef                       get_output_ref() const;
    ReportTypeRef                       get_child_grouping_ref() const;
    boost::optional<SourceRef>          get_bound_source() const;

    GroupingReportRef                   get_ref() const;
    operator GroupingReportRef() const;

private:
    GroupingReport(std::wstring name, ReportTypeRef input_ref, ReportType::Traits output_traits, boost::optional<SourceRef> bound_source = boost::none);
    GroupingReport(std::wstring name, ReportTypeRef input_ref, ReportTypeRef output_ref, ReportType::Traits output_traits, ReportTypeRef child_grouping_ref, boost::optional<SourceRef> bound_source, GroupingReportRef ref);

    std::wstring m_name;
    ReportTypeRef m_input_ref;
    boost::optional<ReportTypeRef> m_output_ref;
    ReportType::Traits m_output_traits;
    boost::optional<ReportTypeRef> m_child_grouping_ref;
    boost::optional<SourceRef> m_bound_source;  
    boost::optional<GroupingReportRef> m_ref;
};

class GroupingReports
{
public:
    GroupingReports(Database& db, Basis& basis, Registry& registry);
    void restore(Transaction& t);

    GroupingReport                      create_grouping_report(std::wstring name, ReportTypeRef input_ref);
    void                                add_grouping_report(GroupingReport& grouping_report, ReportTypeUUID output_uuid, Transaction& t);
    GroupingReport                      get_grouping_report(Key grouping_report_key) const;
    boost::optional<GroupingReport>     find_by_output(ReportTypeRef output_ref) const;
    bool                                is_grouping_output(ReportTypeRef output_ref) const;
    void                                remove_grouping_report(Key grouping_report_key, Transaction& t);

private:
    GroupingReportData                  pack_grouping_report(const GroupingReport& grouping_report) const;
    GroupingReport                      unpack_grouping_report(const GroupingReportData& grouping_report_data) const;

    void                                install_grouping_report(const GroupingReport& grouping_report, Transaction& t);
    void                                uninstall_grouping_report(const GroupingReport& grouping_report, Transaction& t);

    Database& m_db;
    ReportTypes& m_report_types;
    Selections& m_selections;
    Groupings& m_groupings;
    Sources& m_sources;
    Roles& m_roles;
    ResourcePolicies& m_resource_policies;  
};

}} // namespace TR { namespace Core {