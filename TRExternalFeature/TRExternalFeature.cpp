#include "stdafx.h"
#include "TRExternalFeature.h"
#include "CommandSourceType.h"
#include "CommandSelection.h"
#include "RegexParserSelection.h"
#include "XsltSelection.h"
#include "ScriptActionDelegate.h"
#include "SyslogInputTransport.h"
#include "StaticActionShortcut.h"
#include "ComposedPreprocessor.h"
#include "W32Cmd.h"
#include "TRCore\CoreImpl.h"
#include <boost\filesystem.hpp>

namespace TR { namespace External {
namespace {
std::unique_ptr<XsltStylesheet> create_stylesheet(const XmlNode& xslt_template_body, bool omit_body_root = false)
{
    static std::string xml_stylesheet =
        "<?xml version=\"1.0\"?>"
        "<xsl:stylesheet version=\"1.0\" xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\" xmlns:exsl=\"http://exslt.org/common\" extension-element-prefixes=\"exsl\">"
        "<xsl:template match=\"*\" xml:space=\"default\">"
        "</xsl:template>"
        "</xsl:stylesheet>";

    auto stylesheet_doc = xml_tools::parse(xml_stylesheet);
    auto template_root = begin_child(*get_root(*stylesheet_doc));
    _ASSERT(template_root != nullptr);
    if (omit_body_root)
    {
        if (auto space_attr = xml_tools::find_attribute(xslt_template_body, "space"))
        {
            if (xml_tools::attr_value_equal(*space_attr, "preserve"))
            {
                auto root_space_attr = xml_tools::find_attribute(*template_root, "space");
                _ASSERT(root_space_attr != nullptr);
                xml_tools::set_attr_value(*root_space_attr, "preserve");
            }           
        }

        for (auto& xslt_template_node : get_child_nodes(xslt_template_body))
        {
            clone_into(xslt_template_node, *template_root);
        }
    }
    else
    {
        clone_into(xslt_template_body, *template_root);
    }

    return xslt_parse_stylesheet_doc(std::move(stylesheet_doc));
}
} //namespace {
    

std::unique_ptr<XmlFeatureDef> load_feature_def(const std::wstring& file_system_path)
{       
    return open_feature_def((boost::filesystem::path(file_system_path) / L"Feature.xml").wstring());
}

class ExternalAction: public Action
{
public:
    ExternalAction(const XmlActionDef& action_def):
        Action(ActionUUID(action_def.get_uuid()), action_def.get_name(), XML::parse_properties_def(action_def.get_params_def())),
        m_action_def(action_def)
    {
    }

    // override
    virtual MakeContent prepare_make_content() const override
    {
        auto output_def = m_action_def.get_output_def();
        if (!output_def)
        {
            return nullptr;
        }

        switch (output_def->get_syntax())
        {
        case XmlActionOutputDef::Syntax::PLAIN_TEXT:
            return [](Blob blob, const XmlDoc&){
                return std::make_unique<PlainTextContent>(std::string(blob.begin(), blob.end()), nullptr); 
            };

        case XmlActionOutputDef::Syntax::PLAIN_XML:
            return [](Blob blob, const XmlDoc&){
                return std::make_unique<PlainXmlContent>(std::string(blob.begin(), blob.end()));
            };

        case XmlActionOutputDef::Syntax::REGULAR_XML:
        {
            std::shared_ptr<XML::XmlDefDoc> def_doc = XML::parse_definition(output_def->get_definition());
            return [def_doc](Blob blob, const XmlDoc&){
                return std::make_unique<GeneralRegularContent>(std::string(blob.begin(), blob.end()), def_doc);
            };
        }
    
        default:
            _ASSERT(false);
        }

        return nullptr;
    }

    virtual std::unique_ptr<ReportType> make_report_type(UUID report_type_uuid, std::wstring report_type_name, const XmlDoc& params) const override
    {
        auto output_def = m_action_def.get_output_def();
        if (!output_def)
        {
            return nullptr;
        }

        switch (output_def->get_syntax())
        {
        case XmlActionOutputDef::Syntax::PLAIN_TEXT:
            return std::make_unique<PlainTextReportType>(report_type_uuid, report_type_name);

        case XmlActionOutputDef::Syntax::PLAIN_XML:
            return std::make_unique<PlainXmlReportType>(report_type_uuid, report_type_name);

        case XmlActionOutputDef::Syntax::REGULAR_XML:
            return std::make_unique<GeneralRegularReportType>(report_type_uuid, report_type_name, XML::parse_definition(output_def->get_definition()));

        default:
            _ASSERT(false);
        }

        return nullptr;
    }

private:
    const XmlActionDef& m_action_def;
};

ExternalSourceTypeFactory::ExternalSourceTypeFactory(const std::wstring& feature_path, const XmlSourceTypeDef& source_type_def):
    SourceTypeFactory(source_type_def.get_uuid(), source_type_def.get_name()),
    m_source_type_def(source_type_def)
{
}

std::unique_ptr<SourceType> ExternalSourceTypeFactory::create_source_type(const CoreImpl& core_impl) const
{
    auto source_type = std::make_unique<CommandSourceType>(
        m_source_type_def.get_uuid(),
        m_source_type_def.get_name(),
        XML::parse_properties_def(m_source_type_def.get_settings()),
        stl_tools::null_uuid());

    for (auto& download_node : m_source_type_def.get_downloads())
    {
        auto& report_type = core_impl.m_core_domain.m_basis.m_report_types.get_report_type(download_node.get_report_type_uuid());
        auto command_stylesheet = create_stylesheet(download_node.get_command_node(), true);
        source_type->add_download_command(report_type, { 
            xslt_make_command(std::move(command_stylesheet)), 
            download_node.must_discard_empty() });
    }

    for (auto& stream_node : m_source_type_def.get_streams())
    {
        auto& stream_type = core_impl.m_core_domain.m_basis.m_stream_types.get_stream_type(stream_node.get_stream_type_uuid());
        std::unique_ptr<StreamTransport> stream_transport;
        switch (stream_node.get_protocol())
        {
        case XmlSourceTypeDef::XmlStreamNode::SYSLOG:
            {
                auto grammar_doc = stl_tools::static_pointer_cast<xml_tools::XmlGrammarDoc>(xml_tools::create_doc());
                xml_tools::set_root(*grammar_doc, xml_tools::clone_node(stream_node.get_parser_node()));

                auto stylesheet_doc = xml_tools::create_doc();
                set_root(*stylesheet_doc, clone_node(stream_node.get_format_node()));
                auto format_stylesheet = xslt_parse_stylesheet_doc(std::move(stylesheet_doc));

                stream_transport = std::make_unique<SyslogInputTransport>(stream_type,
                    create_stylesheet(stream_node.get_address_node(), true),
                    std::move(grammar_doc),
                    std::move(format_stylesheet));
                break;
            }

        default:
            _ASSERT(false);
        }
        source_type->add_stream_transport(std::move(stream_transport));
    }

    for (auto& action_node : m_source_type_def.get_actions())
    {
        auto& action = core_impl.m_core_domain.m_basis.m_actions.get_action(action_node.get_action_uuid());
        std::vector<ReportTypeRef> reload_refs;
        for (auto reload_uuid: action_node.get_reload_uuids())
        {
            reload_refs.push_back(core_impl.m_core_domain.m_basis.m_report_types.get_report_type(reload_uuid));
        }
        auto command_stylesheet = create_stylesheet(action_node.get_command_node(), true);
        source_type->add_action_command(action, xslt_make_command(std::move(command_stylesheet)), std::move(reload_refs));
    }

    return std::move(source_type);
}

ExternalReportTypeFactory::ExternalReportTypeFactory(const std::wstring& file_system_path, const XmlReportTypeDef& report_type_def):
    ReportTypeFactory(report_type_def.get_uuid(), report_type_def.get_name()),
    m_report_type_def(report_type_def)
{
}

std::unique_ptr<ReportType> ExternalReportTypeFactory::create_report_type(const CoreImpl& core_impl) const
{
    auto report_type_uuid = m_report_type_def.get_uuid();
    auto report_type_name = m_report_type_def.get_name();

    switch (m_report_type_def.get_syntax())
    {
    case XmlReportTypeDef::Syntax::PLAIN_TEXT:
        return std::make_unique<PlainTextReportType>(report_type_uuid, report_type_name);

    case XmlReportTypeDef::Syntax::PLAIN_XML:
        return std::make_unique<PlainXmlReportType>(report_type_uuid, report_type_name);

    case XmlReportTypeDef::Syntax::REGULAR_XML:
        {
            auto content_def = XML::parse_definition(m_report_type_def.get_definition());
            return std::make_unique<GeneralRegularReportType>(report_type_uuid, report_type_name, std::move(content_def));
        }
    case XmlReportTypeDef::Syntax::CHECKLIST_XML:
        {
            auto compliance_def = XML::parse_compliance_def(m_report_type_def.get_definition());
            return std::make_unique<CheckListReportType>(report_type_uuid, report_type_name, std::move(compliance_def));
        }
    default:
        _ASSERT(false);     
    }

    return nullptr; // to supress warning
}

ExternalStreamTypeFactory::ExternalStreamTypeFactory(const std::wstring& file_system_path, const XmlStreamTypeDef& stream_type_def):
    StreamTypeFactory(stream_type_def.get_uuid(), stream_type_def.get_name()),
    m_stream_type_def(stream_type_def)
{
}

std::unique_ptr<StreamType> ExternalStreamTypeFactory::create_stream_type(const CoreImpl& core_impl) const
{
    auto message_def = stl_tools::static_pointer_cast<XmlDefDoc>(xml_tools::create_doc());
    xml_tools::set_root(*message_def, xml_tools::clone_node(m_stream_type_def.get_message_def()));
    return std::make_unique<StreamType>(m_stream_type_def.get_uuid(), m_stream_type_def.get_name(), std::move(message_def));
}

ExternalActionFactory::ExternalActionFactory(const std::wstring& file_system_path, const XmlActionDef& action_def):
    ActionFactory(ActionUUID(action_def.get_uuid()), action_def.get_name(), action_def.get_params_def()),
    m_action_def(action_def)
{
}

std::unique_ptr<Action> ExternalActionFactory::create_action(const CoreImpl& core_impl) const
{
    return std::make_unique<ExternalAction>(m_action_def);
}

ExternalActionDelegateFactory::ExternalActionDelegateFactory(const std::wstring& file_system_path, const XmlActionDelegateDef& action_delegate_def):
    ActionDelegateFactory({action_delegate_def.get_host_action_uuid()}, action_delegate_def.get_guest_action_uuid()),
    m_action_delegate_def(action_delegate_def)
{
}

std::unique_ptr<ActionDelegate> ExternalActionDelegateFactory::create_action_delegate(const CoreImpl& core_impl) const
{
    auto params_transform_functor = xslt_params_transform(create_stylesheet(m_action_delegate_def.get_params_node()));
    
    ScriptActionDelegate::OutputTransformFunctor output_transform_func; 
    if (auto output_transform_def = m_action_delegate_def.get_output_transform_def())
    {
        switch (output_transform_def->get_script_type())
        {
        case XmlActionOutputTransformDef::ScriptType::REGEX_PARSER:
            output_transform_func = regex_parser_output_transform(output_transform_def->get_regex_grammar());
            break;

        case XmlActionOutputTransformDef::ScriptType::XSLT:
            output_transform_func = xslt_output_transform(xslt_parse_stylesheet_doc(xml_tools::parse(output_transform_def->get_xslt_stylesheet())));
            break;

        default:
            _ASSERT(false);
        }
    }

    ActionRef host_action_ref = core_impl.m_core_domain.m_basis.m_actions.get_action(m_action_delegate_def.get_host_action_uuid());
    ActionRef guest_action_ref = core_impl.m_core_domain.m_basis.m_actions.get_action(m_action_delegate_def.get_guest_action_uuid());
    return std::make_unique<ScriptActionDelegate>(host_action_ref, guest_action_ref, params_transform_functor, output_transform_func);
}

ExternalShortcutOutputFactory::ExternalShortcutOutputFactory(const std::wstring& file_system_path, const XmlActionShortcutDef& action_shortcut_def):
    ReportTypeFactory(*action_shortcut_def.get_output_uuid(), action_shortcut_def.get_name()),
    m_action_shortcut_def(action_shortcut_def)
{
}

std::unique_ptr<ReportType> ExternalShortcutOutputFactory::create_report_type(const CoreImpl& core_impl) const
{
    auto params_doc = stl_tools::static_pointer_cast<XmlDefDoc>(xml_tools::create_doc());
    xml_tools::set_root(*params_doc, xml_tools::clone_node(m_action_shortcut_def.get_params_node()));

    auto& host_action = core_impl.m_core_domain.m_basis.m_actions.get_action(m_action_shortcut_def.get_host_action_uuid());
    return host_action.make_report_type(get_report_type_uuid(), get_report_type_name(), *params_doc);
}

ExternalActionShortcutFactory::ExternalActionShortcutFactory(const std::wstring& file_system_path, const XmlActionShortcutDef& action_shortcut_def):
    ActionShortcutFactory(action_shortcut_def.get_uuid(), action_shortcut_def.get_name(), action_shortcut_def.get_host_action_uuid()),
    m_action_shortcut_def(action_shortcut_def)
{
}

std::unique_ptr<ActionShortcut> ExternalActionShortcutFactory::create_action_shortcut(const CoreImpl& core_impl) const
{
    ActionRef host_action_ref = core_impl.m_core_domain.m_basis.m_actions.get_action(m_host_action_uuid);
    boost::optional<ReportTypeRef> output_ref;
    if (auto output_uuid = m_action_shortcut_def.get_output_uuid())
    {
        output_ref = core_impl.m_core_domain.m_basis.m_report_types.get_report_type(*output_uuid);
    }
    auto params_doc = stl_tools::static_pointer_cast<XmlDefDoc>(xml_tools::create_doc());
    xml_tools::set_root(*params_doc, xml_tools::clone_node(m_action_shortcut_def.get_params_node()));

    return std::make_unique<StaticActionShortcut>(
        m_uuid,
        m_name,
        host_action_ref,
        output_ref,
        std::move(params_doc)
    );
}

ExternalSelectionFactory::ExternalSelectionFactory(const std::wstring& file_system_path, const XmlSelectionDef& selection_def):
    SelectionFactory({}, selection_def.get_output_uuid()),
    m_selection_def(selection_def)
{
}

std::unique_ptr<Selection> ExternalSelectionFactory::create_selection(const CoreImpl& core_impl) const
{
    auto& report_types = core_impl.m_core_domain.m_basis.m_report_types;
    std::vector<SelectionInput> inputs;
    for (auto& input_node : m_selection_def.get_input())
    {
        inputs.push_back({ report_types.get_report_type(input_node.get_uuid()), SelectionInput::Mode(input_node.get_mode()) });
    }
    auto output_ref = report_types.get_report_type(m_selection_def.get_output_uuid()).get_ref();
    auto selection_plan = SelectionPlan(inputs, output_ref);

    ComposedPreprocessor preprocessor;
    for (auto& preprocessor_node : m_selection_def.get_preprocessors())
    {
        auto& regex_replace_node = static_cast<const XmlRegexReplaceNode&>(preprocessor_node);
        preprocessor.add_regex_replace(regex_replace_node.get_pattern(), regex_replace_node.get_replacement());
    }

    auto& script_node = m_selection_def.get_script();
    std::unique_ptr<Selection> selection;
    switch (script_node.get_script_type())
    {
    case XmlSelectionScriptNode::ScriptType::REGEX_PARSER:
        {
            auto parser_doc = xml_tools::create_doc();
            xml_tools::set_root(*parser_doc, xml_tools::clone_node(*script_node.get_regex_parser()));
            selection = std::make_unique<RegexParserSelection>(
                m_selection_def.get_uuid(),
                selection_plan,
                xml_tools::as_string(*parser_doc), preprocessor.empty() ? Preprocessor(nullptr): preprocessor);

            break;
        }

    case XmlSelectionScriptNode::ScriptType::COMMAND:
        selection = std::make_unique<CommandSelection>(
            m_selection_def.get_uuid(),
            selection_plan,
            script_node.get_command());

        break;
    
    case XmlSelectionScriptNode::ScriptType::XSLT:
        {
            auto xslt_doc = xml_tools::create_doc();
            xml_tools::set_root(*xslt_doc, xml_tools::clone_node(*script_node.get_xslt()));
            selection = std::make_unique<XsltSelection>(
                m_selection_def.get_uuid(),
                selection_plan,
                xml_tools::as_string(*xslt_doc));

            break;
        }

    default:
        _ASSERT(false);
    }
    return selection;
}

ExternalFeature::ExternalFeature(const std::wstring& file_system_path):
    ExternalFeature(file_system_path, load_feature_def(file_system_path))
{
    ::xmlSubstituteEntitiesDefault(1);
    xmlLoadExtDtdDefaultValue = 1;
    ::exsltRegisterAll();
    ::xslt_tools_init();
}

ExternalFeature::ExternalFeature(const std::wstring& file_system_path, std::unique_ptr<XmlFeatureDef> feature_def):
    ComposedFeature(feature_def->get_feature_uuid(), feature_def->get_feature_name()),
    m_feature_def(std::move(feature_def))
{
    if (auto dependencies_def = m_feature_def->get_dependencies())
    {
        for (auto& dependency_def : dependencies_def->get_dependencies())
        {
            m_dependencies.push_back(dependency_def.get_feature_uuid());
        }
    }

    for (auto& report_type_def : m_feature_def->get_report_types())
    {
        add_report_type_factory(std::make_unique<ExternalReportTypeFactory>(file_system_path, report_type_def));
    }

    for (auto& stream_type_def : m_feature_def->get_stream_types())
    {
        add_stream_type_factory(std::make_unique<ExternalStreamTypeFactory>(file_system_path, stream_type_def));
    }

    for (auto& action_def : m_feature_def->get_actions())
    {
        add_action_factory(std::make_unique<ExternalActionFactory>(file_system_path, action_def));
    }

    for (auto& action_delegate_def : m_feature_def->get_action_delegates())
    {
        add_action_delegate_factory(std::make_unique<ExternalActionDelegateFactory>(file_system_path, action_delegate_def));
    }

    for (auto& action_shortcut_def : m_feature_def->get_action_shortcuts())
    {
        if (auto output_uuid = action_shortcut_def.get_output_uuid())
        {
            add_report_type_factory(std::make_unique<ExternalShortcutOutputFactory>(file_system_path, action_shortcut_def));
        }

        add_action_shortcut_factory(std::make_unique<ExternalActionShortcutFactory>(file_system_path, action_shortcut_def));
    }

    for (auto& source_type_def : m_feature_def->get_source_types())
    {
        add_source_type_factory(std::make_unique<ExternalSourceTypeFactory>(file_system_path, source_type_def));
    }

    for (auto& selection_def : m_feature_def->get_selections())
    {
        add_selection_factory(std::make_unique<ExternalSelectionFactory>(file_system_path, selection_def));
    }

    for (auto& grouping_def : m_feature_def->get_groupings())
    {
        add_grouping_definition({grouping_def.get_name(), grouping_def.get_input_uuid(), grouping_def.get_output_uuid()});
    }

    for (auto& compliance_def : m_feature_def->get_compliances())
    {
        add_compliance_definition({compliance_def.get_name(), compliance_def.get_input_uuid(), compliance_def.get_output_uuid()});
    }

    for (auto& validation_def : m_feature_def->get_validations())
    {
        add_validation_definition({validation_def.get_name(), validation_def.get_input_uuid(), validation_def.get_output_uuid()});
    }
}

}} //namespace TR { namespace External {
