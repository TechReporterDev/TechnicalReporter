#pragma once
#include "TRCore\ActionDelegates.h"
#include "stddecl.h"

namespace TR {namespace External {

class ScriptActionDelegate: public ActionDelegate
{
public:
    using ParamsTransformFunctor = std::function<std::unique_ptr<XmlDoc>(const XmlDoc&)>;
    using OutputTransformFunctor = std::function<Blob(const Blob&)>;

    ScriptActionDelegate(ActionRef host_action_ref, ActionRef action_ref, ParamsTransformFunctor params_transform_func, OutputTransformFunctor output_transform_func);
    virtual ActionFunctor prepare_action(SourceRef source_ref) const override;

private:
    ParamsTransformFunctor m_params_transform_func;
    OutputTransformFunctor m_output_transform_func;
};

ScriptActionDelegate::ParamsTransformFunctor xslt_params_transform(std::unique_ptr<XsltStylesheet> xslt_stylesheet);
ScriptActionDelegate::OutputTransformFunctor regex_parser_output_transform(std::string regex_grammar);
ScriptActionDelegate::OutputTransformFunctor xslt_output_transform(std::unique_ptr<XsltStylesheet> xslt_stylesheet);

}} //namespace TR { namespace External {