#pragma once
#include "XmlNodeFilter.h"
#include "ServicesDecl.h"
namespace TR { namespace Core {
struct CustomFilterData;

class CustomFilter
{
public:
    friend class CustomFilters;

    CustomFilter(std::wstring name, ReportTypeRef input_ref);

    Key                                 get_key() const;
    std::wstring                        get_name() const;
    ReportTypeRef                       get_input_ref() const;
    ReportTypeRef                       get_output_ref() const;

    std::vector<XmlNodeFilter>          get_node_filters() const;
    void                                set_node_filters(std::vector<XmlNodeFilter> node_filters);
    
private:
    CustomFilter(Key key, std::wstring name, ReportTypeRef input_ref, boost::optional<ReportTypeRef> output_ref, std::vector<XmlNodeFilter> node_filters);

    Key m_key;
    std::wstring m_name;
    ReportTypeRef m_input_ref;
    boost::optional<ReportTypeRef> m_output_ref;
    std::vector<XmlNodeFilter> m_node_filters;
};

class CustomFilters
{
public:
    static const Priority CUSTOM_FILTERS_PRIORITY = 1850;

    CustomFilters(Database& db, Basis& basis, Registry& registry);
    void                                run(Executive* executive, Transaction& t);

    void                                add_custom_filter(CustomFilter& custom_filter, Transaction& t, boost::optional<ReportTypeUUID> output_uuid = boost::none);
    void                                update_custom_filter(const CustomFilter& custom_filter, Transaction& t) const;
    CustomFilter                        get_custom_filter(Key custom_filter_key) const;
    void                                remove_custom_filter(Key custom_filter_key, Transaction& t);
    
    std::vector<CustomFilter>           find_by_input(ReportTypeRef input_ref) const;
    boost::optional<CustomFilter>       find_by_output(ReportTypeRef output_ref) const;
    void                                remove_custom_filter(ReportTypeRef output_ref, Transaction& t);

private:
    CustomFilterData                    pack_custom_filter(const CustomFilter& custom_filter) const;
    CustomFilter                        unpack_custom_filter(const CustomFilterData& custom_filter_data) const;

    void                                install_custom_filter(const CustomFilter& custom_filter, Transaction& t);
    void                                uninstall_custom_filter(const CustomFilter& custom_filter, Transaction& t);

    Database& m_db;
    Sources& m_sources;
    ReportTypes& m_report_types;
    Selections& m_selections;
    ResourcePolicies& m_resource_policies;
    bool m_running;
};

}} // namespace TR { namespace Core {