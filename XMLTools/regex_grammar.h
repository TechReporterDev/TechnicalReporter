#pragma once
#include "libxml2_wrap.h"
#include "libxml2_range.h"
namespace xml_tools {

struct XmlGrammarNode: XmlNode
{
    enum class type { MARKUP_NODE, COPY_NODE, TEXT_NODE };
    type                                get_node_type() const;
    size_t                              get_input_index() const;
    boost::optional<std::string>        get_default_input() const;
};

struct XmlGrammarCopyNode: XmlGrammarNode
{
};

struct XmlGrammarTextNode: XmlGrammarNode
{
    std::string get_text() const;
};

struct XmlGrammarMarkupNode: XmlGrammarNode
{
    static const XmlGrammarNode* next_grammar_node(const XmlGrammarNode* grammar_node);
    using ConstGrammarNodeIterator = xml_tools::node_iterator<const XmlGrammarNode, next_grammar_node>;
    using ConstGrammarNodeRange = boost::iterator_range<ConstGrammarNodeIterator>;

    boost::optional<std::string>        get_selection() const;  
    bool                                continue_selection() const;
    ConstGrammarNodeRange               get_child_grammar_nodes() const;
};

struct XmlGrammarDoc: XmlDoc
{
    const XmlGrammarMarkupNode& get_root() const;
};

std::unique_ptr<XmlGrammarDoc> parse_grammar(const std::string& grammar);

} //namespace xml_tools