#include "stdafx.h"
#include "XMLRegularTable.h"
namespace TR { namespace XML {

namespace {

using Header = XMLRegularTable::Header;
using Row = XMLRegularTable::Row;
using Rows = XMLRegularTable::Rows;

Rows accumulate(Rows rows1, Rows rows2)
{
    for (auto& row : rows2)
    {
        if (!row.empty())
        {
            rows1.push_back(std::move(row));
        }
    }
    return rows1;
}

Rows combine(Rows rows1, Rows rows2)
{
    Rows result;
    for (const Row& row1 : rows1)
    {
        for (const Row& row2 : rows2)
        {
            Row row(row1);
            std::copy(row2.begin(), row2.end(), std::back_inserter(row));
            result.push_back(row);
        }
    }
    return result;
}

Row append(Row rows1, Row rows2)
{
    std::copy(rows2.begin(), rows2.end(), std::back_inserter(rows1));
    return rows1;
}

Header make_table_header(const XmlDefNode& def_node)
{
    if (def_node.is_trivial())
    {
        return{&def_node};
    }

    Header header;
    for (auto& child_def_node : get_child_defs(def_node))
    {
        boost::copy(make_table_header(child_def_node), std::back_inserter(header));
    }
    return header;
}

struct Context
{
    bool is_recursive(const XmlDefNode& def_node) const
    {
        return m_recursive.find(&def_node) != m_recursive.end();
    }

    bool is_top(XmlRegularNode& regular_node) const
    {
        for (auto parent_node : m_path)
        {
            if (&parent_node->get_bound_def() == &regular_node.get_bound_def())
            {
                return false;
            }
        }
        return true;
    }

    std::set<const XmlDefNode*> m_recursive;
    std::vector<XmlRegularNode*> m_path;
    std::list<XmlRegularNode*> m_links;
};

void initialize_context(const XmlDefNode& def_node, Context& context)
{
    if (def_node.is_link())
    {
        auto& resolved = def_node.resolve();
        if (xml_tools::is_descendant(resolved, def_node))
        {
            context.m_recursive.insert(&resolved);
        }
    }

    for (auto& child_def : get_child_defs(def_node))
    {
        initialize_context(child_def, context);
    }
}

void initialize_context(const XmlDefDoc& def_doc, Context& context)
{
    initialize_context(*def_doc.get_root_def(), context);
}

Row make_empty_row(const XmlDefNode& def_node)
{
    auto header = make_table_header(def_node);
    return Row(header.size(), nullptr);
}

struct NodeRows
{
    Rows m_expanded;
    Row m_collapsed;
};

Rows make_result(NodeRows&& node_rows)
{
    if (!node_rows.m_expanded.empty())
    {
        return std::move(node_rows.m_expanded);
    }
    return{std::move(node_rows.m_collapsed)};
}

NodeRows make_node_rows(XmlRegularNode& regular_node, Context& context);
NodeRows make_list_rows(XmlListNode<XmlRegularNode>& list_node, Context& context)
{
    Rows expanded;
    for (auto& item : list_node.get_items())
    {
        expanded = accumulate(std::move(expanded), make_result(make_node_rows(item, context)));
    }

    return{std::move(expanded), make_empty_row(list_node.get_bound_def())};
}

NodeRows make_struct_rows(XmlStructNode& struct_node, Context& context)
{
    Rows expanded;
    Row collapsed;

    stl_tools::associate(struct_node.get_members(), struct_node.get_bound_def().get_member_defs(), [&](XmlRegularNode* member_node, const XmlDefNode* member_def_node){

        if (member_node != nullptr && member_def_node == nullptr)
        {
            throw std::logic_error("Invalid regular document");
        }

        if (member_node == nullptr || &member_node->get_bound_def() != member_def_node)
        {
            if (member_def_node->is_optional())
            {
                auto empty_row = make_empty_row(*member_def_node);
                expanded = combine(std::move(expanded), {empty_row});
                collapsed = append(std::move(collapsed), empty_row);
                return stl_tools::loop_stmt::CONTINUE_RIGHT;
            }
            throw std::logic_error("Invalid regular document");
        }

        auto member_rows = make_node_rows(*member_node, context);
        expanded = accumulate(
            combine(std::move(expanded), {member_rows.m_collapsed}),
            combine({collapsed}, std::move(member_rows.m_expanded))
        );

        collapsed = append(std::move(collapsed), std::move(member_rows.m_collapsed));
        return stl_tools::loop_stmt::CONTINUE;
    });
    return{std::move(expanded), std::move(collapsed)};
}

NodeRows make_trivial_rows(XmlGenericTrivialNode& trivial_node)
{
    return{{}, {&trivial_node}};
}

NodeRows make_element_rows(XmlRegularNode& regular_node, Context& context)
{
    context.m_path.push_back(&regular_node);
    auto guard = stl_tools::finally([&]{ context.m_path.pop_back(); });

    switch (regular_node.get_bound_def().get_class())
    {
    case XmlNodeClass::LIST:
        return make_list_rows(static_cast<XmlListNode<XmlRegularNode>&>(regular_node), context);

    case XmlNodeClass::STRUCT:
        return make_struct_rows(static_cast<XmlStructNode&>(regular_node), context);

    case XmlNodeClass::TRIVIAL:
        return make_trivial_rows(static_cast<XmlGenericTrivialNode&>(regular_node));

    default:
        _ASSERT(false);
    }

    return{}; //supress warning
}

NodeRows make_node_rows(XmlRegularNode& regular_node, Context& context)
{
    if (!context.is_recursive(regular_node.get_bound_def()))
    {
        return make_element_rows(regular_node, context);
    }

    if (!context.is_top(regular_node))
    {
        context.m_links.push_back(&regular_node);
        return {};
    }

    auto rows = make_result(make_element_rows(regular_node, context));
    for (auto link = context.m_links.begin(); link != context.m_links.end();)
    {
        if (&(*link)->get_bound_def() == &regular_node.get_bound_def())
        {
            rows = accumulate(std::move(rows), make_result(make_element_rows(**link, context)));
            context.m_links.erase(link++);
            continue;
        }
        ++link;
    }
    return{std::move(rows), make_empty_row(regular_node.get_bound_def())};
}

} // namespace {

XMLRegularTable::XMLRegularTable(std::vector<const XmlDefNode*> header, std::vector<Row> rows):
    m_header(std::move(header)),
    m_rows(std::move(rows))
{
}

XMLRegularTable::XMLRegularTable(XMLRegularTable&& regular_table):
    m_header(std::move(regular_table.m_header)),
    m_rows(std::move(regular_table.m_rows))
{
}

XMLRegularTable& XMLRegularTable::operator = (XMLRegularTable&& regular_table)
{
    m_header = std::move(regular_table.m_header);
    m_rows = std::move(regular_table.m_rows);
    return *this;
}

XMLRegularTable make_regular_table(XmlRegularDoc& regular_doc)
{
    auto doc_root = regular_doc.get_root();
    _ASSERT(doc_root);

    auto def_root = regular_doc.get_bound_def().get_root_def();
    _ASSERT(def_root);

    Context context;
    initialize_context(regular_doc.get_bound_def(), context);
    return{make_table_header(*def_root), make_result(make_node_rows(*doc_root, context))};
}

}} //namespace TR { namespace XML {