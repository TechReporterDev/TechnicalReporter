#include "stdafx.h"
#include "regex_parser.h"
#include "boost\regex.hpp"
namespace xml_tools {

void add_nodes(XmlNode& parent, std::vector<std::unique_ptr<XmlNode>> nodes)
{
    for (auto& node : nodes)
    {
        if (node_name_equal(*node, "id"))
        {
            set_attr_value(parent, "id", get_node_text(*node).c_str());         
        }
        else
        {
            add_node(parent, std::move(node));
        }       
    }
}

regex_parser::regex_parser(std::unique_ptr<XmlGrammarDoc> grammar):
    m_grammar(std::move(grammar))
{
}

regex_parser::regex_parser(const std::string& grammar):
    m_grammar(parse_grammar(grammar))
{
}

using substr = boost::iterator_range<std::string::const_iterator>;
std::vector<std::unique_ptr<XmlNode>> parse(substr input, const XmlGrammarNode& grammar_node);

std::vector<std::unique_ptr<XmlNode>> apply_selection(const boost::smatch& selection, const XmlGrammarNode& grammar_node)
{
    if (grammar_node.get_node_type() == XmlGrammarNode::type::MARKUP_NODE)
    {
        auto& grammar_markup_node = static_cast<const XmlGrammarMarkupNode&>(grammar_node);
        if (grammar_markup_node.continue_selection())
        {
            std::vector<std::unique_ptr<XmlNode>> result;
            result.push_back(clone_node(grammar_markup_node, XmlCloneOpt::NONE));
            for (auto& child_grammar_node : grammar_markup_node.get_child_grammar_nodes())
            {
                add_nodes(*result.back(), apply_selection(selection, child_grammar_node));
            }
            return result;
        }
    }

    auto input_index = grammar_node.get_input_index();
    if (selection.size() <= input_index)
    {
        throw std::logic_error("Regex parser failed: invalid input index");
    }

    if (selection[input_index].matched)
    {
        return parse({selection[input_index].first, selection[input_index].second}, grammar_node);
    }
    else if (auto default_input = grammar_node.get_default_input())
    {
        return parse({default_input->begin(), default_input->end()}, grammar_node);
    }
    else
    {
        return{};
    }
}

std::unique_ptr<XmlNode> parse(substr input, const XmlGrammarTextNode& grammar_text_node)
{
    return create_text_node(grammar_text_node.get_text().c_str());
}

std::unique_ptr<XmlNode> parse(substr input, const XmlGrammarCopyNode& grammar_copy_node)
{
    return create_text_node(std::string(input.begin(), input.end()).c_str());
}

boost::match_flag_type match_flags(bool begin_of_line)
{
    auto match_flags = boost::regex_constants::match_default;
    if (!begin_of_line)
    {
        match_flags |= boost::regex_constants::match_not_bol;
    }
    return match_flags;
}

std::vector<std::unique_ptr<XmlNode>> parse(substr input, const XmlGrammarMarkupNode& grammar_markup_node)
{
    std::vector<std::unique_ptr<XmlNode>> result;

    if (auto selection = grammar_markup_node.get_selection())
    {       
        boost::regex regex(*selection);
        boost::smatch match_result;
        bool begin_of_line = true;
        auto start = input.begin();     
        while (boost::regex_search(start, input.end(), match_result, regex, match_flags(begin_of_line)))
        {
            if (match_result[0].length() == 0)
            {
                throw std::logic_error("Regex parser failed: infinite processing");
            }

            result.push_back(clone_node(grammar_markup_node, XmlCloneOpt::NONE));
            for (auto& child_grammar_node : grammar_markup_node.get_child_grammar_nodes())
            {
                add_nodes(*result.back(), apply_selection(match_result, child_grammar_node));               
            }           
            
            begin_of_line = *(match_result[0].second - 1) == '\n';
            start = match_result[0].second;
        }

        if (result.empty())
        {
            if (auto default_input = grammar_markup_node.get_default_input())
            {
                if (*default_input != input)
                {
                    result = parse({default_input->begin(), default_input->end()}, grammar_markup_node);
                }
            }
        }       
    }
    else
    {
        result.push_back(clone_node(grammar_markup_node, XmlCloneOpt::NONE));
        for (auto& child_grammar_node : grammar_markup_node.get_child_grammar_nodes())
        {
            add_nodes(*result.back(), parse(input, child_grammar_node));
        }
    }
    return result;
}

std::vector<std::unique_ptr<XmlNode>> parse(substr input, const XmlGrammarNode& grammar_node)
{
    std::vector<std::unique_ptr<XmlNode>> result;
    switch (grammar_node.get_node_type())
    {
    case XmlGrammarNode::type::TEXT_NODE:
        result.push_back(parse(input, static_cast<const XmlGrammarTextNode&>(grammar_node)));
        break;

    case XmlGrammarNode::type::COPY_NODE:
        result.push_back(parse(input, static_cast<const XmlGrammarCopyNode&>(grammar_node)));
        break;

    case XmlGrammarNode::type::MARKUP_NODE:
        result = parse(input, static_cast<const XmlGrammarMarkupNode&>(grammar_node));
        break;

    default:
        _ASSERT(false);
    }
    return result;
}

std::unique_ptr<XmlDoc> regex_parser::parse(const std::string& input)
{
    auto top = xml_tools::parse({input.begin(), input.end()}, m_grammar->get_root());
    if (top.empty())
    {
        return nullptr;
    }

    if (top.size() > 1)
    {
        throw std::logic_error("Regex parser failed: multiple root nodes found");
    }

    auto parsed_doc = create_doc();
    set_root(*parsed_doc, std::move(top.at(0)));
    return parsed_doc;
}

} //namespace xml_tools