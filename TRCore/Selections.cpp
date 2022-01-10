#include "stdafx.h"
#include "Selections.h"
#include "Transaction.h"
#include "ReportTypes.h"
namespace TR { namespace Core {

SelectionDirection::SelectionDirection(ReportTypeRef input_ref, ReportTypeRef output_ref):
    SelectionDirection(std::vector<ReportTypeRef>{input_ref}, output_ref)
{
}

SelectionDirection::SelectionDirection(std::vector<ReportTypeRef> input_refs, ReportTypeRef output_ref) :
    m_input_refs(std::move(input_refs)),
    m_output_ref(output_ref)
{
}

bool operator < (const SelectionDirection& left, const SelectionDirection& right)
{
    if (left.m_input_refs < right.m_input_refs)
        return true;

    if (right.m_input_refs < left.m_input_refs)
        return false;

    return left.m_output_ref < right.m_output_ref;
}

bool operator == (const SelectionDirection& left, const SelectionDirection& right)
{
    return left.m_input_refs == right.m_input_refs && left.m_output_ref == right.m_output_ref;
}

bool operator < (const SelectionInput& left, const SelectionInput& right)
{
    if (left.m_report_type_ref < right.m_report_type_ref)
        return true;

    if (right.m_report_type_ref < left.m_report_type_ref)
        return false;
    
    return left.m_mode < right.m_mode;
}

bool operator == (const SelectionInput& left, const SelectionInput& right)
{
    return left.m_report_type_ref == right.m_report_type_ref && left.m_mode == right.m_mode;
}

namespace {
struct DirectionIndex: stl_tools::unique_storage_index<SelectionDirection>
{
    static SelectionDirection get_key(const std::shared_ptr<Selection>& selection)
    {
        return get_direction(selection->get_plan());
    }
};

struct InputIndex: stl_tools::multi_storage_index<ReportTypeUUID>
{
    template<class F>
    static void enum_keys(const std::shared_ptr<Selection>& selection, F f)
    {
        auto selection_plan = selection->get_plan();
        for (auto& input_ref : selection_plan.m_input_refs)
        {
            f(input_ref.m_report_type_ref.get_uuid());
        }
    }
};

struct OutputIndex: stl_tools::single_storage_index<ReportTypeUUID>
{
    static ReportTypeUUID get_key(const std::shared_ptr<Selection>& selection)
    {
        return selection->get_plan().m_output_ref.get_uuid();
    }
};
} //namespace {

class Selections::Storage: public stl_tools::storage<std::shared_ptr<Selection>, DirectionIndex, InputIndex, OutputIndex>
{
};

Selections::Selections():
    m_report_types(nullptr),
    m_storage(std::make_unique<Storage>())
{
}

Selections::~Selections()
{
}

void Selections::set_report_types(ReportTypes* report_types)
{
    m_report_types = report_types;
    m_report_types->connect_remove_report_type([this](ReportTypeUUID report_type_uuid, Transaction& t){
        if (!m_storage->find_range<InputIndex>(report_type_uuid).empty())
        {
            throw Exception(L"Can`t remove 'report_type' until selection exists");
        }

        if (!m_storage->find_range<OutputIndex>(report_type_uuid).empty())
        {
            throw Exception(L"Can`t remove 'report_type' until selection exists");
        }
    }, SELECTIONS_PRIORITY);
}

void Selections::add_selection(std::shared_ptr<Selection> selection, Transaction& t)
{
    auto direction = get_direction(selection->get_plan());
    if (m_storage->find(direction) != m_storage->end())
    {
        throw std::logic_error("Selection already exists.");
    }
    transact_insert(*m_storage, selection, t);
    m_add_selection_sig(direction, t);
}

void Selections::remove_selection(const SelectionDirection& direction, Transaction& t)
{
    auto it = m_storage->find(direction);
    if (it == m_storage->end())
    {
        throw std::logic_error("Selection not found.");
    }
    transact_erase(*m_storage, it, t);
    m_remove_selection_sig(direction, t);
}

std::shared_ptr<Selection> Selections::get_selection(const SelectionDirection& direction) const
{
    auto it = m_storage->find(direction);
    if (it == m_storage->end())
    {
        throw std::logic_error("Transformation not found.");
    }
    return *it;
}

std::shared_ptr<Selection> Selections::find_selection(const SelectionDirection& direction) const
{
    auto it = m_storage->find(direction);
    if (it == m_storage->end())
    {
        return nullptr;
    }
    return *it;
}

std::vector<std::shared_ptr<Selection>> Selections::find_by_input(ReportTypeRef input_ref) const
{
    auto range = m_storage->find_range<InputIndex>(input_ref.get_uuid());
    return{range.begin(), range.end()};
}

std::vector<std::shared_ptr<Selection>> Selections::find_by_output(ReportTypeRef output_ref) const
{
    auto range = m_storage->find_range<OutputIndex>(output_ref.get_uuid());
    return{range.begin(), range.end()};
}

}} //namespace TR { namespace Core {