#pragma once
#include "ReportTypes.h"
#include "ServicesDecl.h"
namespace TR { namespace Core {
struct CustomQueryData;

class CustomQuery
{
public:
    friend class CustomQueries;

    std::wstring                        get_name() const;
    std::vector<ReportTypeRef>          get_input_refs() const;
    const XmlQueryDoc&                  get_query_doc() const;
    void                                set_query_doc(std::unique_ptr<XmlQueryDoc> query_doc);
    std::shared_ptr<const XmlDefDoc>    get_query_def() const;

    ReportTypeRef                       get_output_ref() const;
    CustomQueryRef                      get_ref() const;
    operator                            CustomQueryRef() const;

private:
    CustomQuery(std::wstring name, std::vector<ReportTypeRef> input_type_refs,
        std::unique_ptr<XmlQueryDoc> query_doc, std::unique_ptr<XmlDefDoc> query_def, 
        boost::optional<ReportTypeRef> output_ref = boost::none, std::shared_ptr<const XmlDefDoc> output_def = nullptr, ReportType::Traits m_output_traits = ReportType::Traits(),
        boost::optional<CustomQueryRef> ref = boost::none);

    std::wstring m_name;
    std::vector<ReportTypeRef> m_input_refs;
    std::shared_ptr<const XmlQueryDoc> m_query_doc;
    std::shared_ptr<const XmlDefDoc> m_query_def;
    boost::optional<ReportTypeRef> m_output_ref;
    std::shared_ptr<const XmlDefDoc> m_output_def;
    ReportType::Traits m_output_traits;
    boost::optional<CustomQueryRef> m_ref;
};

class CustomQueries
{
public:
    CustomQueries(Database& db, Basis& basis, Registry& registry);
    void restore(Transaction& t);

    CustomQuery                         create_custom_query(std::wstring name, std::vector<ReportTypeRef> input_type_refs);
    void                                add_custom_query(CustomQuery& custom_query, Transaction& t);
    CustomQuery                         get_custom_query(Key custom_query_key) const;
    boost::optional<CustomQuery>        find_by_output(ReportTypeRef output_ref) const;
    void                                remove_custom_query(Key custom_query_key);
    void                                remove_custom_query(Key custom_query_key, Transaction& t);
    void                                remove_custom_query(ReportTypeRef output_ref);

private:
    CustomQueryData                     pack_custom_query(const CustomQuery& custom_query) const;
    CustomQuery                         unpack_custom_query(const CustomQueryData& custom_query_data) const;

    void                                install_custom_query(const CustomQuery& custom_query, Transaction& t);
    void                                uninstall_custom_query(const CustomQuery& custom_query, Transaction& t);
    void                                install_output(const CustomQuery& custom_query, Transaction& t);
    void                                uninstall_output(const CustomQuery& custom_query, Transaction& t);
    void                                install_selection(const CustomQuery& custom_query, Transaction& t);
    void                                uninstall_selection(const CustomQuery& custom_query, Transaction& t);

    Database& m_db;
    Sources& m_sources;
    ReportTypes& m_report_types;
    Selections& m_selections;
};

}} // namespace TR { namespace Core {