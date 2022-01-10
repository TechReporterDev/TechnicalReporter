#pragma once
#include "TRCore\Selections.h"
#include "XmlTools\regex_parser.h"
#include "stddecl.h"
namespace TR { namespace External {

class RegexParserSelection: public Selection
{
public:
    RegexParserSelection(UUID uuid, SelectionPlan selection_plan, std::string regex_grammar, std::function<std::string(const std::string&)> preprocessor = nullptr);

    virtual SelectionPlan       get_plan() const override;
    virtual Functor             prepare_functor(SourceRef /*source_ref*/) const override;

private:
    SelectionPlan m_plan;
    std::shared_ptr<xml_tools::regex_parser> m_parser;
    std::function<std::string(const std::string&)> m_preprocessor;
};

}} //namespace TR { namespace External {
