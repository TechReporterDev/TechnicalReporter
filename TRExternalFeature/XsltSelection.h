#pragma once
#include "TRCore\Selections.h"
#include "XmlTools\libxslt_wrap.h"
#include "stddecl.h"
namespace TR { namespace External {

class XsltSelection: public Selection
{
public:
    XsltSelection(UUID uuid, SelectionPlan selection_plan, std::string xslt_stylesheet);

    virtual SelectionPlan       get_plan() const override;
    virtual Functor             prepare_functor(SourceRef /*source_ref*/) const override;

private:
    SelectionPlan m_plan;
    std::shared_ptr<xml_tools::XsltStylesheet> m_stylesheet;
};

}} //namespace TR { namespace External {
