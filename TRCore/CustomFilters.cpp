#include "stdafx.h"
#include "CustomFilters.h"
#include "Basis.h"
#include "Registry.h"
#include "Transaction.h"
#include "ServicesPacking.h"
#include "CustomFilterData-odb.hxx"
namespace TR { namespace Core {

static UUID CUSTOM_FILTER_SELECTION_UUID = stl_tools::gen_uuid("45F33886-BE94-48E4-BF09-5E803CE84429");

class CustomFilterSelection: public Selection
{
public:
    CustomFilterSelection(const CustomFilters& custom_filters, const CustomFilter& custom_filter):
        Selection(CUSTOM_FILTER_SELECTION_UUID),
        m_plan({custom_filter.get_input_ref(), custom_filter.get_output_ref()}),
        m_custom_filters(custom_filters),
        m_custom_filter_key(custom_filter.get_key())
    {
    }

    virtual SelectionPlan get_plan() const override
    {
        return m_plan;
    }

    virtual Functor prepare_functor(SourceRef /*source_ref*/) const override
    {
        auto& output = dynamic_cast<const RegularReportType&>(*m_plan.m_output_ref);
        auto make_content = output.prepare_make_content_from_xml();     
        auto custom_filter = m_custom_filters.get_custom_filter(m_custom_filter_key);

        auto node_filters = custom_filter.get_node_filters();
        auto filter_def = output.get_def_doc(); 

        return [node_filters, filter_def, make_content](std::vector<std::shared_ptr<Content>>& inputs) -> std::unique_ptr<Content> {

            auto filter_doc = XML::create_query(*filter_def);
            for (auto& node_filter : node_filters)
            {
                add_node_filter(*filter_doc, node_filter);
            }

            auto& regular_content = dynamic_cast<RegularContent&>(*inputs[0]);
            auto result = do_query(*filter_doc, regular_content.as_regular_doc());
            if (!result)
            {
                return nullptr;
            }

            return make_content(std::move(result));
        };
    }   

private:
    SelectionPlan m_plan;
    const CustomFilters& m_custom_filters;
    Key m_custom_filter_key;
};

CustomFilter::CustomFilter(std::wstring name, ReportTypeRef input_ref):
    CustomFilter(0, std::move(name), input_ref, boost::none, {})
{
}

CustomFilter::CustomFilter(Key key, std::wstring name, ReportTypeRef input_ref, boost::optional<ReportTypeRef> output_ref, std::vector<XmlNodeFilter> node_filters) :
    m_key(key),
    m_name(std::move(name)),
    m_input_ref(input_ref),
    m_output_ref(output_ref),
    m_node_filters(std::move(node_filters))
{
}

Key CustomFilter::get_key() const
{
    if (!m_key)
    {
        throw Exception(L"CustomFilter is not persist.");
    }
    return m_key;
}

std::wstring CustomFilter::get_name() const
{
    return m_name;
}

ReportTypeRef CustomFilter::get_input_ref() const
{
    return m_input_ref;
}

ReportTypeRef CustomFilter::get_output_ref() const
{
    _ASSERT(m_output_ref);
    if (!m_output_ref)
    {
        throw Exception(L"CustomFilter is not persist.");
    }
    return *m_output_ref;
}

std::vector<XmlNodeFilter> CustomFilter::get_node_filters() const
{
    return m_node_filters;
}

void CustomFilter::set_node_filters(std::vector<XmlNodeFilter> node_filters)
{
    m_node_filters = std::move(node_filters);
}

CustomFilters::CustomFilters(Database& db, Basis& basis, Registry& registry):
    m_db(db),
    m_sources(registry.m_sources),
    m_report_types(basis.m_report_types),
    m_selections(basis.m_transformations.m_selections),
    m_resource_policies(registry.m_resource_policies),
    m_running(false)
{
    m_report_types.connect_add_report_type([&](ReportTypeRef ref, Transaction& t){
        if (m_running)
        {
            return;
        }

        for (auto& custom_filter: find_by_input(ref))
        {
            install_custom_filter(custom_filter, t);
        }

    }, CUSTOM_FILTERS_PRIORITY);
}

void CustomFilters::run(Executive*, Transaction& t)
{
    transact_assign(m_running, true, t);
}

void CustomFilters::add_custom_filter(CustomFilter& custom_filter, Transaction& t, boost::optional<ReportTypeUUID> output_uuid)
{
    _ASSERT(!custom_filter.m_key);
    custom_filter.m_output_ref = unpack(output_uuid ? *output_uuid: ReportTypeUUID(stl_tools::gen_uuid()), &m_report_types);
    custom_filter.m_key = odb::persist(m_db, pack_custom_filter(custom_filter));
    install_custom_filter(custom_filter, t);
}

void CustomFilters::update_custom_filter(const CustomFilter& custom_filter, Transaction& t) const
{
    _ASSERT(custom_filter.m_key);
    m_db.update(pack_custom_filter(custom_filter));

    for (auto& source : m_sources.get_sources())
    {
        m_resource_policies.set_setup_uuid(source, custom_filter.get_output_ref(), stl_tools::gen_uuid(), t);
        //m_conveyor.refresh(source, custom_filter.get_output_ref(), t);
    }
}

CustomFilter CustomFilters::get_custom_filter(Key custom_filter_key) const
{
    ReadOnlyTransaction t(m_db);
    CustomFilterData custom_filter_data;
    m_db.load<CustomFilterData>(custom_filter_key, custom_filter_data);
    return unpack_custom_filter(custom_filter_data);
}

void CustomFilters::remove_custom_filter(Key custom_filter_key, Transaction& t)
{
    CustomFilterData custom_filter_data;
    m_db.load<CustomFilterData>(custom_filter_key, custom_filter_data);

    auto custom_filter = unpack_custom_filter(custom_filter_data);
    uninstall_custom_filter(custom_filter, t);
    m_db.erase(custom_filter_data);
}

std::vector<CustomFilter> CustomFilters::find_by_input(ReportTypeRef input_ref) const
{
    ReadOnlyTransaction t(m_db);
    std::vector<CustomFilter> custom_filters;
    for (auto& custom_filter_data : m_db.query<CustomFilterData>(odb::query<CustomFilterData>::input_uuid.value == input_ref.get_uuid()))
    {
        custom_filters.push_back(unpack_custom_filter(custom_filter_data));
    }
    return custom_filters;
}

boost::optional<CustomFilter> CustomFilters::find_by_output(ReportTypeRef output_ref) const
{
    ReadOnlyTransaction t(m_db);
    auto query_result = m_db.query<CustomFilterData>(odb::query<CustomFilterData>::output_uuid.value == output_ref.get_uuid());
    if (query_result.empty())
    {
        return boost::none;
    }
    return unpack_custom_filter(*query_result.begin());
}

void CustomFilters::remove_custom_filter(ReportTypeRef output_ref, Transaction& t)
{
    auto query_result = m_db.query<CustomFilterData>(odb::query<CustomFilterData>::output_uuid.value == output_ref.get_uuid().m_value);
    _ASSERT(query_result.size() == 1);
    if (query_result.empty())
    {
        throw Exception(L"Custom Query not found");
    }

    auto custom_filter = unpack_custom_filter(*query_result.begin());
    uninstall_custom_filter(custom_filter, t);
    m_db.erase(*query_result.begin());
}

CustomFilterData CustomFilters::pack_custom_filter(const CustomFilter& custom_filter) const
{
    return CustomFilterData(
        custom_filter.m_key,
        pack(custom_filter.m_name),
        pack(custom_filter.m_input_ref),
        pack(*custom_filter.m_output_ref),
        stl_tools::make_blob(custom_filter.m_node_filters)
    );
}

CustomFilter CustomFilters::unpack_custom_filter(const CustomFilterData& custom_filter_data) const
{
    CustomFilter custom_filter(
        custom_filter_data.m_key,
        unpack(custom_filter_data.m_name),
        unpack(custom_filter_data.m_input_uuid, &m_report_types),
        unpack(custom_filter_data.m_output_uuid, &m_report_types),
        {});

    stl_tools::read_blob(custom_filter_data.m_node_filters, custom_filter.m_node_filters);
    return custom_filter;
}

void CustomFilters::install_custom_filter(const CustomFilter& custom_filter, Transaction& t)
{
    _ASSERT(custom_filter.m_key);

    auto& input = dynamic_cast<const RegularReportType&>(*custom_filter.m_input_ref);
    m_report_types.add_report_type(std::make_unique<GeneralRegularReportType>(
        custom_filter.m_output_ref->get_uuid(),
        custom_filter.get_name(),
        xml_tools::clone_doc(*input.get_def_doc()),
        ReportType::Trait::VIEW), t);

    m_selections.add_selection(std::make_shared<CustomFilterSelection>(*this, custom_filter), t);
}

void CustomFilters::uninstall_custom_filter(const CustomFilter& custom_filter, Transaction& t)
{
    _ASSERT(custom_filter.m_key);

    m_selections.remove_selection({custom_filter.m_input_ref, *custom_filter.m_output_ref}, t);
    m_report_types.remove_report_type(custom_filter.m_output_ref->get_uuid(), t);
}

}} //namespace TR { namespace Core {