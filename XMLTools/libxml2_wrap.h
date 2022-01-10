#pragma once
#include <memory>
#include <string>
#include <vector>
#include <libxml\tree.h>
#include "STLTools\stl_tools.h"
namespace xml_tools {

struct XmlDoc: xmlDoc
{
    XmlDoc() = delete;
    XmlDoc(const XmlDoc&) = delete;
    XmlDoc(XmlDoc&&) = delete;
    XmlDoc& operator = (const XmlDoc&) = delete;
    XmlDoc& operator = (XmlDoc&&) = delete;
    
    static void operator delete(void* ptr, std::size_t size)
    {
        xmlFreeDoc((XmlDoc*)ptr);
    }
};

struct XmlNode: xmlNode
{
    XmlNode() = delete;
    XmlNode(const XmlNode&) = delete;
    XmlNode(XmlNode&&) = delete;
    XmlNode& operator = (const XmlNode&) = delete;
    XmlNode& operator = (XmlNode&&) = delete;

    static void operator delete(void* ptr, std::size_t size)
    {
        xmlFreeNode((XmlNode*)ptr);
    }
};


enum class XmlNodeType { NONE = 0x00, ELEMENT = 0x01, TEXT = 0x02, COMMENT = 0x04 };
enum class XmlCloneOpt { NONE = 0x00, ELEMENT = 0x01, TEXT = 0x02, COMMENT = 0x04, ATTRIBUTE = 0x08, NAMESPACE = 0x10, PRIVATE = 0x20, ALL = 0xFF};

// doc functions
std::unique_ptr<XmlDoc>     create_doc();
std::unique_ptr<XmlDoc>     clone_doc(const XmlDoc& doc, stl_tools::flag_type<XmlCloneOpt> opt = XmlCloneOpt::ALL);
std::unique_ptr<XmlDoc>     parse(const std::string& xml);
std::unique_ptr<XmlDoc>     compose(std::vector<std::unique_ptr<XmlDoc>> parts, const char* root_name);
std::unique_ptr<XmlDoc>     compose(std::unique_ptr<XmlDoc> part1, std::unique_ptr<XmlDoc> part2, const char* root_name);

std::string                 as_string(const XmlDoc& doc);
bool                        equal(const XmlDoc& left, const XmlDoc& right);

// navigating
XmlNode*                    get_root(XmlDoc& doc);
const XmlNode*              get_root(const XmlDoc& doc);
XmlNode*                    begin_child(XmlNode& node);
const XmlNode*              begin_child(const XmlNode& node);
XmlNode*                    next_child(XmlNode& node);
const XmlNode*              next_child(const XmlNode& node);
XmlNode*                    next_child(XmlNode* node);
const XmlNode*              next_child(const XmlNode* node);
XmlNode*                    get_parent(XmlNode& node);
const XmlNode*              get_parent(const XmlNode& node);
XmlNode*                    find_child(XmlNode& parent_node, const char* name);
const XmlNode*              find_child(const XmlNode& parent_node, const char* name);
XmlNode*                    find_sibling(XmlNode& prev_node, const char* name);
const XmlNode*              find_sibling(const XmlNode& prev_node, const char* name);
XmlNode*                    begin_element(XmlNode& node);
const XmlNode*              begin_element(const XmlNode& node);
XmlNode*                    next_element(XmlNode& node);
const XmlNode*              next_element(const XmlNode& node);

//xpath
std::vector<XmlNode*>       find_xpath_node_set(XmlDoc& doc, XmlNode* ctx_node, const char* xpath);
std::vector<XmlNode*>       find_xpath_node_set(XmlDoc& doc, const char* xpath);
XmlNode*                    find_xpath_node(XmlDoc& doc, XmlNode* ctx_node, const char* xpath);
XmlNode*                    find_xpath_node(XmlDoc& doc, const char* xpath);
const XmlNode*              find_xpath_node(const XmlDoc& doc, const char* xpath);
std::string                 make_xpath(const XmlNode& node);

// basic building
XmlNode&                    set_root(XmlDoc& doc, const char* name);
XmlNode&                    add_element(XmlNode& parent, const char* name);
XmlNode&                    add_element(XmlNode& parent, const wchar_t* name);
XmlNode&                    add_text_node(XmlNode& parent, const char* text);
XmlNode&                    add_text_node(XmlNode& parent, const wchar_t* text);
xmlAttr&                    set_attr_value(XmlNode& parent, const char* attr_name, const char* attr_value);
xmlAttr&                    set_attr_value(XmlNode& parent, const wchar_t* attr_name, const wchar_t* attr_value);
void                        unset_attr(XmlNode& parent, const char* attr_name);

// advanced building
std::unique_ptr<XmlNode>    create_element(const char* name);
XmlNode&                    set_root(XmlDoc& doc, std::unique_ptr<XmlNode> new_root);
XmlNode&                    add_node(XmlNode& parent, std::unique_ptr<XmlNode> node);
XmlNode&                    push_node_front(XmlNode& parent, std::unique_ptr<XmlNode> node);
XmlNode&                    insert_node(XmlNode& sibling, std::unique_ptr<XmlNode> node);
XmlNode&                    insert_after(XmlNode& sibling, std::unique_ptr<XmlNode> node);
std::unique_ptr<XmlNode>    replace_node(XmlNode& old_node, std::unique_ptr<XmlNode> new_node);
void                        swap_nodes(XmlNode& left_node, XmlNode& right_node);
std::unique_ptr<XmlNode>    remove_node(XmlNode& node);
void                        remove_children(XmlNode& parent);
std::unique_ptr<XmlNode>    clone_node(const XmlNode& node, stl_tools::flag_type<XmlCloneOpt> opt = XmlCloneOpt::ALL);
XmlNode&                    clone_into(const XmlNode& node, XmlNode& parent, stl_tools::flag_type<XmlCloneOpt> opt = XmlCloneOpt::ALL);
std::unique_ptr<XmlNode>    create_text_node(const char* text);

// node props
XmlNodeType                 get_node_type(const XmlNode& node);
std::string                 get_node_name(const XmlNode& node);
std::wstring                get_node_wname(const XmlNode& node);
const char*                 get_node_content(const XmlNode& node); //for text and comment nodes
void                        set_node_content(XmlNode& node, const char* content); //for text and comment nodes
std::string                 get_node_text(const XmlNode& node); //for element node
std::wstring                get_node_wtext(const XmlNode& node); //for element node

// attribute navigeting
xmlAttr*                    begin_attribute(XmlNode& node);
const xmlAttr*              begin_attribute(const XmlNode& node);
xmlAttr*                    next_attribute(xmlAttr& attr);
const xmlAttr*              next_attribute(const xmlAttr& attr);
xmlAttr*                    find_attribute(XmlNode& node, const char* name);
const xmlAttr*              find_attribute(const XmlNode& node, const char* name);
xmlAttr*                    find_attribute(XmlNode& node, const char* ns, const char* name);
const xmlAttr*              find_attribute(const XmlNode& node, const char* ns, const char* name);

// attribute props
std::string                 get_attr_name(const xmlAttr& attr);
std::wstring                get_attr_wname(const xmlAttr& attr);
std::string                 get_attr_value(const xmlAttr& attr);
std::wstring                get_attr_wvalue(const xmlAttr& attr);
std::string                 get_attr_value(const XmlNode& node, const char* name, const char* default_value);
std::wstring                get_attr_wvalue(const XmlNode& node, const char* name, const wchar_t* default_value);
void                        set_attr_value(xmlAttr& attr, const char* value);
void                        set_attr_value(xmlAttr& attr, const wchar_t* value);

// effective checks
bool                        node_content_equal(const XmlNode& node, const char* content); //for text and comment nodes
bool                        node_name_equal(const XmlNode& left, const XmlNode& right);
bool                        node_name_equal(const XmlNode& node, const char* name);
bool                        node_text_equal(const XmlNode& node, const char* text);
bool                        node_text_equal(const XmlNode& node, const wchar_t* text);
bool                        attr_name_equal(const xmlAttr& attr, const char* name);
bool                        attr_ns_equal(const xmlAttr& attr, const char* ns);
bool                        attr_value_equal(const xmlAttr& attr, const char* value);
bool                        is_descendant(const XmlNode& ancestor, const XmlNode& descendant);

// utils
template<class NodeType>
NodeType& add_node(XmlNode& parent, std::unique_ptr<NodeType> node)
{
    static_assert(sizeof(NodeType) == sizeof(XmlNode), "Node type must contains no members");
    return static_cast<NodeType&>(add_node(parent, std::unique_ptr<XmlNode>(std::move(node))));
}

template<class NodeType>
NodeType& push_node_front(XmlNode& parent, std::unique_ptr<NodeType> node)
{
    static_assert(sizeof(NodeType) == sizeof(XmlNode), "Node type must contains no members");
    return static_cast<NodeType&>(push_node_front(parent, std::unique_ptr<XmlNode>(std::move(node))));
}

template<class NodeType>
NodeType& insert_node(XmlNode& sibling, std::unique_ptr<NodeType> node)
{
    static_assert(sizeof(NodeType) == sizeof(XmlNode), "Node type must contains no members");
    return static_cast<NodeType&>(insert_node(sibling, std::unique_ptr<XmlNode>(std::move(node))));
}

template<class NodeType>
NodeType& insert_after(XmlNode& sibling, std::unique_ptr<NodeType> node)
{
    static_assert(sizeof(NodeType) == sizeof(XmlNode), "Node type must contains no members");
    return static_cast<NodeType&>(insert_after(sibling, std::unique_ptr<XmlNode>(std::move(node))));
}

template<class NodeType>
std::unique_ptr<NodeType> clone_node(const NodeType& node)
{
    static_assert(sizeof(NodeType) == sizeof(XmlNode), "Node type must contains no members");
    auto clone = clone_node(static_cast<const XmlNode&>(node));
    return stl_tools::static_pointer_cast<NodeType>(std::move(clone));
}

template<class NodeType>
std::unique_ptr<NodeType> remove_node(NodeType& node)
{
    return stl_tools::static_pointer_cast<NodeType>(remove_node(static_cast<XmlNode&>(node)));
}

template<class DocType>
std::unique_ptr<DocType> clone_doc(const DocType& doc)
{
    static_assert(sizeof(DocType) == sizeof(XmlDoc), "DocType must contains no members");
    auto clone = clone_doc(static_cast<const XmlDoc&>(doc));
    return std::unique_ptr<DocType>(static_cast<DocType*>(clone.release()));
}

template<class DocType>
std::unique_ptr<XmlDoc> compose(std::vector<std::unique_ptr<DocType>> _parts, const char* root_name)
{
    std::vector<std::unique_ptr<XmlDoc>> parts;
    for (auto& part : _parts)
    {
        parts.push_back(std::move(part));
    }
    return compose(std::move(parts), root_name);
}

} //namespace xml_tools {