#include "stdafx.h"
#include "StaticActionShortcut.h"
#include "TRCore\RegistryRefs.h"
#include "W32Cmd.h"
namespace TR { namespace External {

StaticActionShortcut::StaticActionShortcut(UUID uuid, std::wstring name, ActionRef action_ref, boost::optional<ReportTypeRef> output_ref, std::unique_ptr<XmlDoc> params):
    ActionShortcut(uuid, std::move(name), action_ref, output_ref),
    m_params(std::move(params))
{
}

std::unique_ptr<XmlDoc> StaticActionShortcut::get_action_params(SourceRef /*source*/) const
{
    return xml_tools::clone_doc(*m_params);
}

}} //namespace TR { namespace External {