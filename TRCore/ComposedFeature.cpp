#include "stdafx.h"
#include "ComposedFeature.h"
#include "CoreImpl.h"
namespace TR {  namespace Core {

SourceTypeFactory::SourceTypeFactory(SourceTypeUUID source_type_uuid, std::wstring source_type_name):
    m_source_type_uuid(source_type_uuid),
    m_source_type_name(std::move(source_type_name))
{
}

SourceTypeUUID SourceTypeFactory::get_source_type_uuid() const
{
    return m_source_type_uuid;
}

std::wstring SourceTypeFactory::get_source_type_name() const
{
    return m_source_type_name;
}

CustomSourceTypeFactory::CustomSourceTypeFactory(SourceTypeUUID source_type_uuid, std::wstring source_type_name, Factory factory):
    SourceTypeFactory(source_type_uuid, std::move(source_type_name)),
    m_factory(factory)
{
}

std::unique_ptr<SourceType> CustomSourceTypeFactory::create_source_type(const CoreImpl& core_impl) const
{
    return m_factory(core_impl);
}

ReportTypeFactory::ReportTypeFactory(ReportTypeUUID report_type_uuid, std::wstring report_type_name):
    m_report_type_uuid(report_type_uuid),
    m_report_type_name(std::move(report_type_name))
{
}

ReportTypeUUID ReportTypeFactory::get_report_type_uuid() const
{
    return m_report_type_uuid;
}

std::wstring ReportTypeFactory::get_report_type_name() const
{
    return m_report_type_name;
}

CustomReportTypeFactory::CustomReportTypeFactory(ReportTypeUUID report_type_uuid, std::wstring report_type_name, std::function<std::unique_ptr<ReportType>(const CoreImpl& core_impl)> factory):
    ReportTypeFactory(report_type_uuid, std::move(report_type_name)),
    m_factory(factory)
{
}

std::unique_ptr<ReportType> CustomReportTypeFactory::create_report_type(const CoreImpl& core_impl) const
{
    return m_factory(core_impl);
}

StreamTypeFactory::StreamTypeFactory(StreamTypeUUID stream_type_uuid, std::wstring stream_type_name):
    m_stream_type_uuid(stream_type_uuid),
    m_stream_type_name(std::move(stream_type_name))
{

}

StreamTypeUUID StreamTypeFactory::get_stream_type_uuid() const
{
    return m_stream_type_uuid;
}

std::wstring StreamTypeFactory::get_stream_type_name() const
{
    return m_stream_type_name;
}

ActionFactory::ActionFactory(ActionUUID action_uuid, std::wstring action_name, std::string params_def):
    m_action_uuid(std::move(action_uuid)),
    m_action_name(std::move(action_name)),
    m_params_def(std::move(params_def))
{
}

ActionUUID ActionFactory::get_action_uuid() const
{
    return m_action_uuid;
}

std::wstring ActionFactory::get_action_name() const
{
    return m_action_name;
}

std::string ActionFactory::get_params_def() const
{
    return m_params_def;
}

ActionDelegateFactory::ActionDelegateFactory(std::vector<ActionUUID> host_action_uuids, ActionUUID guest_action_uuid):
    m_host_action_uuids(std::move(host_action_uuids)),
    m_guest_action_uuid(guest_action_uuid)
{
}

std::vector<ActionUUID> ActionDelegateFactory::get_host_action_uuids() const
{
    return m_host_action_uuids;
}

ActionUUID ActionDelegateFactory::get_guest_action_uuid() const
{
    return m_guest_action_uuid;
}

ActionShortcutFactory::ActionShortcutFactory(UUID uuid, std::wstring name, ActionUUID host_action_uuid):
    m_uuid(uuid),
    m_name(std::move(name)),
    m_host_action_uuid(host_action_uuid)
{
}

UUID ActionShortcutFactory::get_uuid() const
{
    return m_uuid;
}

std::wstring ActionShortcutFactory::get_name() const
{
    return m_name;
}

ActionUUID ActionShortcutFactory::get_host_action_uuid() const
{
    return m_host_action_uuid;
}

SelectionFactory::SelectionFactory(std::vector<ReportTypeUUID> input_uuids, ReportTypeUUID output_uuid):
    m_input_uuids(std::move(input_uuids)),
    m_output_uuid(output_uuid)
{
}

std::vector<ReportTypeUUID> SelectionFactory::get_input_uuids() const
{
    return m_input_uuids;
}

ReportTypeUUID SelectionFactory::get_output_uuid() const
{
    return m_output_uuid;
}

CustomSelectionFactory::CustomSelectionFactory(std::vector<ReportTypeUUID> input_uuids, ReportTypeUUID output_uuid, Factory factory):
    SelectionFactory(std::move(input_uuids), output_uuid),
    m_factory(factory)
{
}

std::unique_ptr<Selection> CustomSelectionFactory::create_selection(const CoreImpl& core_impl) const
{
    return m_factory(core_impl);
}

GroupingDefinition::GroupingDefinition(std::wstring name, ReportTypeUUID input_uuid, ReportTypeUUID output_uuid):
    m_name(std::move(name)),
    m_input_uuid(input_uuid),
    m_output_uuid(output_uuid)
{
}

std::wstring GroupingDefinition::get_name() const
{
    return m_name;
}

ReportTypeUUID GroupingDefinition::get_input_uuid() const
{
    return m_input_uuid;
}

ReportTypeUUID GroupingDefinition::get_output_uuid() const
{
    return m_output_uuid;
}

ComplianceDefinition::ComplianceDefinition(std::wstring name, ReportTypeUUID input_uuid, ReportTypeUUID output_uuid):
    m_name(std::move(name)),
    m_input_uuid(input_uuid),
    m_output_uuid(output_uuid)
{
}

std::wstring ComplianceDefinition::get_name() const
{
    return m_name;
}

ReportTypeUUID ComplianceDefinition::get_input_uuid() const
{
    return m_input_uuid;
}

ReportTypeUUID ComplianceDefinition::get_output_uuid() const
{
    return m_output_uuid;
}

ValidationDefinition::ValidationDefinition(std::wstring name, ReportTypeUUID input_uuid, ReportTypeUUID output_uuid, boost::optional<std::string> default_validator):
    m_name(std::move(name)),
    m_input_uuid(input_uuid),
    m_output_uuid(output_uuid),
    m_default_validator(std::move(default_validator))
{
}

ValidationDefinition::ValidationDefinition(std::wstring name, ReportTypeUUID input_uuid, ReportTypeUUID output_uuid):
    ValidationDefinition(std::move(name), input_uuid, output_uuid, boost::none)
{
}

std::wstring ValidationDefinition::get_name() const
{
    return m_name;
}

ReportTypeUUID ValidationDefinition::get_input_uuid() const
{
    return m_input_uuid;
}

ReportTypeUUID ValidationDefinition::get_output_uuid() const
{
    return m_output_uuid;
}

boost::optional<std::string> ValidationDefinition::get_default_validator() const
{
    return m_default_validator;
}

ComposedFeature::ComposedFeature(UUID uuid, std::wstring name):
    Feature(uuid, std::move(name))
{
}

void ComposedFeature::add_dependency(UUID feature_uuid)
{
    m_dependencies.push_back(feature_uuid);
}

void ComposedFeature::add_source_type_factory(std::unique_ptr<SourceTypeFactory> source_type_factory)
{
    m_source_type_factories.push_back(std::move(source_type_factory));
}
void ComposedFeature::add_report_type_factory(std::unique_ptr<ReportTypeFactory> report_type_factory)
{
    m_report_type_factories.push_back(std::move(report_type_factory));
}

void ComposedFeature::add_stream_type_factory(std::unique_ptr<StreamTypeFactory> stream_type_factory)
{
    m_stream_type_factories.push_back(std::move(stream_type_factory));
}

void ComposedFeature::add_selection_factory(std::unique_ptr<SelectionFactory> selection_factory)
{
    m_selection_factories.push_back(std::move(selection_factory));
}

void ComposedFeature::add_action_factory(std::unique_ptr<ActionFactory> action_factory)
{
    m_action_factories.push_back(std::move(action_factory));
}

void ComposedFeature::add_action_delegate_factory(std::unique_ptr<ActionDelegateFactory> action_delegate_factory)
{
    m_action_delegate_factories.push_back(std::move(action_delegate_factory));
}

void ComposedFeature::add_action_shortcut_factory(std::unique_ptr<ActionShortcutFactory> action_shortcut_factory)
{
    m_action_shortcut_factories.push_back(std::move(action_shortcut_factory));
}

void ComposedFeature::add_grouping_definition(GroupingDefinition grouping_definition)
{
    m_grouping_definitions.push_back(std::move(grouping_definition));
}

void ComposedFeature::add_compliance_definition(ComplianceDefinition compliance_definition)
{
    m_compliance_definitions.push_back(std::move(compliance_definition));
}

void ComposedFeature::add_validation_definition(ValidationDefinition validation_definition)
{
    m_validation_definitions.push_back(std::move(validation_definition));
}

void ComposedFeature::install(CoreImpl& core_impl, bool restore, Transaction& t)
{
    for (auto& action_factory : m_action_factories | boost::adaptors::indirected)
    {
        core_impl.m_core_domain.m_basis.m_actions.add_action(action_factory.create_action(core_impl), t);
    }

    for (auto& report_type_factory : m_report_type_factories | boost::adaptors::indirected)
    {
        core_impl.m_core_domain.m_basis.m_report_types.add_report_type(report_type_factory.create_report_type(core_impl), t);
    }

    for (auto& stream_type_factory : m_stream_type_factories | boost::adaptors::indirected)
    {
        core_impl.m_core_domain.m_basis.m_stream_types.add_stream_type(stream_type_factory.create_stream_type(core_impl), t);
    }

    for (auto& action_delegate_factory : m_action_delegate_factories | boost::adaptors::indirected)
    {
        core_impl.m_core_domain.m_basis.m_action_delegates.add_action_delegate(action_delegate_factory.create_action_delegate(core_impl), t);
    }   

    for (auto& action_shortcut_factory : m_action_shortcut_factories | boost::adaptors::indirected)
    {
        core_impl.m_core_domain.m_basis.m_action_shortcuts.add_shortcut(action_shortcut_factory.create_action_shortcut(core_impl), t);
    }

    for (auto& source_type_factory : m_source_type_factories | boost::adaptors::indirected)
    {
        auto& source_type = core_impl.m_core_domain.m_basis.m_source_types.add_source_type(source_type_factory.create_source_type(core_impl), t);
        core_impl.m_core_domain.m_services.m_settings_capture.enable_capture_source_settings(source_type, t);
    }

    for (auto& selection_factory : m_selection_factories | boost::adaptors::indirected)
    {
        core_impl.m_core_domain.m_basis.m_transformations.m_selections.add_selection(selection_factory.create_selection(core_impl), t);
    }

    if (restore)
    {
        return;
    }

    for (auto& grouping_definition : m_grouping_definitions)
    {
        auto& grouping_reports = core_impl.m_core_domain.m_services.m_grouping_reports;

        auto& input_ref(core_impl.m_core_domain.m_basis.m_report_types.get_report_type(grouping_definition.get_input_uuid()));
        GroupingReport grouping_report = grouping_reports.create_grouping_report(grouping_definition.get_name(), input_ref);
        grouping_reports.add_grouping_report(grouping_report, grouping_definition.get_output_uuid(), t);        
    }   

    for (auto& compliance_definition : m_compliance_definitions)
    {
        auto& input_ref(core_impl.m_core_domain.m_basis.m_report_types.get_report_type(compliance_definition.get_input_uuid()));
        Compliance compliance(compliance_definition.get_name(), input_ref);
        core_impl.m_core_domain.m_services.m_compliances.add_compliance(compliance, t);
    }

    for (auto& validation_definition : m_validation_definitions)
    {
        auto& input = dynamic_cast<const RegularReportType&>(core_impl.m_core_domain.m_basis.m_report_types.get_report_type(validation_definition.get_input_uuid()));
        std::unique_ptr<XmlQueryDoc> default_validator;
        if (auto& xml_validator = validation_definition.get_default_validator())
        {
            default_validator = parse_validator(*xml_validator, *input.get_def_doc());
        }       

        ValidationInitializer validation_initializer(validation_definition.get_name(), input, validation_definition.get_output_uuid(), std::move(default_validator));
        core_impl.m_core_domain.m_services.m_validations.add_validation(std::move(validation_initializer), t);
    }
}

void ComposedFeature::uninstall(CoreImpl& core_impl, Transaction& t)
{
    auto& report_types = core_impl.m_core_domain.m_basis.m_report_types;

    for (auto& validation_definition : m_validation_definitions)
    {
        auto& output_ref = core_impl.m_core_domain.m_basis.m_report_types.get_report_type(validation_definition.get_output_uuid());
        auto validation = core_impl.m_core_domain.m_services.m_validations.find_by_output(output_ref);
        _ASSERT(validation);
        core_impl.m_core_domain.m_services.m_validations.remove_validation(validation->get_ref().get_key(), t);
    }

    for (auto& compliance_definition : m_compliance_definitions)
    {
        auto& output_ref = core_impl.m_core_domain.m_basis.m_report_types.get_report_type(compliance_definition.get_output_uuid());
        auto compliance = core_impl.m_core_domain.m_services.m_compliances.find_by_output(output_ref);
        _ASSERT(compliance);
        core_impl.m_core_domain.m_services.m_compliances.remove_compliance(compliance->get_ref().get_key(), t);
    }

    for (auto& grouping_definition : m_grouping_definitions)
    {
        auto& output_ref(core_impl.m_core_domain.m_basis.m_report_types.get_report_type(grouping_definition.get_output_uuid()));
        auto grouping_report = core_impl.m_core_domain.m_services.m_grouping_reports.find_by_output(output_ref);
        _ASSERT(grouping_report);

        core_impl.m_core_domain.m_services.m_grouping_reports.remove_grouping_report(grouping_report->get_ref().get_key(), t);
    }

    for (auto& selection_factory : m_selection_factories | boost::adaptors::indirected)
    {
        std::vector<ReportTypeRef> input_refs;
        for (auto input_uuid : selection_factory.get_input_uuids())
        {
            input_refs.push_back(report_types.get_report_type(input_uuid));
        }
        auto& output_ref = report_types.get_report_type(selection_factory.get_output_uuid());
        
        core_impl.m_core_domain.m_basis.m_transformations.m_selections.remove_selection({std::move(input_refs), output_ref}, t);
    }
    
    for (auto& source_type_factory : m_source_type_factories | boost::adaptors::indirected)
    {
        auto source_type_uuid = source_type_factory.get_source_type_uuid();
        core_impl.m_core_domain.m_services.m_settings_capture.disable_capture_source_settings(
            core_impl.m_core_domain.m_basis.m_source_types.get_source_type(source_type_uuid), t);
        core_impl.m_core_domain.m_basis.m_source_types.remove_source_type(source_type_uuid, t);
    }

    for (auto& action_shortcut_factory : m_action_shortcut_factories | boost::adaptors::indirected)
    {
        core_impl.m_core_domain.m_basis.m_action_shortcuts.remove_shortcut(action_shortcut_factory.get_uuid(), t);
    }

    for (auto& action_delegate_factory : m_action_delegate_factories | boost::adaptors::indirected)
    {
        auto& actions = core_impl.m_core_domain.m_basis.m_actions;
        std::vector<ActionRef> host_actions_refs;   
        for (auto host_action_uuid : action_delegate_factory.get_host_action_uuids())
        {
            host_actions_refs.push_back(actions.get_action(host_action_uuid));
        }

        auto guest_action_ref = ActionRef(actions.get_action(action_delegate_factory.get_guest_action_uuid()));
        core_impl.m_core_domain.m_basis.m_action_delegates.remove_action_delegate({host_actions_refs, guest_action_ref}, t);
    }

    for (auto& stream_type_factory : m_stream_type_factories | boost::adaptors::indirected)
    {
        core_impl.m_core_domain.m_basis.m_stream_types.remove_stream_type(stream_type_factory.get_stream_type_uuid(), t);
    }

    for (auto& report_type_factory : m_report_type_factories | boost::adaptors::indirected)
    {
        core_impl.m_core_domain.m_basis.m_report_types.remove_report_type(report_type_factory.get_report_type_uuid(), t);
    }

    for (auto& action_factory : m_action_factories | boost::adaptors::indirected)
    {
        core_impl.m_core_domain.m_basis.m_actions.remove_action(action_factory.get_action_uuid(), t);
    }
}

}} //namespace TR { namespace Core {