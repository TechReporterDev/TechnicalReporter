#pragma once
#include "regex_grammar.h"
namespace xml_tools {

class regex_parser
{
public:
    regex_parser(std::unique_ptr<XmlGrammarDoc> grammar);
    regex_parser(const std::string& grammar);
    
    std::unique_ptr<XmlDoc> parse(const std::string& input);

private:
    std::unique_ptr<XmlGrammarDoc> m_grammar;
};

} //namespace xml_tools