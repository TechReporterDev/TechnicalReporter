#pragma once
#include "TRCore\Selections.h"
#include "stddecl.h"
namespace TR { namespace External {

class CommandSelection: public Selection
{
    public:
        CommandSelection(UUID uuid, SelectionPlan selection_plan, std::wstring command);

        virtual SelectionPlan       get_plan() const override;
        virtual Functor             prepare_functor(SourceRef /*source_ref*/) const override;

private:
    SelectionPlan m_plan;
    std::wstring m_command;     
};

}} //namespace TR { namespace External {
