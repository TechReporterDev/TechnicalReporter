#include "stdafx.h"
#include "CustomQueries.h"
#include "Basis.h"
#include "Registry.h"
#include "Transaction.h"
#include "ServicesPacking.h"
#include "CustomQueryData-odb.hxx"
namespace TR { namespace Core {

static UUID CUSTOM_QUERY_SELECTION_UUID = stl_tools::gen_uuid("8EB93D46-1E27-437B-80CA-16CD7EA4BE6B");

class CustomQuerySelection: public Selection
{
public:
    CustomQuerySelection(const SelectionDirection& direction, std::shared_ptr<const XmlQueryDoc> query_doc, std::shared_ptr<const XmlDefDoc> query_def):
        Selection(CUSTOM_QUERY_SELECTION_UUID),
        m_direction(direction),
        m_query_doc(query_doc),
        m_query_def(query_def)
    {
    }

    virtual SelectionPlan get_plan() const override
    {
        return SelectionPlan{
            std::vector<SelectionInput>(m_direction.m_input_refs.begin(), m_direction.m_input_refs.end()),
            m_direction.m_output_ref
        };
    }

    virtual Functor prepare_functor(SourceRef /*source_ref*/) const override
    {
        auto& output = dynamic_cast<const RegularReportType&>(*m_direction.m_output_ref);
        auto make_content = output.prepare_make_content_from_xml();
        auto query_doc = m_query_doc;
        auto query_def = m_query_def;

        return [query_doc, query_def, make_content](std::vector<std::shared_ptr<Content>>& inputs) -> std::unique_ptr<Content> {
            std::vector<const XmlRegularDoc*> parts;
            for (auto& input : inputs)
            {
                auto& xml_content = dynamic_cast<RegularContent&>(*input);
                parts.push_back(&xml_content.as_regular_doc());
            }

            auto result = filtrate_input(parts, *query_doc);
            if (!result)
            {
                return nullptr;
            }

            return make_content(std::move(result));         
        };
    }

    // method full of hacks for better perfomance
    static std::unique_ptr<XmlRegularDoc> filtrate_input(std::vector<const XmlRegularDoc*> parts, const XmlQueryDoc& query_doc)
    {
        _ASSERT(!parts.empty());
        if (parts.size() == 1)
        {
            return do_query(query_doc, *parts[0]);
        }

        xmlDoc doc = {0};
        doc._private = (void*)(&query_doc.get_bound_def());
        xmlNode root = {0};
        doc.children = &root;
        root.name = BAD_CAST "custom_query";
        root.type = XML_ELEMENT_NODE;
        root._private = (void*)(&query_doc.get_root()->get_bound_def());

        std::list<xmlNode> children;
        children.push_back(*parts[0]->children);
        root.children = &children.front();

        for (size_t i = 1; i < parts.size(); ++i)
        {
            auto& back = children.back();
            children.push_back(*parts[i]->children);
            back.next = &children.back();           
        }       

        return do_query(query_doc, static_cast<XML::XmlRegularDoc&>(doc));
    }

private:
    SelectionDirection m_direction;
    std::shared_ptr<const XmlQueryDoc> m_query_doc;
    std::shared_ptr<const XmlDefDoc> m_query_def;
};

CustomQuery::CustomQuery(std::wstring name, std::vector<ReportTypeRef> input_type_refs, std::unique_ptr<XmlQueryDoc> query_doc, std::unique_ptr<XmlDefDoc> query_def,
    boost::optional<ReportTypeRef> output_ref, std::shared_ptr<const XmlDefDoc> output_def, ReportType::Traits output_traits, boost::optional<CustomQueryRef> ref):
    m_name(name),
    m_input_refs(std::move(input_type_refs)),
    m_query_doc(std::move(query_doc)),
    m_query_def(std::move(query_def)),
    m_output_ref(output_ref),
    m_output_def(output_def),
    m_output_traits(output_traits),
    m_ref(ref)
{
}

std::wstring CustomQuery::get_name() const
{
    return m_name;
}

std::vector<ReportTypeRef> CustomQuery::get_input_refs() const
{
    return m_input_refs;
}

const XmlQueryDoc& CustomQuery::get_query_doc() const
{
    _ASSERT(m_query_doc);
    return *m_query_doc;
}

void CustomQuery::set_query_doc(std::unique_ptr<XmlQueryDoc> query_doc)
{
    m_query_doc = std::move(query_doc);
}

std::shared_ptr<const XmlDefDoc> CustomQuery::get_query_def() const
{
    _ASSERT(m_query_def);
    return m_query_def;
}

ReportTypeRef CustomQuery::get_output_ref() const
{
    _ASSERT(m_output_ref);
    if (!m_output_ref)
    {
        throw Exception(L"Custom Query is not persist.");
    }
    return *m_output_ref;
}

CustomQueryRef CustomQuery::get_ref() const
{
    _ASSERT(m_ref);
    if (!m_ref)
    {
        throw Exception(L"Custom Query is not persist.");
    }
    return *m_ref;
}

CustomQuery::operator CustomQueryRef() const
{
    return get_ref();
}

CustomQueries::CustomQueries(Database& db, Basis& basis, Registry& registry):
    m_db(db),
    m_sources(registry.m_sources),
    m_report_types(basis.m_report_types),
    m_selections(basis.m_transformations.m_selections)
{
}

void CustomQueries::restore(Transaction& t)
{
    for (auto& custom_query_data : m_db.query<CustomQueryData>())
    {
        auto custom_query = unpack_custom_query(custom_query_data);
        install_custom_query(custom_query, t);
    }
}

std::unique_ptr<XmlDefDoc> create_query_definition(std::vector<std::unique_ptr<XmlDefDoc>> input_defs)
{
    _ASSERT(!input_defs.empty());
    if (input_defs.size() == 1)
    {
        return std::move(input_defs[0]);
    }

    auto composed_doc = compose(std::move(input_defs), "custom_query");
    auto root = get_root(*composed_doc);
    set_attr_value(*root, "class", "STRUCT");
    set_attr_value(*root, "caption", "Custom Query");
    return stl_tools::static_pointer_cast<XmlDefDoc>(std::move(composed_doc));
}

CustomQuery CustomQueries::create_custom_query(std::wstring name, std::vector<ReportTypeRef> input_refs)
{
    _ASSERT(!input_refs.empty());
    std::vector<std::unique_ptr<XmlDefDoc>> input_defs;
    ReportType::Traits output_traits;
    for (auto input_ref : input_refs)
    {
        auto& input_type = dynamic_cast<const RegularReportType&>(*input_ref);
        input_defs.push_back(xml_tools::clone_doc(*input_type.get_def_doc()));
        output_traits = output_traits | (input_type.get_traits() & ReportType::Trait::VIEW);
    }

    auto query_def = create_query_definition(std::move(input_defs));
    auto query_doc = XML::create_query(*query_def);
    return CustomQuery(std::move(name), std::move(input_refs), std::move(query_doc), std::move(query_def), 
        boost::none, nullptr, output_traits);
}

void CustomQueries::add_custom_query(CustomQuery& custom_query, Transaction& t)
{   
    _ASSERT(!custom_query.m_ref);

    custom_query.m_output_def = XML::get_output_def(*custom_query.m_query_doc);
    custom_query.m_output_ref = unpack(ReportTypeUUID(stl_tools::gen_uuid()), &m_report_types);
    install_custom_query(custom_query, t);
    custom_query.m_ref = CustomQueryRef(this, odb::persist(m_db, pack_custom_query(custom_query)));
}

CustomQuery CustomQueries::get_custom_query(Key custom_query_key) const
{
    ReadOnlyTransaction t(m_db);
    CustomQueryData custom_query_data;
    m_db.load<CustomQueryData>(custom_query_key, custom_query_data);
    return unpack_custom_query(custom_query_data);
}

boost::optional<CustomQuery> CustomQueries::find_by_output(ReportTypeRef output_ref) const
{
    ReadOnlyTransaction t(m_db);
    auto query_result = m_db.query<CustomQueryData>(odb::query<CustomQueryData>::output_uuid.value == output_ref.get_uuid());
    if (query_result.empty())
    {
        return boost::none;
    }   

    //_ASSERT(std::distance(query_result.begin(), query_result.end()) == 1);
    return unpack_custom_query(*query_result.begin());
}

void CustomQueries::remove_custom_query(Key custom_query_key)
{
    Transaction t(m_db);
    remove_custom_query(custom_query_key, t);
    t.commit();
}

void CustomQueries::remove_custom_query(Key custom_query_key, Transaction& t)
{
    CustomQueryData custom_query_data;
    m_db.load<CustomQueryData>(custom_query_key, custom_query_data);

    auto custom_query = unpack_custom_query(custom_query_data);
    uninstall_custom_query(custom_query, t);
    m_db.erase(custom_query_data);
}

void CustomQueries::remove_custom_query(ReportTypeRef output_ref)
{
    Transaction t(m_db);
    auto query_result = m_db.query<CustomQueryData>(odb::query<CustomQueryData>::output_uuid.value == output_ref.get_uuid().m_value);
    _ASSERT(query_result.size() == 1);
    if (query_result.empty())
    {
        throw Exception(L"Custom Query not found");
    }
    
    auto custom_query = unpack_custom_query(*query_result.begin());
    uninstall_custom_query(custom_query, t);
    m_db.erase(*query_result.begin());
    t.commit();
}

CustomQueryData CustomQueries::pack_custom_query(const CustomQuery& custom_query) const
{
    return CustomQueryData(
        custom_query.m_ref ? custom_query.m_ref->get_key() : 0,
        pack(custom_query.m_name),
        pack(custom_query.m_input_refs),
        pack(*custom_query.m_query_doc),
        pack(*custom_query.m_query_def),
        pack(*custom_query.m_output_ref),
        pack(*custom_query.m_output_def),
        custom_query.m_output_traits.get_value()
    );
}

CustomQuery CustomQueries::unpack_custom_query(const CustomQueryData& custom_query_data) const
{
    auto query_def = unpack(custom_query_data.m_query_def);
    auto query_doc = unpack(custom_query_data.m_query_doc, *query_def);
    
    CustomQuery custom_query(
        unpack(custom_query_data.m_name),
        unpack(custom_query_data.m_input_uuids, &m_report_types),
        std::move(query_doc),
        std::move(query_def)
    );

    custom_query.m_output_ref = unpack(custom_query_data.m_output_uuid, &m_report_types);
    custom_query.m_output_def = unpack(custom_query_data.m_output_def);
    custom_query.m_output_traits = ReportType::Trait(custom_query_data.m_output_traits);
    custom_query.m_ref = CustomQueryRef(this, custom_query_data.m_key);
    return custom_query;
}

void CustomQueries::install_custom_query(const CustomQuery& custom_query, Transaction& t)
{   
    install_output(custom_query, t);
    install_selection(custom_query, t);
}

void CustomQueries::uninstall_custom_query(const CustomQuery& custom_query, Transaction& t)
{
    uninstall_selection(custom_query, t);
    uninstall_output(custom_query, t);
}

void CustomQueries::install_output(const CustomQuery& custom_query, Transaction& t)
{
    _ASSERT(custom_query.m_output_ref);
    m_report_types.add_report_type(std::make_unique<GeneralRegularReportType>(
        custom_query.m_output_ref->get_uuid(),
        custom_query.get_name(),
        xml_tools::clone_doc(*custom_query.m_output_def),
        custom_query.m_output_traits), t);
}

void CustomQueries::uninstall_output(const CustomQuery& custom_query, Transaction& t)
{
    _ASSERT(custom_query.m_output_ref);
    m_report_types.remove_report_type(custom_query.m_output_ref->get_uuid(), t);
}

void CustomQueries::install_selection(const CustomQuery& custom_query, Transaction& t)
{
    SelectionDirection direction(custom_query.get_input_refs(), *custom_query.m_output_ref);
    m_selections.add_selection(std::make_shared<CustomQuerySelection>(direction, custom_query.m_query_doc, custom_query.m_query_def), t);
}

void CustomQueries::uninstall_selection(const CustomQuery& custom_query, Transaction& t)
{
    SelectionDirection direction(custom_query.get_input_refs(), *custom_query.m_output_ref);
    m_selections.remove_selection(direction, t);
}

}} //namespace TR { namespace Core {