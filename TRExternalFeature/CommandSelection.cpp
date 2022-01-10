#include "stdafx.h"
#include "CommandSelection.h"
#include "TRCore\ReportTypes.h"
#include "TRCore\Content.h"
#include "TRCore\RegistryRefs.h"
#include "W32Cmd.h"

namespace TR { namespace External {

CommandSelection::CommandSelection(UUID uuid, SelectionPlan selection_plan, std::wstring command):
    Selection(std::move(uuid)),
    m_plan(selection_plan),
    m_command(std::move(command))
{
}

SelectionPlan CommandSelection::get_plan() const
{
    return m_plan;
}

CommandSelection::Functor CommandSelection::prepare_functor(SourceRef /*source_ref*/) const
{
    auto command = m_command;
    auto make_content = (*m_plan.m_output_ref).prepare_make_content_from_blob();
    
    return [command, make_content](const std::vector<std::shared_ptr<Content>>& content){
        _ASSERT(!content.empty());
        return make_content(exec_command(command, content[0]->as_blob()));
    };
}

}} //namespace TR { namespace External {