#pragma once
#include "stddecl.h"
#include "TRCore\BasisTrivialTypes.h"
namespace TR { namespace External {

struct XmlStreamTypeDef: XmlNode
{
    StreamTypeUUID                      get_uuid() const;
    std::wstring                        get_name() const;
    const XmlDefNode&                   get_message_def() const;
};

}}// namespace TR { namespace External {