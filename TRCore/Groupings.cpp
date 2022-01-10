#include "stdafx.h"
#include "Groupings.h"
#include "Transaction.h"
#include "ReportTypes.h"
namespace TR { namespace Core {

GroupingDirection::GroupingDirection(ReportTypeRef input_ref, ReportTypeRef output_ref):
    m_input_ref(input_ref),
    m_output_ref(output_ref)
{
}

bool operator < (const GroupingDirection& left, const GroupingDirection& right)
{
    if (left.m_input_ref < right.m_input_ref)
        return true;

    if (right.m_input_ref < left.m_input_ref)
        return false;

    return left.m_output_ref < right.m_output_ref;
}

bool operator == (const GroupingDirection& left, const GroupingDirection& right)
{
    return left.m_input_ref == right.m_input_ref && left.m_output_ref == right.m_output_ref;
}

namespace {
struct DirectionIndex: stl_tools::unique_storage_index<GroupingDirection>
{
    static GroupingDirection get_key(const std::shared_ptr<Grouping>& grouping)
    {
        return grouping->get_direction();
    }
};

struct InputIndex: stl_tools::single_storage_index<ReportTypeUUID>
{
    static ReportTypeUUID get_key(const std::shared_ptr<Grouping>& grouping)
    {
        return grouping->get_direction().m_input_ref.get_uuid();
    }
};

struct OutputIndex: stl_tools::single_storage_index<ReportTypeUUID>
{
    static ReportTypeUUID get_key(const std::shared_ptr<Grouping>& grouping)
    {
        return grouping->get_direction().m_output_ref.get_uuid();
    }
};
} //namespace {

class Groupings::Storage: public stl_tools::storage<std::shared_ptr<Grouping>, DirectionIndex, InputIndex, OutputIndex>
{
};

Groupings::Groupings():
    m_report_types(nullptr),
    m_storage(std::make_unique<Storage>())
{
}

Groupings::~Groupings()
{
}

void Groupings::set_report_types(ReportTypes* report_types)
{
    m_report_types = report_types;
    m_report_types->connect_remove_report_type([this](ReportTypeUUID report_type_uuid, Transaction& t){
        if (!m_storage->find_range<InputIndex>(report_type_uuid).empty())
        {
            throw Exception(L"Can`t remove 'report_type' until grouping exists");
        }

        if (!m_storage->find_range<OutputIndex>(report_type_uuid).empty())
        {
            throw Exception(L"Can`t remove 'report_type' until grouping exists");
        }
    }, GROUPINGS_PRIORITY);
}

void Groupings::add_grouping(std::shared_ptr<Grouping> grouping, Transaction& t)
{
    auto direction = grouping->get_direction();
    if (m_storage->find(direction) != m_storage->end())
    {
        throw std::logic_error("Grouping already exists.");
    }
    transact_insert(*m_storage, grouping, t);
    m_add_grouping_sig(direction, t);
}

void Groupings::remove_grouping(const GroupingDirection& direction, Transaction& t)
{
    auto it = m_storage->find(direction);
    if (it == m_storage->end())
    {
        throw std::logic_error("Grouping not found.");
    }
    transact_erase(*m_storage, it, t);
    m_remove_grouping_sig(direction, t);
}

std::vector<std::shared_ptr<Grouping>> Groupings::get_groupings() const
{
    return stl_tools::copy_vector(*m_storage);
}

std::shared_ptr<Grouping> Groupings::get_grouping(const GroupingDirection& direction) const
{
    auto it = m_storage->find(direction);
    if (it == m_storage->end())
    {
        throw std::logic_error("Grouping not found.");
    }
    return *it;
}

std::shared_ptr<Grouping> Groupings::find_grouping(const GroupingDirection& direction) const
{
    auto it = m_storage->find(direction);
    if (it == m_storage->end())
    {
        return nullptr;
    }
    return *it;
}

std::vector<std::shared_ptr<Grouping>> Groupings::find_by_input(ReportTypeRef input_ref) const
{
    auto range = m_storage->find_range<InputIndex>(input_ref.get_uuid());
    return{range.begin(), range.end()};
}

std::vector<std::shared_ptr<Grouping>> Groupings::find_by_output(ReportTypeRef output_ref) const
{
    auto range = m_storage->find_range<OutputIndex>(output_ref.get_uuid());
    return{range.begin(), range.end()};
}

}} //namespace TR { namespace Core {