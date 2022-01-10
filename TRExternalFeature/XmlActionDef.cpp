#include "stdafx.h"
#include "XmlActionDef.h"
namespace TR { namespace External {
using namespace xml_tools;

XmlActionOutputDef::Syntax XmlActionOutputDef::get_syntax() const
{
    auto syntax_node = find_child(*this, "syntax");
    if (!syntax_node)
    {
        throw std::logic_error("Invalid action definition document");
    }

    if (node_text_equal(*syntax_node, "TEXT"))
    {
        return Syntax::PLAIN_TEXT;
    }
    else if (node_text_equal(*syntax_node, "XML"))
    {
        return Syntax::PLAIN_XML;
    }
    else if (node_text_equal(*syntax_node, "REGULAR"))
    {
        return Syntax::REGULAR_XML;
    }
    else
    {
        _ASSERT(false);
        throw std::logic_error("Invalid report type definition document: invalid content of 'syntax' node");
    }
}

std::string XmlActionOutputDef::get_definition() const
{
    auto content_def = find_child(*this, "content_definition");
    if (!content_def)
    {
        throw std::logic_error("Invalid report type definition document");
    }

    auto content_def_root = begin_child(*content_def);
    if (!content_def_root)
    {
        throw std::logic_error("Invalid report type definition document");
    }

    auto def_doc = xml_tools::create_doc();
    xml_tools::set_root(*def_doc, clone_node(*content_def_root));
    return as_string(*def_doc);
}

UUID XmlActionDef::get_uuid() const
{
    auto uuid_node = find_child(*this, "uuid");
    if (!uuid_node)
    {
        throw std::logic_error("Invalid action definition");
    }

    return stl_tools::gen_uuid(get_node_wtext(*uuid_node));
}

std::wstring XmlActionDef::get_name() const
{
    auto name_node = find_child(*this, "name");
    if (!name_node)
    {
        throw std::logic_error("Invalid action definition");
    }

    return get_node_wtext(*name_node);
}

std::string XmlActionDef::get_params_def() const
{
    auto params_def = xml_tools::create_doc();
    set_root(*params_def, clone_node(get_params_root()));
    return as_string(*params_def);
}

const XmlNode& XmlActionDef::get_params_root() const
{
    auto params_def = find_child(*this, "params_definition");
    if (!params_def)
    {
        throw std::logic_error("Invalid action definition");
    }

    auto params_def_root = begin_child(*params_def);
    if (!params_def_root)
    {
        throw std::logic_error("Invalid action definition document");
    }

    return *params_def_root;
}

const XmlActionOutputDef* XmlActionDef::get_output_def() const
{
    return static_cast<const XmlActionOutputDef*>(find_child(*this, "output_definition"));
}

XmlActionOutputTransformDef::ScriptType XmlActionOutputTransformDef::get_script_type() const
{
    auto script_type_attr = find_attribute(*this, "type");
    if (!script_type_attr)
    {
        throw std::logic_error("Invalid action delegate definition");
    }

    else if (attr_value_equal(*script_type_attr, "XSLT"))
    {
        return ScriptType::XSLT;
    }
    else if (attr_value_equal(*script_type_attr, "REGEX_PARSER"))
    {
        return ScriptType::REGEX_PARSER;
    }
    else
    {
        _ASSERT(false);
        throw std::logic_error("Invalid script 'type' attribute");
    }
}

const XmlNode* XmlActionOutputTransformDef::get_xslt() const
{
    auto xslt_root = begin_child(*this);
    if (xslt_root == nullptr || get_node_type(*xslt_root) != XmlNodeType::ELEMENT)
    {
        _ASSERT(false);
        throw std::logic_error("Invalid content of xslt script");
    }
    return xslt_root;
};

std::string XmlActionOutputTransformDef::get_xslt_stylesheet() const
{
    auto stylesheet_doc = xml_tools::create_doc();
    xml_tools::set_root(*stylesheet_doc, xml_tools::clone_node(*get_xslt()));
    return xml_tools::as_string(*stylesheet_doc);
}

const XmlNode* XmlActionOutputTransformDef::get_regex_grammar_node() const
{
    auto regex_parser_root = begin_child(*this);
    if (regex_parser_root == nullptr || get_node_type(*regex_parser_root) != XmlNodeType::ELEMENT)
    {
        _ASSERT(false);
        throw std::logic_error("Invalid content of regex parser script");
    }
    return regex_parser_root;
}

std::string XmlActionOutputTransformDef::get_regex_grammar() const
{
    auto grammar_doc = xml_tools::create_doc();
    xml_tools::set_root(*grammar_doc, xml_tools::clone_node(*get_regex_grammar_node()));
    return xml_tools::as_string(*grammar_doc);
}

ActionUUID XmlActionDelegateDef::get_host_action_uuid() const
{
    auto host_action_node = find_child(*this, "host_action_uuid");
    if (!host_action_node)
    {
        throw std::logic_error("Invalid action delegate definition");
    }
    
    return ActionUUID(stl_tools::gen_uuid(get_node_wtext(*host_action_node)));
}

ActionUUID XmlActionDelegateDef::get_guest_action_uuid() const
{
    auto guest_action_node = find_child(*this, "guest_action_uuid");
    if (!guest_action_node)
    {
        throw std::logic_error("Invalid action delegate definition");
    }

    return ActionUUID(stl_tools::gen_uuid(get_node_wtext(*guest_action_node)));
}

const XmlNode& XmlActionDelegateDef::get_params_node() const
{
    auto params_node = find_child(*this, "params");
    if (!params_node)
    {
        throw std::logic_error("Invalid action delegate definition");
    }
    return *params_node;
}

const XmlActionOutputTransformDef* XmlActionDelegateDef::get_output_transform_def() const
{
    auto output_transform_node = find_child(*this, "output_transform");
    return static_cast<const XmlActionOutputTransformDef*>(output_transform_node);
}

UUID XmlActionShortcutDef::get_uuid() const
{
    auto uuid_node = find_child(*this, "uuid");
    if (!uuid_node)
    {
        throw std::logic_error("Invalid action shortcut definition");
    }

    return ActionUUID(stl_tools::gen_uuid(get_node_wtext(*uuid_node)));
}

std::wstring XmlActionShortcutDef::get_name() const
{
    auto name_node = find_child(*this, "name");
    if (!name_node)
    {
        throw std::logic_error("Invalid action shortcut definition");
    }

    return get_node_wtext(*name_node);
}

ActionUUID XmlActionShortcutDef::get_host_action_uuid() const
{
    auto host_action_node = find_child(*this, "host_action_uuid");
    if (!host_action_node)
    {
        throw std::logic_error("Invalid action shortcut definition");
    }

    return ActionUUID(stl_tools::gen_uuid(get_node_wtext(*host_action_node)));
}

boost::optional<ReportTypeUUID> XmlActionShortcutDef::get_output_uuid() const
{
    auto output_node = find_child(*this, "output");
    if (!output_node)
    {
        return boost::none;
    }

    auto uuid_node = find_child(*output_node, "uuid");
    if (!uuid_node)
    {
        throw std::logic_error("Invalid action shortcut definition");
    }

    return ReportTypeUUID(stl_tools::gen_uuid(get_node_wtext(*uuid_node)));
}

const XmlNode& XmlActionShortcutDef::get_params_node() const
{
    auto params_node = find_child(*this, "params");
    if (!params_node)
    {
        throw std::logic_error("Invalid action shortcut definition");
    }
    return *params_node;
}

}}// namespace TR { namespace External {