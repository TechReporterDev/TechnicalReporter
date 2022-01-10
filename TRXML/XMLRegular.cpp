#include "stdafx.h"
#include "XMLRegular.h"
namespace TR {  namespace XML {
using namespace xml_tools;

const XmlDefDoc& XmlRegularDoc::get_bound_def() const
{
    auto def_doc = (const XmlDefDoc*)_private;
    if (!def_doc)
    {
        throw std::logic_error("Document has no bound definition");
    }
    return *def_doc;
}

XmlRegularNode* XmlRegularDoc::get_root()
{
    return static_cast<XmlRegularNode*>(xml_tools::get_root(*this));
}

const XmlRegularNode* XmlRegularDoc::get_root() const
{
    return const_cast<XmlRegularDoc*>(this)->get_root();
}

const XmlDefNode& XmlRegularNode::get_bound_def() const
{
    auto def_node = (const XmlDefNode*)_private;
    if (!def_node)
    {
        throw std::logic_error("Node has no bound definition");
    }
    return *def_node;
}

XmlRegularNode* XmlRegularNode::get_parent()
{
    return static_cast<XmlRegularNode*>(xml_tools::get_parent(*this));
}

const XmlRegularNode* XmlRegularNode::get_parent() const
{
    return const_cast<XmlRegularNode*>(this)->get_parent();
}


const XmlTrivialDef& XmlGenericTrivialNode::get_bound_def() const
{
    return static_cast<const XmlTrivialDef&>(XmlRegularNode::get_bound_def());
}

std::string XmlGenericTrivialNode::get_text() const
{
    return xml_tools::get_node_text(*this);
}

std::wstring XmlGenericTrivialNode::get_wtext() const
{
    return xml_tools::get_node_wtext(*this);
}

const XmlBooleanDef& XmlBooleanNode::get_bound_def() const
{
    return static_cast<const XmlBooleanDef&>(XmlRegularNode::get_bound_def());
}

const XmlIntegerDef& XmlIntegerNode::get_bound_def() const
{
    return static_cast<const XmlIntegerDef&>(XmlRegularNode::get_bound_def());
}

const XmlStringDef& XmlStringNode::get_bound_def() const
{
    return static_cast<const XmlStringDef&>(XmlRegularNode::get_bound_def());
}

const XmlEnumDef& XmlEnumNode::get_bound_def() const
{
    return static_cast<const XmlEnumDef&>(XmlRegularNode::get_bound_def());
}

XmlRegularNode* XmlStructNode::next_member(XmlRegularNode* member_node)
{
    _ASSERT(member_node);
    return static_cast<XmlRegularNode*>(xml_tools::next_child(member_node));
}

const XmlRegularNode* XmlStructNode::next_member(const XmlRegularNode* member_node)
{
    _ASSERT(member_node);
    return next_member(const_cast<XmlRegularNode*>(member_node));
}

const XmlStructDef& XmlStructNode::get_bound_def() const
{
    return static_cast<const XmlStructDef&>(XmlRegularNode::get_bound_def());
}

xmlAttr* XmlStructNode::find_id_attr()
{
    return find_attribute(*this, "id"); 
}

const xmlAttr* XmlStructNode::find_id_attr() const
{
    return const_cast<XmlStructNode*>(this)->find_id_attr();
}

XmlStructNode::MemberIterator XmlStructNode::begin_member()
{
    return MemberIterator(static_cast<XmlRegularNode*>(xml_tools::begin_child(*this)));
}

XmlStructNode::MemberIterator XmlStructNode::end_member()
{
    return MemberIterator();
}

XmlStructNode::MemberIteratorRange XmlStructNode::get_members()
{
    return MemberIteratorRange(begin_member(), end_member());
}

XmlStructNode::ConstMemberIterator XmlStructNode::begin_member() const
{
    return ConstMemberIterator(static_cast<const XmlRegularNode*>(xml_tools::begin_child(*this)));
}

XmlStructNode::ConstMemberIterator XmlStructNode::end_member() const
{
    return ConstMemberIterator();
}

XmlStructNode::ConstMemberIteratorRange XmlStructNode::get_members() const
{
    return ConstMemberIteratorRange(begin_member(), end_member());
}

XmlRegularNode* XmlGenericListNode::next_item(XmlRegularNode* item_node)
{
    _ASSERT(item_node);
    return static_cast<XmlRegularNode*>(xml_tools::next_child(item_node));
}

const XmlRegularNode* XmlGenericListNode::next_item(const XmlRegularNode* item_node)
{
    _ASSERT(item_node);
    return next_item(const_cast<XmlRegularNode*>(item_node));
}
    
const XmlListDef& XmlGenericListNode::get_bound_def() const
{
    return static_cast<const XmlListDef&>(XmlRegularNode::get_bound_def());
}

bool XmlGenericListNode::empty() const
{
    return begin_item() == end_item();
}
    
std::unique_ptr<XmlRegularNode> XmlGenericListNode::create_item() const
{
    auto& list_def = get_bound_def();
    return create_regular_node(list_def.get_item_def());
}

XmlGenericListNode::ItemIterator XmlGenericListNode::insert_item(ItemIterator _where, std::unique_ptr<XmlRegularNode> item)
{
    if (!item)
    {
        item = create_item();
    }

    if (_where == end_item())
    {
        return ItemIterator(&xml_tools::add_node(*this, std::move(item)));
    }
    return ItemIterator(&xml_tools::insert_node(*_where, std::move(item)));
}

void XmlGenericListNode::remove_item(ItemIterator _where)
{
    xml_tools::remove_node(*_where);
}
    
XmlGenericListNode::ItemIterator XmlGenericListNode::begin_item()
{
    return ItemIterator(static_cast<XmlRegularNode*>(xml_tools::begin_child(*this)));
}

XmlGenericListNode::ItemIterator XmlGenericListNode::end_item()
{
    return ItemIterator();
}

XmlGenericListNode::ItemIteratorRange XmlGenericListNode::get_items()
{
    return ItemIteratorRange(begin_item(), end_item());
}

XmlGenericListNode::ConstItemIterator XmlGenericListNode::begin_item() const
{
    return ConstItemIterator(static_cast<const XmlRegularNode*>(xml_tools::begin_child(*this)));
}

XmlGenericListNode::ConstItemIterator XmlGenericListNode::end_item() const
{
    return ConstItemIterator();
}

XmlGenericListNode::ConstItemIteratorRange XmlGenericListNode::get_items() const
{
    return ConstItemIteratorRange(begin_item(), end_item());
}

std::unique_ptr<XmlRegularDoc> parse_regular(const std::string& xml_reg, const XmlDefDoc& def_doc)
{
    auto doc = xml_tools::parse(xml_reg);
    bind_definition(*doc, def_doc);
    return std::unique_ptr<XmlRegularDoc>((XmlRegularDoc*)doc.release());
}

std::unique_ptr<XmlRegularDoc> clone_regular(const XmlRegularDoc& reg_doc)
{
    return stl_tools::static_pointer_cast<XmlRegularDoc>(xml_tools::clone_doc(reg_doc));
}

static void bind_definition(XmlNode& node, const XmlDefNode& def_node);
struct BindDefinition: boost::static_visitor<void>
{
    BindDefinition(XmlNode& node):
        m_node(node)
    {
    }

    void operator()(const XmlTrivialDef& trivial_def)
    {
    }

    void operator()(const XmlLinkDef& link_def)
    {
        bind_definition(m_node, link_def.resolve());
    }

    void operator()(const XmlStructDef& struct_def)
    {
        stl_tools::associate(get_child_nodes(m_node), struct_def.get_member_defs(), [](XmlNode* child_node, const XmlDefNode* child_def){
        
            if (child_def == nullptr)
            {
                throw std::logic_error("Document does not match to definition");
            }

            if (child_node == nullptr || node_name_equal(*child_node, *child_def) == false)
            {
                if (child_def->is_optional())
                {
                    return stl_tools::loop_stmt::CONTINUE_RIGHT;
                }
                throw std::logic_error("Document does not match to definition");
            }       

            bind_definition(*child_node, *child_def);
            return stl_tools::loop_stmt::CONTINUE;
        }); 
    }

    void operator()(const XmlListDef& list_def)
    {
        for (auto& child: get_child_nodes(m_node))
        {
            bind_definition(child, list_def.get_item_def());
        }
    }

    XmlNode& m_node;
};

static void bind_definition(XmlNode& node, const XmlDefNode& def_node)
{
    if (! node_name_equal(node, def_node))
    {
        throw std::logic_error("Document does not match to definition");
    }

    node._private = (void*)&def_node;
    visit_def_node(BindDefinition(node), def_node);
}

XmlRegularDoc& bind_definition(XmlDoc& doc, const XmlDefDoc& def_doc)
{
    doc._private = (void*)&def_doc;

    auto root = get_root(doc);
    if (!root)
    {
        throw std::logic_error("Document has no root element");
    }

    auto def_root = def_doc.get_root_def();
    _ASSERT(def_root);

    bind_definition(*root, *def_root);
    return (XmlRegularDoc&)doc;
}

static std::unique_ptr<XmlRegularNode> create_node(const XmlDefNode& def_node);
struct CreateNode: public boost::static_visitor<std::unique_ptr<XmlRegularNode>>
{
    std::unique_ptr<XmlRegularNode> operator()(const XmlTrivialDef& trivial_def)
    {
        auto node = xml_tools::create_element(xml_tools::get_node_name(trivial_def).c_str());
        node->_private = (void*)&trivial_def;
        auto default_value = trivial_def.get_default_string();
        if (!default_value.empty())
        { 
            xml_tools::add_text_node(*node, default_value.c_str());
        }
        return stl_tools::static_pointer_cast<XmlRegularNode>(std::move(node));
    }

    std::unique_ptr<XmlRegularNode> operator()(const XmlLinkDef& link_def)
    {
        return create_node(link_def.resolve());
    }

    std::unique_ptr<XmlRegularNode> operator()(const XmlStructDef& struct_def)
    {
        auto node = xml_tools::create_element(xml_tools::get_node_name(struct_def).c_str());
        node->_private = (void*)&struct_def;
        for (auto& member_def : struct_def.get_member_defs())
        {
            if (auto member_node = create_node(member_def))
            {
                xml_tools::add_node(*node, std::move(member_node));
            }
        }
        return stl_tools::static_pointer_cast<XmlRegularNode>(std::move(node));
    }

    std::unique_ptr<XmlRegularNode> operator()(const XmlListDef& list_def)
    {
        auto node = xml_tools::create_element(xml_tools::get_node_name(list_def).c_str());
        node->_private = (void*)&list_def;
        return stl_tools::static_pointer_cast<XmlRegularNode>(std::move(node));
    }
};

static std::unique_ptr<XmlRegularNode> create_node(const XmlDefNode& def_node)
{
    if (def_node.is_optional())
    {
        return nullptr;
    }

    return visit_def_node(CreateNode(), def_node);  
}

std::unique_ptr<XmlRegularDoc> create_regular(const XmlDefDoc& def_doc)
{
    auto doc = xml_tools::create_doc();
    doc->_private = (void*)&def_doc;

    auto root_def = def_doc.get_root_def();
    _ASSERT(root_def);
    xml_tools::set_root(*doc, create_node(*root_def));
    return std::unique_ptr<XmlRegularDoc>((XmlRegularDoc*)doc.release());
}

std::unique_ptr<XmlRegularNode> create_regular_node(const XmlDefNode& def_node)
{
    return create_node(def_node);
}

}} //namespace TR { namespace XML {