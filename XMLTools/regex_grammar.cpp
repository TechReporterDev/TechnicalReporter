#include "stdafx.h"
#include "regex_grammar.h"
#include "boost\lexical_cast.hpp"
namespace xml_tools {

XmlGrammarNode::type XmlGrammarNode::get_node_type() const
{
    if (node_name_equal(*this, "copy"))
    {
        return XmlGrammarNode::type::COPY_NODE;
    }
    else if (node_name_equal(*this, "text"))
    {
        return XmlGrammarNode::type::TEXT_NODE;
    }
    else if (node_name_equal(*this, "selection"))
    {
        throw std::logic_error("Invalid grammar node");
    }
    return XmlGrammarNode::type::MARKUP_NODE;   
}

size_t XmlGrammarNode::get_input_index() const
{
    if (auto attr = find_attribute(*this, "input"))
    {
        return boost::lexical_cast<size_t>(get_attr_value(*attr));
    }
    return 0;
}

boost::optional<std::string> XmlGrammarNode::get_default_input() const
{
    if (auto default_input_attr = find_attribute(*this, "default_input"))
    {
        return get_attr_value(*default_input_attr);
    }
    return boost::none;
}

std::string XmlGrammarTextNode::get_text() const
{
    return get_node_text(*this);
}

const XmlGrammarNode* XmlGrammarMarkupNode::next_grammar_node(const XmlGrammarNode* grammar_node)
{
    _ASSERT(grammar_node);
    for (const XmlNode* sibling = next_child(grammar_node); sibling != nullptr; sibling = next_child(sibling))
    {
        if (!node_name_equal(*sibling, "selection"))
        {
            return static_cast<const XmlGrammarNode*>(sibling);
        }
    }
    return nullptr;
}

boost::optional<std::string> XmlGrammarMarkupNode::get_selection() const
{
    if (auto select_attr = find_attribute(*this, "select"))
    {
        return get_attr_value(*select_attr);
    }
    return boost::none;
}

bool XmlGrammarMarkupNode::continue_selection() const
{
    if (auto select_attr = find_attribute(*this, "select"))
    {
        return attr_value_equal(*select_attr, "continue");
    }
    return false;
}

XmlGrammarMarkupNode::ConstGrammarNodeRange XmlGrammarMarkupNode::get_child_grammar_nodes() const
{
    const XmlGrammarNode* first_node = nullptr;
    for (auto& child : get_child_nodes(*this))
    {
        if (! node_name_equal(child, "selection"))
        {
            first_node = static_cast<const XmlGrammarNode*>(&child);
            break;
        }
    }
    return {ConstGrammarNodeIterator(first_node), ConstGrammarNodeIterator()};
}

const XmlGrammarMarkupNode& XmlGrammarDoc::get_root() const
{
    auto root = xml_tools::get_root(*this);
    if (!root)
    {
        throw std::logic_error("Invalid grammar document");
    }
    return static_cast<const XmlGrammarMarkupNode&>(*root);
}

std::unique_ptr<XmlGrammarDoc> parse_grammar(const std::string& grammar)
{
    return stl_tools::static_pointer_cast<XmlGrammarDoc>(parse(grammar));
}

} //namespace xml_tools