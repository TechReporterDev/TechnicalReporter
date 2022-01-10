#pragma once
#include "TRCore\BasisTrivialTypes.h"
#include "stddecl.h"
namespace TR { namespace External {

struct XmlPreprocessorNode: XmlNode
{
};

struct XmlRegexReplaceNode: XmlPreprocessorNode
{
    std::string     get_pattern() const;
    std::string     get_replacement() const;
};

struct XmlSelectionScriptNode: XmlNode
{
    enum class ScriptType { COMMAND, XSLT, REGEX_PARSER };

    ScriptType          get_script_type() const;
    std::wstring        get_command() const;
    const XmlNode*      get_xslt() const;
    const XmlNode*      get_regex_parser() const;   
};

struct XmlSelectionInputNode : XmlNode
{
    enum class Mode { STRICT_MODE = 0, NULLABLE_MODE, OPTIONAL_MODE };

    Mode            get_mode() const;
    ReportTypeUUID  get_uuid() const;
};

struct XmlSelectionDef: XmlNode
{
    static const XmlSelectionInputNode* next_input(const XmlSelectionInputNode* input_node);
    using XmlSelectionInputIterator = xml_tools::node_iterator<const XmlSelectionInputNode, next_input>;
    using XmlSelectionInputRange = boost::iterator_range<XmlSelectionInputIterator>;

    static const XmlPreprocessorNode* next_preprocessor(const XmlPreprocessorNode* preprocessor_node);
    using ConstPreprocessorIterator = xml_tools::node_iterator<const XmlPreprocessorNode, next_preprocessor>;
    using ConstPreprocessorRange = boost::iterator_range<ConstPreprocessorIterator>;

    UUID                                get_uuid() const;
    XmlSelectionInputRange              get_input() const;
    ReportTypeUUID                      get_output_uuid() const;
    const XmlSelectionScriptNode&       get_script() const;
    ConstPreprocessorRange              get_preprocessors() const;
};

}}// namespace TR { namespace External {