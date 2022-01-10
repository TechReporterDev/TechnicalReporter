#pragma once
#include "XMLDefinition.h"
#include "TRXMLStd.h"
namespace TR { namespace XML {

struct XmlRegularNode;
struct XmlRegularDoc: XmlDoc
{
    const XmlDefDoc&        get_bound_def() const;
    XmlRegularNode*         get_root();
    const XmlRegularNode*   get_root() const;
};

struct XmlRegularNode: XmlNode
{   
    const XmlDefNode&       get_bound_def() const;
    XmlRegularNode*         get_parent();
    const XmlRegularNode*   get_parent() const;
};

struct XmlGenericTrivialNode: XmlRegularNode
{
    const XmlTrivialDef&    get_bound_def() const;
    std::string             get_text() const;
    std::wstring            get_wtext() const;
};

template<class T> //T can be bool, int or std::wstring
struct XmlTrivialNode: XmlGenericTrivialNode
{
    using TrivialType = T;
    T                       get_value() const;
    void                    set_value(const T& value);
};

struct XmlBooleanNode: XmlTrivialNode<bool>
{
    const XmlBooleanDef& get_bound_def() const;
};

struct XmlIntegerNode: XmlTrivialNode<int>
{
    const XmlIntegerDef& get_bound_def() const;
};

struct XmlStringNode: XmlTrivialNode<std::wstring>
{
    const XmlStringDef& get_bound_def() const;
};

struct XmlEnumNode: XmlTrivialNode<int>
{
    const XmlEnumDef& get_bound_def() const;
};

struct XmlStructNode: XmlRegularNode
{
    static XmlRegularNode* next_member(XmlRegularNode* member_node);
    static const XmlRegularNode* next_member(const XmlRegularNode* member_node);

    using MemberIterator = xml_tools::node_iterator<XmlRegularNode, next_member>;
    using ConstMemberIterator = xml_tools::node_iterator<const XmlRegularNode, next_member>;

    using MemberIteratorRange = boost::iterator_range<MemberIterator>;
    using ConstMemberIteratorRange = boost::iterator_range<ConstMemberIterator>;

    const XmlStructDef&             get_bound_def() const;
    xmlAttr*                        find_id_attr();
    const xmlAttr*                  find_id_attr() const;

    MemberIterator                  begin_member();
    MemberIterator                  end_member();
    MemberIteratorRange             get_members();
    
    ConstMemberIterator             begin_member() const;
    ConstMemberIterator             end_member() const; 
    ConstMemberIteratorRange        get_members() const;
};

struct XmlGenericListNode: XmlRegularNode
{
    static XmlRegularNode* next_item(XmlRegularNode* item_node);
    static const XmlRegularNode* next_item(const XmlRegularNode* item_node);
    
    using ItemIterator = xml_tools::node_iterator<XmlRegularNode, next_item>;
    using ConstItemIterator = xml_tools::node_iterator<const XmlRegularNode, next_item>;
    using ItemIteratorRange = boost::iterator_range<ItemIterator>;
    using ConstItemIteratorRange = boost::iterator_range<ConstItemIterator>;

    const XmlListDef&               get_bound_def() const;
    bool                            empty() const;
    
    std::unique_ptr<XmlRegularNode> create_item() const;
    ItemIterator                    insert_item(ItemIterator _where, std::unique_ptr<XmlRegularNode> item = nullptr);
    void                            remove_item(ItemIterator _where);
    
    ItemIterator                    begin_item();
    ItemIterator                    end_item();
    ItemIteratorRange               get_items();

    ConstItemIterator               begin_item() const;
    ConstItemIterator               end_item() const;   
    ConstItemIteratorRange          get_items() const;
};

template<class _ItemNodeType>
struct XmlListNode: XmlGenericListNode
{
    static _ItemNodeType& cast_item_node(XmlRegularNode& item_node);
    static const _ItemNodeType& cast_item_node(const XmlRegularNode& item_node);

    using ItemNodeType = _ItemNodeType;
    using ItemIterator = boost::transform_iterator<ItemNodeType&(*)(XmlRegularNode&), XmlGenericListNode::ItemIterator>;
    using ConstItemIterator = boost::transform_iterator<const ItemNodeType&(*)(const XmlRegularNode&), XmlGenericListNode::ConstItemIterator>;
    
    using ItemIteratorRange = boost::iterator_range<ItemIterator>;
    using ConstItemIteratorRange = boost::iterator_range<ConstItemIterator>;

    std::unique_ptr<ItemNodeType>   create_item() const;
    ItemIterator                    insert_item(ItemIterator _where, std::unique_ptr<ItemNodeType> item = nullptr);
    void                            remove_item(ItemIterator _where);
    
    ItemIterator                    begin_item();
    ItemIterator                    end_item();
    ItemIteratorRange               get_items();

    ConstItemIterator               begin_item() const;
    ConstItemIterator               end_item() const;   
    ConstItemIteratorRange          get_items() const;
};

typedef XmlListNode<XmlBooleanNode> XmlBooleanListNode;
typedef XmlListNode<XmlIntegerNode> XmlIntegerListNode;
typedef XmlListNode<XmlStringNode> XmlStringListNode;
typedef XmlListNode<XmlEnumNode> XmlEnumListNode;
typedef XmlListNode<XmlStructNode> XmlStructListNode;

// doc function
std::unique_ptr<XmlRegularDoc>      parse_regular(const std::string& xml_reg, const XmlDefDoc& def_doc);
std::unique_ptr<XmlRegularDoc>      clone_regular(const XmlRegularDoc&);
XmlRegularDoc&                      bind_definition(XmlDoc& doc, const XmlDefDoc& def_doc);
std::unique_ptr<XmlRegularDoc>      create_regular(const XmlDefDoc& def_doc);
std::unique_ptr<XmlRegularNode>     create_regular_node(const XmlDefNode& def_node);

// navigation
template<class Visitor>
typename Visitor::result_type visit_regular_node(Visitor visitor, XmlRegularNode& reg_node);

template<class Visitor>
typename Visitor::result_type visit_regular_node(Visitor visitor, const XmlRegularNode& reg_node);

template<class T>
T& cast_regular_node(XmlRegularNode& reg_node);

template<class T>
const T& cast_regular_node(const XmlRegularNode& reg_node);

template<class T>
T* cast_regular_node(XmlRegularNode* reg_node);

template<class T>
const T* cast_regular_node(const XmlRegularNode* reg_node);

///////////////
// Implementation
///////////////

template<class T>
T XmlTrivialNode<T>::get_value() const
{
    return boost::lexical_cast<T>(get_wtext());
}

template<class T>
void XmlTrivialNode<T>::set_value(const T& value)
{
    using namespace xml_tools;
    auto text = stl_tools::ucs_to_utf8(boost::lexical_cast<std::wstring>(value));
    auto text_node = begin_child(*this);
    if (!text_node)
    {
        add_text_node(*this, text.c_str());
        return;
    }
    _ASSERT(get_node_type(*text_node) == XmlNodeType::TEXT);
    set_node_content(*text_node, text.c_str());
}

template<class T>
typename XmlListNode<T>::ItemNodeType& XmlListNode<T>::cast_item_node(XmlRegularNode& item_node)
{
    return static_cast<ItemNodeType&>(item_node);
}

template<class T>
const typename XmlListNode<T>::ItemNodeType& XmlListNode<T>::cast_item_node(const XmlRegularNode& item_node)
{
    return static_cast<const ItemNodeType&>(item_node);
}

template<class T>
std::unique_ptr<typename XmlListNode<T>::ItemNodeType> XmlListNode<T>::create_item() const
{
    return stl_tools::static_pointer_cast<ItemNodeType>(XmlGenericListNode::create_item());
}

template<class T>
typename XmlListNode<T>::ItemIterator XmlListNode<T>::insert_item(ItemIterator _where, std::unique_ptr<ItemNodeType> item)
{
    return ItemIterator(XmlGenericListNode::insert_item(_where.base(), std::move(item)), &cast_item_node);
}

template<class T>
void XmlListNode<T>::remove_item(ItemIterator _where)
{
    XmlGenericListNode::remove_item(_where.base());
}

template<class T>
typename XmlListNode<T>::ItemIterator XmlListNode<T>::begin_item()
{
    return ItemIterator(XmlGenericListNode::begin_item(), &cast_item_node);
}

template<class T>
typename XmlListNode<T>::ItemIterator XmlListNode<T>::end_item()
{
    return ItemIterator(XmlGenericListNode::end_item(), &cast_item_node);
}

template<class T>
typename XmlListNode<T>::ItemIteratorRange XmlListNode<T>::get_items()
{
    return ItemIteratorRange(begin_item(), end_item());
}

template<class T>
typename XmlListNode<T>::ConstItemIterator XmlListNode<T>::begin_item() const
{
    return ConstItemIterator(XmlGenericListNode::begin_item(), &cast_item_node);
}

template<class T>
typename XmlListNode<T>::ConstItemIterator XmlListNode<T>::end_item() const
{
    return ConstItemIterator(XmlGenericListNode::end_item(), &cast_item_node);
}

template<class T>
typename XmlListNode<T>::ConstItemIteratorRange XmlListNode<T>::get_items() const
{
    return ConstItemIteratorRange(begin_item(), end_item());
}

inline const XmlDefNode& resolve_link(const XmlDefNode& def_node)
{
    if (def_node.is_link())
    {
        return def_node.resolve();
    }
    return def_node;
}

template<class Visitor>
typename Visitor::result_type visit_regular_node(Visitor visitor, XmlRegularNode& reg_node)
{
    auto& bound_def = reg_node.get_bound_def();
    switch (bound_def.get_class())
    {

    case XmlNodeClass::TRIVIAL:
    {
        auto& trivial_def = static_cast<const XmlTrivialDef&>(bound_def);
        switch (trivial_def.get_type())
        {
        case XmlTrivialType::BOOLEAN:
            return visitor(static_cast<XmlBooleanNode&>(reg_node));

        case XmlTrivialType::INTEGER:
            return visitor(static_cast<XmlIntegerNode&>(reg_node));

        case XmlTrivialType::STRING:
            return visitor(static_cast<XmlStringNode&>(reg_node));

        case XmlTrivialType::ENUM:
            return visitor(static_cast<XmlEnumNode&>(reg_node));

        default:
            throw std::runtime_error("Invalid node type");
        }
    }

    case XmlNodeClass::STRUCT:
    {
        return visitor(static_cast<XmlStructNode&>(reg_node));
    }

    case XmlNodeClass::LIST:
    {
        auto& list_def = static_cast<const XmlListDef&>(bound_def);
        auto& item_def = resolve_link(list_def.get_item_def());

        switch (item_def.get_class())
        {
        case XmlNodeClass::STRUCT:
        {
            return visitor(static_cast<XmlStructListNode&>(reg_node));
        }

        case XmlNodeClass::TRIVIAL:
        {
            auto& trivial_def = static_cast<const XmlTrivialDef&>(item_def);
            switch (trivial_def.get_type())
            {
            case XmlTrivialType::BOOLEAN:
                return visitor(static_cast<XmlBooleanListNode&>(reg_node));

            case XmlTrivialType::INTEGER:
                return visitor(static_cast<XmlIntegerListNode&>(reg_node));

            case XmlTrivialType::STRING:
                return visitor(static_cast<XmlStringListNode&>(reg_node));

            case XmlTrivialType::ENUM:
                return visitor(static_cast<XmlEnumListNode&>(reg_node));

            default:
                throw std::runtime_error("Invalid node type");
            }
        }

        default:
            throw std::runtime_error("Invalid node type");
        }
    }
    
    default:
        throw std::runtime_error("Invalid node type");
    }
}

template<class Visitor>
struct ConstCastVisitor: boost::static_visitor<typename Visitor::result_type>
{
    ConstCastVisitor(Visitor visitor):
        m_visitor(std::move(visitor))
    {
    }

    template<class RegNodeType>
    result_type operator()(RegNodeType& reg_node)
    {
        return m_visitor(const_cast<const RegNodeType&>(reg_node));
    }

    Visitor m_visitor;
};

template<class Visitor>
typename Visitor::result_type visit_regular_node(Visitor visitor, const XmlRegularNode& reg_node)
{
    return visit_regular_node(ConstCastVisitor<Visitor>(std::move(visitor)), const_cast<XmlRegularNode&>(reg_node));
}

template<class RegNodeType>
struct CastRegularNodeRef: boost::static_visitor<RegNodeType&>
{
    RegNodeType& operator()(RegNodeType& reg_node) const
    {
        return reg_node;
    }

    RegNodeType& operator()(const XmlRegularNode& reg_node) const
    {
        throw std::logic_error("Bad cast"); 
    }
};

template<class T>
T& cast_regular_node(XmlRegularNode& reg_node)
{
    return visit_regular_node(CastRegularNodeRef<T>(), reg_node);
}

template<class T>
const T& cast_regular_node(const XmlRegularNode& reg_node)
{
    return visit_regular_node(CastRegularNodeRef<const T>(), reg_node);
}

template<class RegNodeType>
struct CastRegularNodePtr: boost::static_visitor<RegNodeType*>
{
    RegNodeType* operator()(RegNodeType& reg_node) const
    {
        return &reg_node;
    }

    RegNodeType* operator()(const XmlRegularNode& reg_node) const
    {
        return nullptr;
    }
};

template<class T>
T* cast_regular_node(XmlRegularNode* reg_node)
{
    if (!reg_node)
    {
        return nullptr;
    }

    return visit_regular_node(CastRegularNodePtr<T>(), *reg_node);
}

template<class T>
const T* cast_regular_node(const XmlRegularNode* reg_node)
{
    if (!reg_node)
    {
        return nullptr;
    }

    return visit_regular_node(CastRegularNodePtr<const T>(), *reg_node);
}

}} //namespace TR { namespace XML {