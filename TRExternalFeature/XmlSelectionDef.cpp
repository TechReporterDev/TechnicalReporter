#include "stdafx.h"
#include "XmlSelectionDef.h"
namespace TR { namespace External {
using namespace xml_tools;

std::string XmlRegexReplaceNode::get_pattern() const
{
    auto pattern_node = find_child(*this, "pattern");
    if (!pattern_node)
    {
        throw std::logic_error("Invalid selection definition");
    }

    return get_node_text(*pattern_node);
}

std::string XmlRegexReplaceNode::get_replacement() const
{
    auto replacement_node = find_child(*this, "replacement");
    if (!replacement_node)
    {
        throw std::logic_error("Invalid selection definition");
    }

    return get_node_text(*replacement_node);
}

XmlSelectionScriptNode::ScriptType XmlSelectionScriptNode::get_script_type() const
{
    auto script_type_attr = find_attribute(*this, "type");
    if (!script_type_attr)
    {
        throw std::logic_error("Invalid selection definition");
    }

    if (attr_value_equal(*script_type_attr, "COMMAND"))
    {
        return ScriptType::COMMAND;
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
        throw std::logic_error("Invalid selection script 'type' attribute");
    }
}

std::wstring XmlSelectionScriptNode::get_command() const
{
    return get_node_wtext(*this);
}

const XmlNode* XmlSelectionScriptNode::get_xslt() const
{
    auto xslt_root = begin_child(*this);
    if (xslt_root == nullptr || get_node_type(*xslt_root) != XmlNodeType::ELEMENT)
    {
        _ASSERT(false);
        throw std::logic_error("Invalid content of xslt script");
    }
    return xslt_root;
};


XmlSelectionInputNode::Mode XmlSelectionInputNode::get_mode() const
{
    auto mode_attr = find_attribute(*this, "mode");
    if (!mode_attr)
    {
        return Mode::STRICT_MODE;
    }

    if (attr_value_equal(*mode_attr, "STRICT"))
    {
        return Mode::STRICT_MODE;
    }
    else if (attr_value_equal(*mode_attr, "NULLABLE"))
    {
        return Mode::NULLABLE_MODE;
    }
    else if (attr_value_equal(*mode_attr, "OPTIONAL"))
    {
        return Mode::OPTIONAL_MODE;
    }
    else
    {
        _ASSERT(false);
        throw std::logic_error("Invalid input 'mode' attribute");
    }
}

ReportTypeUUID XmlSelectionInputNode::get_uuid() const
{
    return ReportTypeUUID(stl_tools::gen_uuid(get_node_wtext(*this)));
}

const XmlNode* XmlSelectionScriptNode::get_regex_parser() const
{
    auto regex_parser_root = begin_child(*this);
    if (regex_parser_root == nullptr || get_node_type(*regex_parser_root) != XmlNodeType::ELEMENT)
    {
        _ASSERT(false);
        throw std::logic_error("Invalid content of xslt script");
    }
    return regex_parser_root;
}

UUID XmlSelectionDef::get_uuid() const
{
    auto uuid_node = find_child(*this, "uuid");
    if (!uuid_node)
    {
        throw std::logic_error("Invalid selection definition");
    }

    return stl_tools::gen_uuid(get_node_wtext(*uuid_node));    
}

ReportTypeUUID XmlSelectionDef::get_output_uuid() const
{
    auto output_uuid_node = find_child(*this, "output_uuid");
    if (!output_uuid_node)
    {
        throw std::logic_error("Invalid selection definition");
    }

    auto output_uuid = ReportTypeUUID(stl_tools::gen_uuid(get_node_wtext(*output_uuid_node)));
    return output_uuid;
}

const XmlSelectionScriptNode& XmlSelectionDef::get_script() const
{
    auto script_node = find_child(*this, "script");
    if (!script_node)
    {
        throw std::logic_error("Invalid selection definition document");
    }
    return static_cast<const XmlSelectionScriptNode&>(*script_node);
}

const XmlSelectionInputNode* XmlSelectionDef::next_input(const XmlSelectionInputNode* input_node)
{
    _ASSERT(input_node);
    return static_cast<const XmlSelectionInputNode*>(next_child(input_node));
}

XmlSelectionDef::XmlSelectionInputRange XmlSelectionDef::get_input() const
{
    if (auto inputs_node = find_child(*this, "input"))
    {
        return XmlSelectionInputRange(
            XmlSelectionInputIterator(static_cast<const XmlSelectionInputNode*>(begin_child(*inputs_node))),
            XmlSelectionInputIterator());
    }

    return XmlSelectionInputRange();
}

const XmlPreprocessorNode* XmlSelectionDef::next_preprocessor(const XmlPreprocessorNode* preprocessor_node)
{
    _ASSERT(preprocessor_node);
    return static_cast<const XmlPreprocessorNode*>(next_child(preprocessor_node));
}

XmlSelectionDef::ConstPreprocessorRange XmlSelectionDef::get_preprocessors() const
{
    if (auto preprocessor_node = find_child(*this, "preprocessor"))
    {
        return ConstPreprocessorRange(
            ConstPreprocessorIterator(static_cast<const XmlPreprocessorNode*>(begin_child(*preprocessor_node))),
            ConstPreprocessorIterator());       
    }

    return ConstPreprocessorRange();
}

}}// namespace TR { namespace External {