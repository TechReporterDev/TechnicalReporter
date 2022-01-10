#include "stdafx.h"
#include "XmlReportTypeDef.h"
namespace TR {  namespace External {
using namespace xml_tools;

ReportTypeUUID XmlReportTypeDef::get_uuid() const
{
    auto uuid_node = find_child(*this, "uuid");
    if (!uuid_node)
    {
        throw std::logic_error("Invalid report type definition");
    }

    return ReportTypeUUID(stl_tools::gen_uuid(get_node_wtext(*uuid_node)));
}

std::wstring XmlReportTypeDef::get_name() const
{
    auto name_node = find_child(*this, "name");
    if (!name_node)
    {
        throw std::logic_error("Invalid report type definition");
    }

    return get_node_wtext(*name_node);
}

XmlReportTypeDef::Syntax XmlReportTypeDef::get_syntax() const
{
    auto syntax_node = find_child(*this, "syntax");
    if (!syntax_node)
    {
        throw std::logic_error("Invalid report type definition");
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
    else if (node_text_equal(*syntax_node, "CHECKLIST"))
    {
        return Syntax::CHECKLIST_XML;
    }
    else
    {
        _ASSERT(false);
        throw std::logic_error("Invalid report type definition: invalid 'syntax' node");
    }
}

std::string XmlReportTypeDef::get_definition() const
{
    auto def_doc = xml_tools::create_doc();
    set_root(*def_doc, clone_node(get_def_root()));
    return as_string(*def_doc);
}

const XmlNode& XmlReportTypeDef::get_def_root() const
{
    auto content_def = find_child(*this, "content_definition");
    if (!content_def)
    {
        throw std::logic_error("Invalid report type definition");
    }

    auto content_def_root = begin_child(*content_def);
    if (!content_def_root)
    {
        throw std::logic_error("Invalid report type definition");
    }

    return *content_def_root;
}

const XmlNode& XmlReportTypeDef::get_text_def() const
{
    auto content_def = find_child(*this, "content_definition");
    if (!content_def)
    {
        throw std::logic_error("Invalid report type definition");
    }

    auto content_def_root = begin_child(*content_def);
    if (!content_def_root)
    {
        throw std::logic_error("Invalid report type definition document");
    }

    return *content_def_root;
}

const XmlDefNode& XmlReportTypeDef::get_regular_def() const
{
    auto content_def = find_child(*this, "content_definition");
    if (!content_def)
    {
        throw std::logic_error("Invalid report type definition");
    }

    auto content_def_root = begin_child(*content_def);
    if (!content_def_root)
    {
        throw std::logic_error("Invalid report type definition");
    }

    // add some checks needed   
    return static_cast<const XmlDefNode&>(*content_def_root);
}

}}// namespace TR { namespace External {