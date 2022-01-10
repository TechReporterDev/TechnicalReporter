#pragma once
#include "TRXMLStd.h"
namespace TR { namespace XML {

struct XmlDefNode;
enum class XmlNodeClass { LIST = 0x01, STRUCT = 0x02, TRIVIAL = 0x04, LINK = 0xFF, NONE = 0x00 };
enum class XmlTrivialType {STRING = 0x01, BOOLEAN = 0x02, INTEGER = 0x04, ENUM = 0x08, NONE = 0x00};
using XmlNodeValue = boost::variant<bool, int, std::wstring>;

struct XmlDefDoc: XmlDoc
{
    XmlDefNode*         get_root_def();
    const XmlDefNode*   get_root_def() const;
};

struct XmlMetaNode: XmlNode
{
    const XmlDefNode*   get_owner_node() const;
};

struct XmlDefNode: XmlNode
{
    //default values
    static std::wstring         get_default_caption();
    static std::wstring         get_default_description();
        
    static const XmlMetaNode* next_meta_node(const XmlMetaNode* meta_node);
    using ConstMetaNodeIterator = xml_tools::node_iterator<const XmlMetaNode, next_meta_node>;
    using ConstMetaNodeIteratorRange = boost::iterator_range<ConstMetaNodeIterator>;

    // properties
    XmlNodeClass                get_class() const;
    bool                        is_trivial() const;
    bool                        is_link() const;
    bool                        is_optional() const;
    void                        set_optional(bool value);
    std::wstring                get_caption() const;
    std::wstring                format_caption() const;
    std::wstring                get_description() const;
    std::string                 get_xpath() const;
    const XmlDefNode&           resolve() const;
    
    XmlDefNode*                 get_parent_def();
    const XmlDefNode*           get_parent_def() const;

    ConstMetaNodeIterator       begin_meta_node() const;
    ConstMetaNodeIterator       end_meta_node() const;
    ConstMetaNodeIteratorRange  get_meta_nodes() const;
};

struct XmlTrivialDef: XmlDefNode
{
    static std::wstring         get_default_string(XmlTrivialType trivial_type);
    
    XmlTrivialType              get_type() const;
    std::wstring                get_default_string() const; 
};

struct XmlBooleanDef: XmlTrivialDef
{
};

struct XmlIntegerDef: XmlTrivialDef
{
};

struct XmlStringDef: XmlTrivialDef
{
};

struct XmlEnumDef: XmlTrivialDef
{
    std::vector<std::wstring> get_names() const;
};

struct XmlLinkDef: XmlDefNode
{
};

struct XmlStructDef: XmlDefNode
{
    static XmlDefNode* next_member_def(XmlDefNode* def_node);
    using MemberDefIterator = xml_tools::node_iterator<XmlDefNode, next_member_def>;
    
    static const XmlDefNode* next_member_def(const XmlDefNode* def_node);
    using ConstMemberDefIterator = xml_tools::node_iterator<const XmlDefNode, next_member_def>;
    
    using MemberDefRange = boost::iterator_range<MemberDefIterator>;
    using ConstMemberDefRange = boost::iterator_range<ConstMemberDefIterator>;

    MemberDefIterator           begin_member_def();
    ConstMemberDefIterator      begin_member_def() const;
    MemberDefIterator           end_member_def();
    ConstMemberDefIterator      end_member_def() const;
    
    MemberDefRange              get_member_defs();
    ConstMemberDefRange         get_member_defs() const;
};

struct XmlListDef: XmlDefNode
{
    XmlDefNode& get_item_def();
    const XmlDefNode& get_item_def() const;
};

// navigation
using ConstXmlDefNodeRange = boost::any_range<const XmlDefNode, boost::forward_traversal_tag>;
ConstXmlDefNodeRange get_child_defs(const XmlDefNode& parent);

template<class Visitor> 
typename Visitor::result_type visit_def_node(Visitor visitor, const XmlDefNode& def_node);

template<class DefNodeType>
const DefNodeType& cast_def_node(const XmlDefNode& def_node);

template<class DefNodeType>
const DefNodeType* cast_def_node(const XmlDefNode* def_node);

// doc function
std::unique_ptr<XmlDefDoc>      parse_definition(const std::string& xml_def);
void                            inspect_definition(const XmlDefDoc& def_doc);

///////////////
// Implementation
///////////////

template<class Visitor> 
typename Visitor::result_type visit_def_node(Visitor visitor, const XmlDefNode& def_node)
{
    switch (def_node.get_class())
    {
    case XmlNodeClass::TRIVIAL:
        switch (static_cast<const XmlTrivialDef&>(def_node).get_type())
        {
            case XmlTrivialType::BOOLEAN:
                return visitor(static_cast<const XmlBooleanDef&>(def_node));

            case XmlTrivialType::INTEGER:
                return visitor(static_cast<const XmlIntegerDef&>(def_node));

            case XmlTrivialType::STRING:
                return visitor(static_cast<const XmlStringDef&>(def_node));

            case XmlTrivialType::ENUM:
                return visitor(static_cast<const XmlEnumDef&>(def_node));
            
            default:
                _ASSERT(false);
                throw std::logic_error("Unknown trivial type found");
        }
            
    case XmlNodeClass::LINK:
        return visitor(static_cast<const XmlLinkDef&>(def_node));
        
    case XmlNodeClass::STRUCT:
        return visitor(static_cast<const XmlStructDef&>(def_node));

    case XmlNodeClass::LIST:
        return visitor(static_cast<const XmlListDef&>(def_node));

    default:
        _ASSERT(false);
        throw std::logic_error("Unknown node class found");
    }
}

template<class DefNodeType>
struct CastDefNodeRef: boost::static_visitor<const DefNodeType&>
{
    const DefNodeType& operator()(const DefNodeType& def_node) const
    {
        return def_node;
    }

    template<class T>
    const DefNodeType& operator()(T& def_node) const
    {
        throw std::logic_error("Bad cast"); 
    }
};

template<class DefNodeType>
const DefNodeType& cast_def_node(const XmlDefNode& def_node)
{
    return visit_def_node(CastDefNodeRef<DefNodeType>(), def_node);
}

template<class DefNodeType>
struct CastDefNodePtr: boost::static_visitor<const DefNodeType*>
{
    const DefNodeType* operator()(const DefNodeType& def_node) const
    {
        return &def_node;
    }

    template<class T>
    const DefNodeType* operator()(T& def_node) const
    {
        return nullptr;
    }
};

template<class DefNodeType>
const DefNodeType* cast_def_node(const XmlDefNode* def_node)
{
    if (!def_node)
    {
        return nullptr;
    }

    return visit_def_node(CastDefNodePtr<DefNodeType>(), *def_node);
}

}} //namespace TR { namespace XML {