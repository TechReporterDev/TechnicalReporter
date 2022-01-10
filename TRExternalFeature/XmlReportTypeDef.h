#pragma once
#include "stddecl.h"
#include "TRCore\BasisTrivialTypes.h"
namespace TR { namespace External {

struct XmlReportTypeDef: XmlNode
{
    enum class Syntax { PLAIN_TEXT, PLAIN_XML, REGULAR_XML, CHECKLIST_XML };

    ReportTypeUUID              get_uuid() const;
    std::wstring                get_name() const;
    Syntax                      get_syntax() const;
    std::string                 get_definition() const;

    const XmlNode&              get_def_root() const;
    const XmlNode&              get_text_def() const;
    const XmlDefNode&           get_regular_def() const;
};

}}// namespace TR { namespace External {