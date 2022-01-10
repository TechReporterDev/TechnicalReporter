#include "stdafx.h"
#include "libxml2_wrap.h"

namespace xml_tools {

struct xml_free_char
{
    void operator ()(xmlChar* p_char)
    {
        xmlFree(p_char);
    }
};

std::unique_ptr<XmlDoc> create_doc()
{
    std::unique_ptr<XmlDoc> doc((XmlDoc*)xmlNewDoc(BAD_CAST "1.0"));
    if(! doc)
    {
        throw std::runtime_error("Can`t create XML document.");
    }
    return doc;
}

std::unique_ptr<XmlDoc> clone_doc(const XmlDoc& doc, stl_tools::flag_type<XmlCloneOpt> opt)
{
    std::unique_ptr<XmlDoc> cloned(create_doc());
    if(opt.contains(XmlCloneOpt::PRIVATE))
    {
        cloned->_private = doc._private;
    }

    if (auto root = get_root(doc))
    {
        set_root(*cloned, clone_node(*root, opt));
    }
    return cloned;  
}

std::unique_ptr<XmlDoc> parse(const std::string& xml)
{
    std::unique_ptr<XmlDoc> doc((XmlDoc*)xmlParseMemory(&xml[0], xml.size()));
    if(! doc)
    {
        throw std::runtime_error("XML parse error");
    }
    return doc;
}

std::unique_ptr<XmlDoc> compose(std::vector<std::unique_ptr<XmlDoc>> parts, const char* root_name)
{
    auto doc = create_doc();
    auto& root = set_root(*doc, root_name);
    for (auto& part : parts)
    {
        if (auto part_root = get_root(*part))
        {
            add_node(root, remove_node(*part_root));
        }
    }
    return doc;
}

std::unique_ptr<XmlDoc> compose(std::unique_ptr<XmlDoc> part1, std::unique_ptr<XmlDoc> part2, const char* root_name)
{
    std::vector<std::unique_ptr<XmlDoc>> parts;
    if (part1)
    {
        parts.push_back(std::move(part1));
    }

    if (part2)
    {
        parts.push_back(std::move(part2));
    }
    return compose(std::move(parts), root_name);
}

std::string as_string(const XmlDoc& doc)
{
    xmlChar* str = nullptr;
    int size = 0;
    xmlDocDumpMemory(const_cast<XmlDoc*>(&doc), &str, &size);
    std::unique_ptr<xmlChar, xml_free_char> free_guard(str);
    return std::string(str, str+size);  
}

bool equal(const XmlDoc& left, const XmlDoc& right)
{
    return as_string(left) == as_string(right);
}

XmlNode* get_root(XmlDoc& doc)
{
    return (XmlNode*)xmlDocGetRootElement(&doc);
}

const XmlNode* get_root(const XmlDoc& doc)
{
    return (XmlNode*)xmlDocGetRootElement((XmlDoc*)&doc);
}

XmlNode* begin_child(XmlNode& node)
{
    return (XmlNode*)node.children;
}

const XmlNode* begin_child(const XmlNode& node)
{
    return (XmlNode*)node.children;
}

XmlNode* next_child(XmlNode& node)
{
    return (XmlNode*)node.next;
}

const XmlNode* next_child(const XmlNode& node)
{
    return (XmlNode*)node.next;
}

XmlNode* next_child(XmlNode* node)
{
    if (!node)
    {
        throw std::runtime_error("Can`t get sibling of NULL node");
    }
    return (XmlNode*)next_child(*node);
}

const XmlNode* next_child(const XmlNode* node)
{
    if (!node)
    {
        throw std::runtime_error("Can`t get sibling of NULL node");
    }
    return (XmlNode*)next_child(*node);
}

XmlNode* get_parent(XmlNode& node)
{
    if (node.parent == nullptr || node.parent->type != XML_ELEMENT_NODE)
    {
        return nullptr;
    }
    return (XmlNode*)node.parent;
}

const XmlNode* get_parent(const XmlNode& node)
{
    return get_parent(const_cast<XmlNode&>(node));
}

XmlNode* find_child(XmlNode& parent_node, const char* name)
{
    for (auto child_node = begin_child(parent_node); child_node != nullptr; child_node = next_child(child_node))
    {
        if (node_name_equal(*child_node, name))
        {
            return child_node;
        }
    }
    return nullptr;
}

const XmlNode* find_child(const XmlNode& parent_node, const char* name)
{
    return find_child(const_cast<XmlNode&>(parent_node), name);
}

XmlNode* find_sibling(XmlNode& prev_node, const char* name)
{
    for (auto sibling_node = next_child(prev_node); sibling_node != nullptr; sibling_node = next_child(sibling_node))
    {
        if (node_name_equal(*sibling_node, name))
        {
            return sibling_node;
        }
    }
    return nullptr;
}

const XmlNode* find_sibling(const XmlNode& prev_node, const char* name)
{
    return find_sibling(const_cast<XmlNode&>(prev_node), name);
}

XmlNode* begin_element(XmlNode& node)
{
    auto child_node = begin_child(node);
    for (; child_node; child_node = next_child(child_node))
    {
        if (get_node_type(*child_node) == XmlNodeType::ELEMENT)
        {
            break;
        }
    }
    return child_node;
}

const XmlNode* begin_element(const XmlNode& node)
{
    return begin_element(const_cast<XmlNode&>(node));
}

XmlNode* next_element(XmlNode& node)
{
    auto child_node = next_child(node);
    for (; child_node; child_node = next_child(child_node))
    {
        if (get_node_type(*child_node) == XmlNodeType::ELEMENT)
        {
            break;
        }
    }
    return child_node;
}

const XmlNode* next_element(const XmlNode& node)
{
    return next_element(const_cast<XmlNode&>(node));
}

struct xpath_free_context
{
    void operator ()(xmlXPathContextPtr context)
    {
        xmlXPathFreeContext(context);
    }
};

struct xpath_free_object
{
    void operator ()(xmlXPathObjectPtr object)
    {
        xmlXPathFreeObject(object);
    }
};

std::vector<XmlNode*> find_xpath_node_set(XmlDoc& doc, XmlNode* ctx_node, const char* xpath)
{
    std::unique_ptr<xmlXPathContext, xpath_free_context> context(xmlXPathNewContext(&doc));
    if (!context)
    {
        throw std::exception("Can`t create xpath context");
    }

    if (ctx_node)
    {
        context->node = ctx_node;
    }
    
    std::unique_ptr<xmlXPathObject, xpath_free_object> object(xmlXPathEval((xmlChar*)xpath, context.get()));
    if (!object)
    {
        throw std::exception("Can`t get xpath object");
    }

    std::vector<XmlNode*> result;
    if (xmlXPathNodeSetIsEmpty(object->nodesetval))
    {
        return result;
    }

    xmlNodeSetPtr nodeset = object->nodesetval;
    for (int i = 0; i < nodeset->nodeNr; i++)
    {
        result.push_back((XmlNode*)nodeset->nodeTab[i]);
    }
    return result;
}

std::vector<XmlNode*> find_xpath_node_set(XmlDoc& doc, const char* xpath)
{
    return find_xpath_node_set(doc, nullptr, xpath);
}

XmlNode* find_xpath_node(XmlDoc& doc, XmlNode* ctx_node, const char* xpath)
{
    std::unique_ptr<xmlXPathContext, xpath_free_context> context(xmlXPathNewContext(&doc));
    if (!context)
    {
        throw std::exception("Can`t create xpath context");
    }

    if (ctx_node)
    {
        context->node = ctx_node;
    }

    std::unique_ptr<xmlXPathObject, xpath_free_object> object(xmlXPathEvalExpression((xmlChar*)xpath, context.get()));
    if (!object)
    {
        throw std::exception("Can`t get xpath object");
    }

    if (!xmlXPathNodeSetIsEmpty(object->nodesetval))
    {
        xmlNodeSetPtr nodeset = object->nodesetval;
        return (XmlNode*)nodeset->nodeTab[0];
    }   

    return nullptr;
}

XmlNode* find_xpath_node(XmlDoc& doc, const char* xpath)
{
    return find_xpath_node(doc, nullptr, xpath);
}

const XmlNode* find_xpath_node(const XmlDoc& doc, const char* xpath)
{
    return find_xpath_node(const_cast<XmlDoc&>(doc), xpath);
}

std::string make_xpath(const XmlNode& node)
{
    std::string xpath;
    if (auto parent = get_parent(node))
    {
        xpath = make_xpath(*parent);
    }
    xpath.push_back(L'/');
    xpath.append(get_node_name(node));
    return xpath;
}

XmlNode& set_root(XmlDoc& doc, const char* name)
{
    _ASSERT(name);
    auto new_root((XmlNode*)xmlNewNode(nullptr, BAD_CAST name));
    if(! new_root)
    {
        throw std::runtime_error("Can`t create root element");
    }

    std::unique_ptr<XmlNode> old_root((XmlNode*)xmlDocSetRootElement(&doc, new_root));
    return *new_root;
}

XmlNode& add_element(XmlNode& parent, const char* name)
{
    _ASSERT(name);
    auto element = (XmlNode*)xmlAddChild(&parent, xmlNewNode(nullptr, BAD_CAST name));
    if (!element)
    {
        throw std::runtime_error("Can`t add element");
    }
    return *element;
}

XmlNode& add_element(XmlNode& parent, const wchar_t* name)
{
    _ASSERT(name);
    return add_element(parent, stl_tools::ucs_to_utf8(name).c_str());   
}

XmlNode& add_text_node(XmlNode& parent, const char* text)
{   
    _ASSERT(text);
    auto text_node = (XmlNode*)xmlAddChild(&parent, xmlNewText(BAD_CAST text));
    if (!text_node)
    {
        throw std::runtime_error("Can`t add text node");
    }
    return *text_node;
}

XmlNode& add_text_node(XmlNode& parent, const wchar_t* text)
{
    _ASSERT(text);
    return add_text_node(parent, stl_tools::ucs_to_utf8(text).c_str()); 
}

xmlAttr& set_attr_value(XmlNode& parent, const char* attr_name, const char* attr_value)
{
    _ASSERT(attr_name && attr_value);
    auto attr = ::xmlSetProp(&parent, BAD_CAST attr_name, BAD_CAST attr_value);
    if (! attr)
    {
        throw std::runtime_error("Can`t set XML attribute value");
    }
    return *attr;
}

xmlAttr& set_attr_value(XmlNode& parent, const wchar_t* attr_name, const wchar_t* attr_value)
{
    _ASSERT(attr_name && attr_value);
    return set_attr_value(parent, stl_tools::ucs_to_utf8(attr_name).c_str(), stl_tools::ucs_to_utf8(attr_value).c_str());
}

void unset_attr(XmlNode& parent, const char* attr_name)
{
    xmlUnsetProp(&parent, BAD_CAST attr_name);
}

std::unique_ptr<XmlNode> create_element(const char* name)
{
    std::unique_ptr<XmlNode> element((XmlNode*)xmlNewNode(nullptr, BAD_CAST name));
    if (!element)
    {
        throw std::runtime_error("Can`t create element");
    }
    return element;
}

XmlNode& set_root(XmlDoc& doc, std::unique_ptr<XmlNode> new_root)
{
    std::unique_ptr<XmlNode> old_root((XmlNode*)xmlDocSetRootElement(&doc, new_root.get()));
    return *new_root.release();
}

XmlNode& add_node(XmlNode& parent, std::unique_ptr<XmlNode> node)
{
    _ASSERT(node);
    auto new_node = (XmlNode*)xmlAddChild(&parent, node.release());
    if (!new_node)
    {
        throw std::runtime_error("Can`t add element");
    }
    return *new_node;
}

XmlNode& push_node_front(XmlNode& parent, std::unique_ptr<XmlNode> node)
{
    if (auto first_child = begin_child(parent))
    {
        return insert_node(*first_child, std::move(node));
    }

    return add_node(parent, std::move(node));       
}

XmlNode& insert_node(XmlNode& sibling, std::unique_ptr<XmlNode> node)
{
    _ASSERT(node);
    auto new_node = (XmlNode*)xmlAddPrevSibling(&sibling, node.release());
    if (!new_node)
    {
        throw std::runtime_error("Can`t insert element");
    }
    return *new_node;
}

XmlNode& insert_after(XmlNode& sibling, std::unique_ptr<XmlNode> node)
{
    _ASSERT(node);
    auto new_node = (XmlNode*)xmlAddNextSibling(&sibling, node.release());
    if (!new_node)
    {
        throw std::runtime_error("Can`t insert element");
    }
    return *new_node;
}

std::unique_ptr<XmlNode> replace_node(XmlNode& cur_node, std::unique_ptr<XmlNode> new_node)
{
    _ASSERT(new_node);
    std::unique_ptr<XmlNode> old_node((XmlNode*)xmlReplaceNode(&cur_node, new_node.release()));
    if (!old_node)
    {
        throw std::runtime_error("Can`t replace element");
    }
    return old_node;
}

void swap_nodes(XmlNode& left_node, XmlNode& right_node)
{
    xmlNode tmp_node = {};
    xmlReplaceNode(&right_node, &tmp_node);
    xmlReplaceNode(&left_node, &right_node);
    xmlReplaceNode(&tmp_node, &left_node);  
}

std::unique_ptr<XmlNode> remove_node(XmlNode& node)
{
    xmlUnlinkNode(&node);
    return std::unique_ptr<XmlNode>(&node);
}

void remove_children(xml_tools::XmlNode& parent)
{
    while (auto child_node = xml_tools::begin_child(parent))
    {
        remove_node(*child_node);
    }
}

xmlNsPtr search_ns(const XmlNode& node, const char* prefix)
{
    auto ns_def = node.nsDef;
    while (ns_def)
    {
        if (xmlStrcmp(ns_def->prefix, BAD_CAST prefix) == 0)
        {
            return ns_def;
        }
        ns_def = ns_def->next;
    }
    
    if (auto parent = get_parent(node))
    {
        return search_ns(*parent, prefix);
    }

    return nullptr;
}

struct clone_context
{   
    xmlNsPtr search_ns(const char* prefix) const
    {
        if (auto ns = xml_tools::search_ns(*m_node, prefix))
        {
            return ns;
        }

        if (m_parent_context)
        {
            return m_parent_context->search_ns(prefix);
        }

        return nullptr;
    }

    const clone_context* m_parent_context;
    XmlNode* m_node;
};

std::unique_ptr<XmlNode> clone_node(const XmlNode& node, stl_tools::flag_type<XmlCloneOpt> opt, const clone_context* context)
{
    std::unique_ptr<XmlNode> new_node((XmlNode*)xmlCopyNode(const_cast<XmlNode*>(&node), 0));
    if (!new_node)
    {
        throw std::runtime_error("Can`t clone node");
    }

    clone_context new_context = {context, new_node.get()};

    if (opt.contains(XmlCloneOpt::PRIVATE))
    {
        new_node->_private = node._private;
    }

    if (opt.contains(XmlCloneOpt::ATTRIBUTE))
    {
        if (auto attr = begin_attribute(node))
        {
            new_node->properties = ::xmlCopyPropList(new_node.get(), const_cast<xmlAttr*>(attr));
        }       
    }

    if (opt.contains(XmlCloneOpt::NAMESPACE))
    {
        if (node.nsDef)
        {
            new_node->nsDef = xmlCopyNamespaceList(node.nsDef);         
        }

        if (node.ns)
        {
            new_node->ns = new_context.search_ns((const char*)node.ns->prefix);
        }
    }
    
    for (auto child_node = begin_child(node); child_node != nullptr; child_node = next_child(*child_node))
    {
        bool clonable = false;
        switch (get_node_type(*child_node))
        {
        case XmlNodeType::ELEMENT:
            clonable = opt.contains(XmlCloneOpt::ELEMENT);
            break;

        case XmlNodeType::TEXT:
            clonable = opt.contains(XmlCloneOpt::TEXT);
            break;

        case XmlNodeType::COMMENT:
            clonable = opt.contains(XmlCloneOpt::COMMENT);
            break;

        default:
            _ASSERT(false);
        }
        if (clonable)
        {
            auto cloned = clone_node(*child_node, opt, &new_context);
            add_node(*new_node, std::move(cloned));         
        }
    }
    return new_node;
}

std::unique_ptr<XmlNode> clone_node(const XmlNode& node, stl_tools::flag_type<XmlCloneOpt> opt)
{
    return clone_node(node, opt, nullptr);
}

XmlNode& clone_into(const XmlNode& node, XmlNode& parent, stl_tools::flag_type<XmlCloneOpt> opt)
{
    clone_context context = { nullptr, &parent };
    return add_node(parent, clone_node(node, opt, &context));
}

std::unique_ptr<XmlNode> create_text_node(const char* text)
{
    _ASSERT(text);
    std::unique_ptr<XmlNode> text_node((XmlNode*)xmlNewText(BAD_CAST text));
    if (!text_node)
    {
        throw std::runtime_error("Can`t create text node");
    }
    return text_node;
}

XmlNodeType get_node_type(const XmlNode& node)
{
    XmlNodeType node_type = XmlNodeType::NONE;
    switch(node.type)
    {
    case XML_ELEMENT_NODE:
        node_type = XmlNodeType::ELEMENT;
        break;

    case XML_TEXT_NODE:
        node_type = XmlNodeType::TEXT;
        break;

    case XML_COMMENT_NODE:
        node_type = XmlNodeType::COMMENT;
        break;

    default:
        _ASSERT(false);
    }
    return node_type;
}

std::string get_node_name(const XmlNode& node)
{
    return (const char*)node.name;
}

std::wstring get_node_wname(const XmlNode& node)
{
    return stl_tools::utf8_to_ucs((const char*)node.name);
}

const char* get_node_content(const XmlNode& node)
{
    _ASSERT(node.content);
    return (const char*)node.content;
}

void set_node_content(XmlNode& node, const char* content)
{
    _ASSERT(node.type == XML_TEXT_NODE || node.type == XML_COMMENT_NODE);
    xmlNodeSetContent(&node, BAD_CAST content);
}

std::string get_node_text(const XmlNode& node)
{
    std::string text;
    auto child = node.children;
    while (child)
    {
        if(child->type == XML_TEXT_NODE)
        {
            text.append((const char*) child->content);
        }
        child = child->next;
    }
    return text;
}

std::wstring get_node_wtext(const XmlNode& node)
{
    return stl_tools::utf8_to_ucs(get_node_text(node));
}

xmlAttr* begin_attribute(XmlNode& node)
{
    return node.properties;
}

const xmlAttr* begin_attribute(const XmlNode& node)
{
    return node.properties;
}

xmlAttr* next_attribute(xmlAttr& attr)
{
    return attr.next;
}

const xmlAttr* next_attribute(const xmlAttr& attr)
{
    return attr.next;
}

xmlAttr* find_attribute(XmlNode& node, const char* name)
{
    return const_cast<xmlAttr*>(find_attribute(const_cast<const XmlNode&>(node), name));
}

const xmlAttr* find_attribute(const XmlNode& node, const char* name)
{
    _ASSERT(name);
    xmlAttr* attr = node.properties;
    while (attr)
    {
        if(attr_name_equal(*attr, name))
        {
            return attr;
        }
        attr = attr->next;
    }
    return nullptr;
}

xmlAttr* find_attribute(XmlNode& node, const char* ns, const char* name)
{
    return const_cast<xmlAttr*>(find_attribute(const_cast<const XmlNode&>(node), ns, name));
}

const xmlAttr* find_attribute(const XmlNode& node, const char* ns, const char* name)
{
    _ASSERT(ns && name);
    xmlAttr* attr = node.properties;
    while (attr)
    {
        if (attr_ns_equal(*attr, ns) && attr_name_equal(*attr, name))
        {
            return attr;
        }
        attr = attr->next;
    }
    return nullptr;
}

std::string get_attr_name(const xmlAttr& attr)
{
    return (const char*)attr.name;  
}

std::wstring get_attr_wname(const xmlAttr& attr)
{
    return stl_tools::utf8_to_ucs((const char*)attr.name);  
}

std::string get_attr_value(const xmlAttr& attr)
{
    _ASSERT(attr.children && attr.children->content);
    return (const char*)attr.children->content;
}

std::wstring get_attr_wvalue(const xmlAttr& attr)
{
    _ASSERT(attr.children && attr.children->content);
    return stl_tools::utf8_to_ucs((const char*)attr.children->content);
}

std::string get_attr_value(const XmlNode& node, const char* name, const char* default_value)
{
    if (auto attr = find_attribute(node, name))
    {
        return get_attr_value(*attr);
    }
    return default_value;
}

std::wstring get_attr_wvalue(const XmlNode& node, const char* name, const wchar_t* default_value)
{
    if (auto attr = find_attribute(node, name))
    {
        return get_attr_wvalue(*attr);
    }
    return default_value;
}

void set_attr_value(xmlAttr& attr, const char* value)
{
    _ASSERT(attr.children);
    ::xmlNodeSetContent(attr.children, BAD_CAST value);
}

void set_attr_value(xmlAttr& attr, const wchar_t* value)
{
    return set_attr_value(attr, stl_tools::ucs_to_utf8(value).c_str());
}

bool node_content_equal(const XmlNode& node, const char* content)
{
    _ASSERT(node.content);
    _ASSERT(content);   
    return xmlStrcmp(node.content, BAD_CAST content) == 0;
}

bool node_name_equal(const XmlNode& left, const XmlNode& right)
{
    return xmlStrcmp(left.name, right.name) == 0;
}

bool node_name_equal(const XmlNode& node, const char* name)
{
    return xmlStrcmp(node.name, BAD_CAST name) == 0;
}

bool node_text_equal(const XmlNode& node, const char* text)
{
    _ASSERT(node.type == XML_ELEMENT_NODE);
    _ASSERT(text);
    const char* cursor = text;
    auto child = node.children;
    while (child)
    {
        if (child->type == XML_TEXT_NODE)
        {
            const char* content = (const char*) child->content;
            while (*content)
            {
                if (*content++ != *cursor++)
                    return false;
            }
        }
        child = child->next;
    }
    return *cursor == 0;
}

bool node_text_equal(const XmlNode& node, const wchar_t* text)
{       
    return node_text_equal(node, stl_tools::ucs_to_utf8(text).c_str());
}

bool attr_name_equal(const xmlAttr& attr, const char* name)
{
    _ASSERT(name);
    return xmlStrcmp(attr.name, BAD_CAST name) == 0;
}

bool attr_ns_equal(const xmlAttr& attr, const char* ns)
{
    _ASSERT(ns);    
    if (!attr.ns)
    {
        return false;
    }

    return xmlStrcmp(attr.ns->prefix, BAD_CAST ns) == 0;
}

bool attr_value_equal(const xmlAttr& attr, const char* value)
{
    _ASSERT(value);
    _ASSERT(attr.children && attr.children->content);
    return xmlStrcmp(attr.children->content, BAD_CAST value) == 0;
}

bool is_descendant(const XmlNode& ancestor, const XmlNode& descendant)
{
    if (!descendant.parent)
    {
        return false;
    }

    if (descendant.parent == &ancestor)
    {
        return true;
    }

    return is_descendant(ancestor, static_cast<const XmlNode&>(*descendant.parent));
}

} //namespace xml_tools {