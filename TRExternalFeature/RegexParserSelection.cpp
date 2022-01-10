#include "stdafx.h"
#include "RegexParserSelection.h"
#include "TRCore\ReportTypes.h"
#include "TRCore\Content.h"
#include "TRCore\RegistryRefs.h"

namespace TR { namespace External {

RegexParserSelection::RegexParserSelection(UUID uuid, SelectionPlan selection_plan, std::string regex_grammar, std::function<std::string(const std::string&)> preprocessor):
    Selection(std::move(uuid)),
    m_plan(selection_plan),
    m_parser(std::make_shared<xml_tools::regex_parser>(regex_grammar)),
    m_preprocessor(std::move(preprocessor))
{
}

SelectionPlan RegexParserSelection::get_plan() const
{
    return m_plan;
}

RegexParserSelection::Functor RegexParserSelection::prepare_functor(SourceRef /*source_ref*/) const
{
    auto parser = m_parser;
    auto preprocessor = m_preprocessor;
    auto& output_type = dynamic_cast<const XmlReportType&>(*m_plan.m_output_ref);
    auto make_content = output_type.prepare_make_content_from_xml();

    return [parser, preprocessor, make_content](const std::vector<std::shared_ptr<Content>>& content){
        _ASSERT(!content.empty());
        auto& text_content = dynamic_cast<TextContent&>(*content[0]);
        if (preprocessor)
        {
            return make_content(parser->parse(preprocessor(text_content.as_string())));
        }
        return make_content(parser->parse(text_content.as_string()));
    };
}

}} //namespace TR { namespace External {