#pragma once
#include "TRCore\BasisTrivialTypes.h"
#include "stddecl.h"
namespace TR {namespace External {

struct XmlActionOutputDef: XmlNode
{
    enum class Syntax { PLAIN_TEXT, PLAIN_XML, REGULAR_XML };
    Syntax                          get_syntax() const;
    std::string                     get_definition() const;
};

struct XmlActionDef: XmlNode
{
    UUID                            get_uuid() const;
    std::wstring                    get_name() const;
    std::string                     get_params_def() const;
    const XmlNode&                  get_params_root() const;
    const XmlActionOutputDef*       get_output_def() const;
};

struct XmlActionOutputTransformDef: XmlNode
{
    enum class ScriptType { XSLT, REGEX_PARSER };

    ScriptType                          get_script_type() const;
    const XmlNode*                      get_xslt() const;
    std::string                         get_xslt_stylesheet() const;
    const XmlNode*                      get_regex_grammar_node() const;
    std::string                         get_regex_grammar() const;
};

struct XmlActionDelegateDef: XmlNode
{
    ActionUUID                          get_host_action_uuid() const;
    ActionUUID                          get_guest_action_uuid() const;

    const XmlNode&                      get_params_node() const;
    const XmlActionOutputTransformDef*  get_output_transform_def() const;
};

struct XmlActionShortcutDef: XmlNode
{
    UUID                                get_uuid() const;
    std::wstring                        get_name() const;
    ActionUUID                          get_host_action_uuid() const;
    boost::optional<ReportTypeUUID>     get_output_uuid() const;
    const XmlNode&                      get_params_node() const;
};

}}// namespace TR { namespace External {