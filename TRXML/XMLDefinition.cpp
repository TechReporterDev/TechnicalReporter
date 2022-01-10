#include "stdafx.h"
#include "XMLDefinition.h"
namespace TR { namespace XML {
using namespace xml_tools;

static std::wstring default_caption(L"no caption");
static std::wstring default_description(L"no description");

namespace{
XmlDefNode* begin_child_def(XmlDefNode& def_node)
{
    for (auto child_node = begin_child(def_node); child_node; child_node = next_child(child_node))
    {
        if (get_node_type(*child_node) != XmlNodeType::ELEMENT)
        {
            continue;
        }

        if (!node_name_equal(*child_node, "META"))
        {
            return static_cast<XmlDefNode*>(child_node);
        }
    }
    return nullptr;
}

const XmlDefNode* begin_child_def(const XmlDefNode& def_node)
{
    return begin_child_def(const_cast<XmlDefNode&>(def_node));
}

XmlDefNode* next_child_def(XmlDefNode* def_node)
{
    for (auto child_node = next_child(def_node); child_node; child_node = next_child(child_node))
    {
        if (get_node_type(*child_node) != XmlNodeType::ELEMENT)
        {
            continue;
        }

        if (!node_name_equal(*child_node, "META"))
        {
            return static_cast<XmlDefNode*>(child_node);
        }
    }
    return nullptr;
}

const XmlDefNode* next_child_def(const XmlDefNode* def_node)
{
    return next_child_def(const_cast<XmlDefNode*>(def_node));
}
} //namespace{

XmlDefNode* XmlDefDoc::get_root_def()
{
    return static_cast<XmlDefNode*>(get_root(*this));
}

const XmlDefNode* XmlDefDoc::get_root_def() const
{
    return const_cast<XmlDefDoc*>(this)->get_root_def();
}

const XmlDefNode* XmlMetaNode::get_owner_node() const
{
    if (auto parent_node = get_parent(*this))
    {
        _ASSERT(node_name_equal(*parent_node, "META"));
        return static_cast<const XmlDefNode*>(get_parent(*parent_node));
    }
    return nullptr;
}

std::wstring XmlDefNode::get_default_caption()
{
    return default_caption;
}

std::wstring XmlDefNode::get_default_description()
{
    return default_description;
}

const XmlMetaNode* XmlDefNode::next_meta_node(const XmlMetaNode* meta_node)
{
    return static_cast<const XmlMetaNode*>(next_child(meta_node));
}

XmlNodeClass XmlDefNode::get_class() const
{
    auto class_attr = find_attribute(*this, "class");
    if (!class_attr)
    {
        throw std::logic_error("Attribute class not found");
    }

    if (attr_value_equal(*class_attr, "LIST"))
    {
        return XmlNodeClass::LIST;
    }
    else if (attr_value_equal(*class_attr, "STRUCT"))
    {
        return XmlNodeClass::STRUCT;
    }
    else if (attr_value_equal(*class_attr, "STRING"))
    {
        return XmlNodeClass::TRIVIAL;
    }
    else if (attr_value_equal(*class_attr, "BOOLEAN"))
    {
        return XmlNodeClass::TRIVIAL;
    }
    else if (attr_value_equal(*class_attr, "INTEGER"))
    {
        return XmlNodeClass::TRIVIAL;
    }
    else if (attr_value_equal(*class_attr, "ENUM"))
    {
        return XmlNodeClass::TRIVIAL;
    }
    else if (attr_value_equal(*class_attr, "LINK"))
    {
        return XmlNodeClass::LINK;
    }
    else
    {
        throw std::logic_error("Unknown node \"class\" found");
    }

    return XmlNodeClass::NONE;
}

bool XmlDefNode::is_trivial() const
{
    return get_class() == XmlNodeClass::TRIVIAL;
}

bool XmlDefNode::is_link() const
{
    return get_class() == XmlNodeClass::LINK;
}

bool XmlDefNode::is_optional() const
{
    auto optional_attr = find_attribute(*this, "optional");
    if (optional_attr)
    {
        return attr_value_equal(*optional_attr, "yes");
    }
    return false;
}

void XmlDefNode::set_optional(bool value)
{
    if (value)
    {
        set_attr_value(*this, "optional", "yes");       
    }
    else
    {
        unset_attr(*this, "optional");
    }   
}

std::wstring XmlDefNode::get_caption() const
{
    if (auto caption_attr = find_attribute(*this, "caption"))
    {
        return get_attr_wvalue(*caption_attr);
    }
    return default_caption;
}

std::wstring XmlDefNode::format_caption() const
{
    std::wstring caption;
    if (auto parent = get_parent_def())
    {
        caption += parent->format_caption();
    }
    caption += L"/";
    caption += is_link() ? resolve().get_caption() : get_caption();
    return caption;
}

std::wstring XmlDefNode::get_description() const
{
    if (auto description_attr = find_attribute(*this, "description"))
    {
        return get_attr_wvalue(*description_attr);
    }
    return default_description;
}

std::string XmlDefNode::get_xpath() const
{
    if (auto xpath_attr = find_attribute(*this, "xpath"))
    {
        return get_attr_value(*xpath_attr);
    }
    throw std::logic_error("\"xpath\" attribute not found found");
}

const XmlDefNode& XmlDefNode::resolve() const
{
    if (auto resolved = find_xpath_node(*static_cast<XmlDoc*>(doc), get_xpath().c_str()))
    {
        return static_cast<XmlDefNode&>(*resolved);
    }

    throw std::logic_error("Document does not match to definition");
}

XmlDefNode* XmlDefNode::get_parent_def()
{
    return static_cast<XmlDefNode*>(get_parent(*this));
}

const XmlDefNode* XmlDefNode::get_parent_def() const
{
    return const_cast<XmlDefNode*>(this)->get_parent_def();
}

XmlDefNode::ConstMetaNodeIterator XmlDefNode::begin_meta_node() const
{
    if (auto meta_root = find_child(*this, "META"))
    {
        return ConstMetaNodeIterator(static_cast<const XmlMetaNode*>(begin_child(*meta_root)));
    }
    return ConstMetaNodeIterator();
}

XmlDefNode::ConstMetaNodeIterator XmlDefNode::end_meta_node() const
{
    return ConstMetaNodeIterator();
}

XmlDefNode::ConstMetaNodeIteratorRange XmlDefNode::get_meta_nodes() const
{
    return ConstMetaNodeIteratorRange(begin_meta_node(), end_meta_node());
}

std::wstring XmlTrivialDef::get_default_string(XmlTrivialType trivial_type)
{
    switch (trivial_type)
    {
    case XmlTrivialType::STRING:
        return L"";

    case XmlTrivialType::BOOLEAN:
        return boost::lexical_cast<std::wstring>(false);

    case XmlTrivialType::INTEGER:
        return boost::lexical_cast<std::wstring>(0);

    case XmlTrivialType::ENUM:
        return boost::lexical_cast<std::wstring>(0);

    default:
        _ASSERT(false);
    }
    return L"";
}

XmlTrivialType XmlTrivialDef::get_type() const
{
    auto class_attr = find_attribute(*this, "class");
    if (!class_attr)
    {
        throw std::logic_error("Attribute class not found");
    }

    else if (attr_value_equal(*class_attr, "STRING"))
    {
        return XmlTrivialType::STRING;
    }
    else if (attr_value_equal(*class_attr, "BOOLEAN"))
    {
        return XmlTrivialType::BOOLEAN;
    }
    else if (attr_value_equal(*class_attr, "INTEGER"))
    {
        return XmlTrivialType::INTEGER;
    }
    else if (attr_value_equal(*class_attr, "ENUM"))
    {
        return XmlTrivialType::ENUM;
    }
    else
    {
        throw std::logic_error("Unknown trivial \"type\" found");
    }

    return XmlTrivialType::NONE;
}

std::wstring XmlTrivialDef::get_default_string() const
{
    if (auto default_attr = find_attribute(*this, "default"))
    {
        return get_attr_wvalue(*default_attr);
    }
    return get_default_string(get_type());
}

std::vector<std::wstring> XmlEnumDef::get_names() const
{
    auto names_attr = find_attribute(*this, "names");
    if (!names_attr)
    {
        throw std::logic_error("Invalid enum definition");
    }
    
    std::vector<std::wstring> names;
    boost::split(names, get_attr_wvalue(*names_attr), boost::is_any_of(L"|"));
    if (names.empty())
    {
        throw std::logic_error("Invalid enum definition");
    }
    
    return names;
}

XmlDefNode* XmlStructDef::next_member_def(XmlDefNode* def_node)
{
    _ASSERT(def_node);
    return next_child_def(def_node);
}

const XmlDefNode* XmlStructDef::next_member_def(const XmlDefNode* def_node)
{
    return next_child_def(const_cast<XmlDefNode*>(def_node));
}

XmlStructDef::MemberDefIterator XmlStructDef::begin_member_def()
{
    return MemberDefIterator(begin_child_def(*this));
}

XmlStructDef::ConstMemberDefIterator XmlStructDef::begin_member_def() const
{
    return ConstMemberDefIterator(begin_child_def(*this));
}

XmlStructDef::MemberDefIterator XmlStructDef::end_member_def()
{
    return MemberDefIterator();
}

XmlStructDef::ConstMemberDefIterator XmlStructDef::end_member_def() const
{
    return ConstMemberDefIterator();
}

XmlStructDef::MemberDefRange XmlStructDef::get_member_defs()
{
    return MemberDefRange(begin_member_def(), end_member_def());
}

XmlStructDef::ConstMemberDefRange XmlStructDef::get_member_defs() const
{
    return ConstMemberDefRange(begin_member_def(), end_member_def());
}

XmlDefNode& XmlListDef::get_item_def()
{
    auto child_def = begin_child_def(*this);
    if (!child_def)
    {
        throw std::logic_error("Invalid list definition");
    }
    return *child_def;
}

const XmlDefNode& XmlListDef::get_item_def() const
{
    return const_cast<XmlListDef*>(this)->get_item_def();
}

ConstXmlDefNodeRange get_child_defs(const XmlDefNode& parent)
{
    using ConstDefNodeIterator = xml_tools::node_iterator<const XmlDefNode, next_child_def>;
    return boost::iterator_range<ConstDefNodeIterator>(
        ConstDefNodeIterator(begin_child_def(parent)),
        ConstDefNodeIterator());
}

std::unique_ptr<XmlDefDoc> parse_definition(const std::string& xml_def)
{
    std::unique_ptr<XmlDefDoc> def_doc(static_cast<XmlDefDoc*>(parse(xml_def).release()));
    inspect_definition(*def_doc);
    return def_doc;
}

static void inspect_node_def(const XmlDefNode& def_node);
static void inspect_list_def(const XmlDefNode& def_node)
{
    auto child_def = begin_child_def(def_node);
    if (!child_def)
    {
        throw std::logic_error("Empty list definition found");
    }

    inspect_node_def(*child_def);

    if (next_child_def(child_def))
    {
        throw std::logic_error("Definition of list contains more then one element");
    }
}

static void inspect_struct_def(const XmlDefNode& def_node)
{
    for (auto& child_def : get_child_defs(def_node))
    {
        inspect_node_def(child_def);
    }
}

static void inspect_trivial_def(const XmlDefNode& def_node)
{
    if (begin_child_def(def_node))
    {
        throw std::logic_error("Trivial class definition has child");
    }
}

static void inspect_link_def(const XmlDefNode& def_node)
{
    if (begin_child_def(def_node))
    {
        throw std::logic_error("Link class definition has child");
    }
}

static void inspect_node_def(const XmlDefNode& def_node)
{
    switch (def_node.get_class())
    {
    case XmlNodeClass::LIST:
        inspect_list_def(def_node);
        break;

    case XmlNodeClass::STRUCT:
        inspect_struct_def(def_node);
        break;

    case XmlNodeClass::TRIVIAL:
        inspect_trivial_def(def_node);
        break;

    case XmlNodeClass::LINK:
        inspect_link_def(def_node);
        break;      

    default:
        _ASSERT(false);
        throw std::logic_error("Unknown node class found");
    }
}

void inspect_definition(const XmlDefDoc& def_doc)
{
    auto def_root = def_doc.get_root_def();
    if (!def_root)
    {
        throw std::logic_error("Definition has no root element");
    }
    inspect_node_def(*def_root);
}

}} //namespace TR { namespace XML {