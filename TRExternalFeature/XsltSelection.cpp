#include "stdafx.h"
#include "XsltSelection.h"
#include <TRCore\ReportTypes.h>
#include <TRCore\Content.h>
#include <TRCore\RegistryRefs.h>
namespace TR { namespace External {


XsltSelection::XsltSelection(UUID uuid, SelectionPlan selection_plan, std::string xslt_stylesheet):
    Selection(std::move(uuid)),
    m_plan(selection_plan)
{
    auto doc = xml_tools::parse(xslt_stylesheet);
    m_stylesheet = xml_tools::xslt_parse_stylesheet_doc(std::move(doc));
}

SelectionPlan XsltSelection::get_plan() const
{
    return m_plan;
}

XsltSelection::Functor XsltSelection::prepare_functor(SourceRef /*source_ref*/) const
{
    auto stylesheet = m_stylesheet;
    auto& output_type = dynamic_cast<const XmlReportType&>(*m_plan.m_output_ref);
    auto make_content = output_type.prepare_make_content_from_xml();

    return [stylesheet, make_content](const std::vector<std::shared_ptr<Content>>& contents){
        _ASSERT(!contents.empty());

        if (contents.size() == 1)
        {
            auto& xml_content = dynamic_cast<XmlContent&>(*contents.front());
            return make_content(xml_tools::xslt_apply_stylesheet(*stylesheet, xml_content.as_xml_doc()));
        }

        std::vector<std::unique_ptr<XmlDoc>> inputs;
        for (auto content : contents)
        {
            if (auto xml_content = dynamic_cast<XmlContent*>(content.get()))
            {
                inputs.push_back(clone_doc(xml_content->as_xml_doc()));
            }
        }
        return make_content(xml_tools::xslt_apply_stylesheet(*stylesheet, *compose(std::move(inputs), "custom_query")));
    };
}

}} //namespace TR { namespace External {