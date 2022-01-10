#pragma once
#include "TRCore\ActionShortcuts.h"
#include "stddecl.h"

namespace TR {namespace External {

class StaticActionShortcut: public ActionShortcut
{
public:
    StaticActionShortcut(UUID uuid, std::wstring name, ActionRef action_ref, boost::optional<ReportTypeRef> output_ref, std::unique_ptr<XmlDoc> params);
    virtual std::unique_ptr<XmlDoc> get_action_params(SourceRef source) const override;

private:
    std::unique_ptr<XmlDoc> m_params;
};

}} //namespace TR { namespace External {