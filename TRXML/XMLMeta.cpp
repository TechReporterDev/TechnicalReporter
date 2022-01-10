#include "stdafx.h"
#include "XMLMeta.h"

namespace TR { namespace XML { namespace META {
using namespace xml_tools;

XmlActionNode* XmlActionNode::cast_meta_node(XmlMetaNode* meta_node)
{
    if (!meta_node)
    {
        return nullptr;
    }

    if (!node_name_equal(*meta_node, "ACTION"))
    {
        return nullptr;
    }

    return static_cast<XmlActionNode*>(meta_node);
}

const XmlActionNode* XmlActionNode::cast_meta_node(const XmlMetaNode* meta_node)
{
    return cast_meta_node(const_cast<XmlMetaNode*>(meta_node));
}

UUID XmlActionNode::get_action_uuid() const
{
    auto uuid_attr = find_attribute(*this, "uuid");
    if (!uuid_attr)
    {
        throw std::logic_error("Invalid action node");      
    }
    return stl_tools::gen_uuid(get_attr_wvalue(*uuid_attr));    
}

std::wstring XmlActionNode::get_caption() const
{
    if (auto caption_attr = find_attribute(*this, "caption"))
    {
        return get_attr_wvalue(*caption_attr);
    }
    return L"";
}

std::unique_ptr<XmlDoc> XmlActionNode::get_params(const XmlNode& context) const
{
    auto params_node = find_child(*this, "params");
    if (!params_node)
    {
        throw std::logic_error("Invalid action node");
    }

    static std::string xml_stylesheet =
        "<?xml version=\"1.0\"?>"
        "<xsl:stylesheet version=\"1.0\" xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\" xmlns:exsl=\"http://exslt.org/common\" extension-element-prefixes=\"exsl\">"
        "<xsl:template match=\"*\">"
        "</xsl:template>"
        "</xsl:stylesheet>";
    
    auto stylesheet_doc = parse(xml_stylesheet);
    auto template_node = begin_child(*get_root(*stylesheet_doc));
    _ASSERT(template_node);
    clone_into(*params_node, *template_node);

    auto stylesheet = xslt_parse_stylesheet_doc(std::move(stylesheet_doc));
    return xslt_apply_stylesheet(*stylesheet, (XmlDoc&)context);
}

}}}