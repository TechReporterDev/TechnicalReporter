#include "stdafx.h"
#include "ScriptActionDelegate.h"
#include "W32Cmd.h"
#include "TRCore\Sources.h"
#include "TRCore\ReportTypes.h"
#include "TRCore\Actions.h"
#include "TRCore\Content.h"
#include "XMLTools\regex_parser.h"
namespace TR { namespace External {

ScriptActionDelegate::ScriptActionDelegate(ActionRef host_action_ref, ActionRef action_ref, ParamsTransformFunctor params_transform_func, OutputTransformFunctor output_transform_func):
    ActionDelegate({}, {host_action_ref}, action_ref),
    m_params_transform_func(params_transform_func),
    m_output_transform_func(output_transform_func)
{
}

ScriptActionDelegate::ActionFunctor ScriptActionDelegate::prepare_action(SourceRef source_ref) const
{
    auto& action = *m_guest_action_ref;
    auto make_content = action.prepare_make_content();
    auto params_transform_func = m_params_transform_func;
    auto output_transform_func = m_output_transform_func;

    return [params_transform_func, output_transform_func, make_content](const XmlDoc& params, std::vector<std::shared_ptr<Content>> inputs, std::vector<HostActionFunctor> host_actions)
    {
        auto host_params = params_transform_func(params);
        auto output_content = host_actions[0](*host_params);
        if (output_content && output_transform_func && make_content)
        {
            return make_content(output_transform_func(output_content->as_blob()), params);
        }
        return std::unique_ptr<Content>();
    };
}

ScriptActionDelegate::ParamsTransformFunctor xslt_params_transform(std::unique_ptr<XsltStylesheet> stylesheet)
{
    std::shared_ptr<xml_tools::XsltStylesheet> shared_stylesheet(std::move(stylesheet));
    return [shared_stylesheet](const XmlDoc& doc){
        return xml_tools::xslt_apply_stylesheet(*shared_stylesheet, doc);
    };
}

ScriptActionDelegate::OutputTransformFunctor regex_parser_output_transform(std::string regex_grammar)
{
    auto parser = std::make_shared<xml_tools::regex_parser>(regex_grammar);
    return [parser](const Blob& blob){
        auto parsed = xml_tools::as_string(*parser->parse(std::string(blob.begin(), blob.end())));
        return Blob(parsed.begin(), parsed.end());
    };
}

ScriptActionDelegate::OutputTransformFunctor xslt_output_transform(std::unique_ptr<XsltStylesheet> stylesheet)
{
    std::shared_ptr<xml_tools::XsltStylesheet> shared_stylesheet(std::move(stylesheet)); 
    return [shared_stylesheet](const Blob& blob){
        auto transformed = xml_tools::as_string(*xml_tools::xslt_apply_stylesheet(*shared_stylesheet, *xml_tools::parse(std::string(blob.begin(), blob.end()))));
        return Blob(transformed.begin(), transformed.end());
    };
}

}} //namespace TR { namespace External {