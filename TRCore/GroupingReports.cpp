#include "stdafx.h"
#include "GroupingReports.h"
#include "Basis.h"
#include "Registry.h"
#include "Transaction.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
#include "ServicesPacking.h"
#include "GroupingReportData-odb.hxx"
namespace TR { namespace Core {

namespace{

static UUID APPEND_SELECTION_UUID = stl_tools::gen_uuid("825458E6-02CB-4B88-A53C-EC73D918D77B");
static UUID GROUPING_CHILDS_UUID = stl_tools::gen_uuid("69CD2ABA-95D4-42D0-BDDB-F0D8E11B64F9");

class AppendSelectionOutput: public GeneralRegularReportType
{
public:
    AppendSelectionOutput::AppendSelectionOutput(UUID uuid, std::wstring name, ReportTypeRef input_ref, Traits traits = Traits(), int version = 0):
        GeneralRegularReportType(uuid, std::move(name), [this]{ return create_definition(); }, traits, version),
        m_input_ref(input_ref)
    {
    }

    std::unique_ptr<XmlDefDoc> create_definition() const
    {
        using namespace xml_tools;
        auto def_doc = stl_tools::static_pointer_cast<XmlDefDoc>(create_doc());
        auto& source_def = set_root(*def_doc, create_element("source"));
        set_attr_value(source_def, "class", "STRUCT");
        set_attr_value(source_def, "caption", "Source");

        auto& sources_def = add_element(source_def, "sources");
        set_attr_value(sources_def, "class", "LIST");
        set_attr_value(sources_def, "caption", "Sources");

        auto& source_link_def = add_element(sources_def, "source");
        set_attr_value(source_link_def, "class", "LINK");
        set_attr_value(source_link_def, "xpath", "/source");
        
        auto& name_def = add_element(source_def, "name");
        set_attr_value(name_def, "class", "STRING");
        set_attr_value(name_def, "caption", "Name");

        auto& data_def = add_element(source_def, "data");
        set_attr_value(data_def, "class", "STRUCT");        
        set_attr_value(data_def, "caption", "Report");

        auto& source_report_type = dynamic_cast<const RegularReportType&>(*m_input_ref);
        auto source_report_def = source_report_type.get_def_doc();
        auto& source_report_root_def = add_node(data_def, clone_node(*source_report_def->get_root_def()));
        set_attr_value(source_report_root_def, "optional", "yes");
        return std::move(def_doc);
    }

private:
    ReportTypeRef m_input_ref;
};

class AppendSelection: public Selection
{
public:
    AppendSelection(ReportTypeRef input_ref, ReportTypeRef child_grouping_ref, ReportTypeRef output_ref):
        Selection(APPEND_SELECTION_UUID),
        m_input_ref(input_ref),
        m_child_grouping_ref(child_grouping_ref),
        m_output_ref(output_ref)
    {
    }

    virtual SelectionPlan get_plan() const override
    {
        return SelectionPlan({
            {m_input_ref, SelectionInput::Mode::OPTIONAL_MODE},
            m_child_grouping_ref
            }, m_output_ref);
    }

    virtual Functor prepare_functor(SourceRef source_ref) const override
    {
        auto& append_selection_output = dynamic_cast<const AppendSelectionOutput&>(*m_output_ref);
        auto make_content = append_selection_output.prepare_make_content_from_xml();
        auto source_name = (*source_ref).get_name();
        return [make_content, source_name](std::vector<std::shared_ptr<Content>>& contents){
            using namespace xml_tools;

            _ASSERT(contents[1]);           

            auto doc = create_doc();
            auto& source_node = set_root(*doc, create_element("source"));

            auto& xml_content1 = dynamic_cast<XmlContent&>(*contents[1]);
            add_node(source_node, clone_node(*get_root(xml_content1.as_xml_doc())));

            auto& name_node = add_element(source_node, "name");
            add_text_node(name_node, source_name.c_str());

            auto& data_node = add_element(source_node, "data");

            if (contents[0])
            {
                auto& xml_content0 = dynamic_cast<XmlContent&>(*contents[0]);
                add_node(data_node, clone_node(*get_root(xml_content0.as_xml_doc())));              
            }

            return make_content(std::move(doc));
        };
    } 

private:
    ReportTypeRef m_input_ref;
    ReportTypeRef m_child_grouping_ref; 
    ReportTypeRef m_output_ref;
};

class SpliceGroupingOutput: public PlainXmlReportType
{
public:
    SpliceGroupingOutput(UUID uuid, std::wstring name, Traits traits = Traits(), int version = 0):
        PlainXmlReportType(uuid, std::move(name), traits, version)
    {
    }
};

class SpliceGrouping: public Grouping
{
public:
    SpliceGrouping(const GroupingDirection& direction):
        Grouping(GROUPING_CHILDS_UUID),
        m_direction(direction)
    {
    }

    SpliceGrouping(const ReportTypeRef& input_ref, const ReportTypeRef& output_ref):
        Grouping(GROUPING_CHILDS_UUID),
        m_direction({input_ref, output_ref})
    {
    }

    virtual GroupingDirection get_direction() const override
    {
        return m_direction;
    }
    virtual Functor prepare_functor(SourceRef, const std::vector<SourceRef>&) const override
    {
        auto& splice_grouping_output = dynamic_cast<const SpliceGroupingOutput&>(*m_direction.m_output_ref);
        auto make_content = splice_grouping_output.prepare_make_content_from_xml();
        
        return [make_content](const std::vector<std::shared_ptr<Content>>& contents){
            using namespace xml_tools;
            auto doc = create_doc();
            auto& sources_node = set_root(*doc, create_element("sources"));

            for (auto& content : contents)
            {
                if (!content)
                {
                    continue;
                }

                auto& xml_content = dynamic_cast<XmlContent&>(*content);
                add_node(sources_node, clone_node(*get_root(xml_content.as_xml_doc())));
            }
            return make_content(std::move(doc));
        };
    }

private:
    GroupingDirection m_direction;
};
} //namespace{

GroupingReport::GroupingReport(std::wstring name, ReportTypeRef input_ref, ReportType::Traits output_traits, boost::optional<SourceRef> bound_source):
    m_name(std::move(name)),
    m_input_ref(input_ref),
    m_output_traits(output_traits),
    m_bound_source(bound_source)
{
}

GroupingReport::GroupingReport(std::wstring name, ReportTypeRef input_ref, ReportTypeRef output_ref, ReportType::Traits output_traits, ReportTypeRef child_grouping_ref, boost::optional<SourceRef> bound_source, GroupingReportRef ref):
    m_name(std::move(name)),
    m_input_ref(input_ref), 
    m_output_ref(output_ref),
    m_output_traits(output_traits),
    m_child_grouping_ref(child_grouping_ref),
    m_bound_source(bound_source),
    m_ref(ref)
{
}

std::wstring GroupingReport::get_name() const
{
    return m_name;
}

ReportTypeRef GroupingReport::get_input_ref() const
{
    return m_input_ref;
}

boost::optional<SourceRef> GroupingReport::get_bound_source() const
{
    return m_bound_source;
}

ReportTypeRef GroupingReport::get_output_ref() const
{
    _ASSERT(m_output_ref);
    if (!m_output_ref)
    {
        throw Exception(L"GroupingReport is not persist.");
    }
    return *m_output_ref;
}

ReportTypeRef GroupingReport::get_child_grouping_ref() const
{
    _ASSERT(m_child_grouping_ref);
    if (!m_child_grouping_ref)
    {
        throw Exception(L"GroupingReport is not persist.");
    }
    return *m_child_grouping_ref;
}

GroupingReportRef GroupingReport::get_ref() const
{
    _ASSERT(m_ref);
    if (!m_ref)
    {
        throw Exception(L"GroupingReport is not persist");
    }
    return *m_ref;
}

GroupingReport::operator GroupingReportRef() const
{
    return get_ref();
}

GroupingReports::GroupingReports(Database& db, Basis& basis, Registry& registry):
    m_db(db),
    m_report_types(basis.m_report_types),
    m_selections(basis.m_transformations.m_selections),
    m_groupings(basis.m_transformations.m_groupings),
    m_sources(registry.m_sources),
    m_roles(registry.m_roles),
    m_resource_policies(registry.m_resource_policies)
{
}

void GroupingReports::restore(Transaction& t)
{
    for (auto& grouping_report_data : m_db.query<GroupingReportData>())
    {
        auto grouping_report = unpack_grouping_report(grouping_report_data);
        install_grouping_report(grouping_report, t);
    }
}

GroupingReport GroupingReports::create_grouping_report(std::wstring name, ReportTypeRef input_ref)
{   
    return GroupingReport{
        name,
        input_ref,
        (*input_ref).get_traits() | ReportType::Trait::VIEW
    };
}

void GroupingReports::add_grouping_report(GroupingReport& grouping_report, ReportTypeUUID output_uuid, Transaction& t)
{
    _ASSERT(grouping_report.m_ref == boost::none);
    grouping_report.m_output_ref = unpack(output_uuid == stl_tools::null_uuid() ? ReportTypeUUID(stl_tools::gen_uuid()) : output_uuid, &m_report_types);
    grouping_report.m_child_grouping_ref = unpack(ReportTypeUUID(stl_tools::gen_uuid()), &m_report_types);  
    grouping_report.m_ref = GroupingReportRef(this, odb::persist(m_db, pack_grouping_report(grouping_report)));
    install_grouping_report(grouping_report, t);

    //defer child grouping
    auto default_role = m_roles.get_default_role();
    m_resource_policies.set_defer_policy(default_role, *grouping_report.m_child_grouping_ref, DeferPolicy::DEFER, t);
}

GroupingReport GroupingReports::get_grouping_report(Key grouping_report_key) const
{
    ReadOnlyTransaction t(m_db);
    GroupingReportData grouping_report_data;
    m_db.load<GroupingReportData>(grouping_report_key, grouping_report_data);
    return unpack_grouping_report(grouping_report_data);
}

boost::optional<GroupingReport> GroupingReports::find_by_output(ReportTypeRef output_ref) const
{
    ReadOnlyTransaction t(m_db);
    GroupingReportData grouping_report_data;
    auto query_result = m_db.query<GroupingReportData>(odb::query<GroupingReportData>::output_uuid.value == output_ref.get_uuid());
    if (query_result.empty())
    {
        return boost::none;
    }
    _ASSERT(query_result.size() == 1);
    return unpack_grouping_report(*query_result.begin());
}

bool GroupingReports::is_grouping_output(ReportTypeRef output_ref) const
{
    ReadOnlyTransaction t(m_db);
    GroupingReportData grouping_report_data;
    auto query_result = m_db.query<GroupingReportData>(odb::query<GroupingReportData>::output_uuid.value == output_ref.get_uuid());
    return query_result.empty() == false;   
}

void GroupingReports::remove_grouping_report(Key grouping_report_key, Transaction& t)
{
    GroupingReportData grouping_report_data;
    m_db.load<GroupingReportData>(grouping_report_key, grouping_report_data);
    auto grouping_report = unpack_grouping_report(grouping_report_data);
    uninstall_grouping_report(grouping_report, t);
    m_db.erase(grouping_report_data);
}

GroupingReportData GroupingReports::pack_grouping_report(const GroupingReport& grouping_report) const
{
    return GroupingReportData(
        grouping_report.m_ref ? grouping_report.m_ref->get_key() : 0,
        pack(grouping_report.m_name),
        pack(grouping_report.m_input_ref),
        pack(*grouping_report.m_output_ref),
        grouping_report.m_output_traits.get_value(),
        pack(*grouping_report.m_child_grouping_ref),
        pack(grouping_report.m_bound_source)
    );
}

GroupingReport GroupingReports::unpack_grouping_report(const GroupingReportData& grouping_report_data) const
{
    return GroupingReport(
        unpack(grouping_report_data.m_name),
        unpack(grouping_report_data.m_input_uuid, &m_report_types),
        unpack(grouping_report_data.m_output_uuid, &m_report_types),
        ReportType::Trait(grouping_report_data.m_output_traits),
        unpack(grouping_report_data.m_childs_grouping_uuid, &m_report_types),
        unpack(grouping_report_data.m_bound_source_key, &m_sources),
        GroupingReportRef(this, grouping_report_data.m_key)
    );  
}

void GroupingReports::install_grouping_report(const GroupingReport& grouping_report, Transaction& t)
{
    m_report_types.add_report_type(std::make_unique<AppendSelectionOutput>(
        grouping_report.m_output_ref->get_uuid(),
        grouping_report.m_name,
        grouping_report.m_input_ref,
        grouping_report.m_output_traits), t);

    m_report_types.add_report_type(std::make_unique<SpliceGroupingOutput>(
        grouping_report.m_child_grouping_ref->get_uuid(),
        grouping_report.m_name + L"(system)",
        grouping_report.m_output_traits), t);

    SelectionDirection selection_direction({grouping_report.m_input_ref, *grouping_report.m_child_grouping_ref}, *grouping_report.m_output_ref);
    m_selections.add_selection(std::make_shared<AppendSelection>(
        grouping_report.m_input_ref,
        *grouping_report.m_child_grouping_ref,
        *grouping_report.m_output_ref), t);

    GroupingDirection grouping_direction(*grouping_report.m_output_ref, *grouping_report.m_child_grouping_ref);
    m_groupings.add_grouping(std::make_shared<SpliceGrouping>(grouping_direction), t);
}

void GroupingReports::uninstall_grouping_report(const GroupingReport& grouping_report, Transaction& t)
{
    GroupingDirection grouping_direction(*grouping_report.m_output_ref, *grouping_report.m_child_grouping_ref);
    m_groupings.remove_grouping(grouping_direction, t);

    SelectionDirection selection_direction({grouping_report.m_input_ref, *grouping_report.m_child_grouping_ref}, *grouping_report.m_output_ref);
    m_selections.remove_selection(selection_direction, t);

    m_report_types.remove_report_type(grouping_report.m_child_grouping_ref->get_uuid(), t);
    m_report_types.remove_report_type(grouping_report.m_output_ref->get_uuid(), t);
}

}} //namespace TR { namespace Core {