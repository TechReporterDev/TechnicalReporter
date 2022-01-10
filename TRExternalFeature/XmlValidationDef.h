#pragma once
#include "TRCore\BasisTrivialTypes.h"
#include "stddecl.h"
namespace TR { namespace External {

struct XmlValidationDef: XmlNode
{
    std::wstring        get_name() const;
    ReportTypeUUID      get_input_uuid() const;
    ReportTypeUUID      get_output_uuid() const;
};

}}// namespace TR { namespace External {