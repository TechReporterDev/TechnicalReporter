#include "stdafx.h"
#include "Trackings.h"
#include "Transaction.h"
#include "ReportTypes.h"
#include "StreamTypes.h"
namespace TR { namespace Core {

TrackingDirection::TrackingDirection(ReportTypeRef input_ref, StreamTypeRef output_ref):
    m_input_ref(input_ref),
    m_output_ref(output_ref)
{
}

bool operator < (const TrackingDirection& left, const TrackingDirection& right)
{
    if (left.m_input_ref < right.m_input_ref)
        return true;

    if (right.m_input_ref < left.m_input_ref)
        return false;

    return left.m_output_ref < right.m_output_ref;
}

bool operator == (const TrackingDirection& left, const TrackingDirection& right)
{
    return left.m_input_ref == right.m_input_ref && left.m_output_ref == right.m_output_ref;
}

namespace {
struct DirectionIndex: stl_tools::unique_storage_index<TrackingDirection>
{
    static TrackingDirection get_key(const std::shared_ptr<Tracking>& tracking)
    {
        return tracking->get_direction();
    }
};

struct InputIndex: stl_tools::single_storage_index<ReportTypeUUID>
{
    static ReportTypeUUID get_key(const std::shared_ptr<Tracking>& tracking)
    {
        return tracking->get_direction().m_input_ref.get_uuid();
    }
};

struct OutputIndex: stl_tools::single_storage_index<StreamTypeUUID>
{
    static StreamTypeUUID get_key(const std::shared_ptr<Tracking>& tracking)
    {
        return tracking->get_direction().m_output_ref.get_uuid();
    }
};
} //namespace {

class Trackings::Storage: public stl_tools::storage<std::shared_ptr<Tracking>, DirectionIndex, InputIndex, OutputIndex>
{
};

Trackings::Trackings():
    m_report_types(nullptr),
    m_stream_types(nullptr),
    m_storage(std::make_unique<Storage>())
{
}

Trackings::~Trackings()
{
}

void Trackings::set_report_types(ReportTypes* report_types)
{
    m_report_types = report_types;
    m_report_types->connect_remove_report_type([this](ReportTypeUUID report_type_uuid, Transaction& t){
        if (!m_storage->find_range<InputIndex>(report_type_uuid).empty())
        {
            throw Exception(L"Can`t remove 'report_type' until tracking exists");
        }
    }, TRACKINGS_PRIORITY);
}

void Trackings::set_stream_types(StreamTypes* stream_types)
{
    m_stream_types = stream_types;
    m_stream_types->connect_remove_stream_type([this](StreamTypeUUID stream_type_uuid, Transaction& t){
        if (!m_storage->find_range<OutputIndex>(stream_type_uuid).empty())
        {
            throw Exception(L"Can`t remove 'stream_type' until tracking exists");
        }
    }, TRACKINGS_PRIORITY);
}

void Trackings::add_tracking(std::shared_ptr<Tracking> tracking, Transaction& t)
{
    auto direction = tracking->get_direction();
    if (m_storage->find(direction) != m_storage->end())
    {
        throw std::logic_error("Tracking already exists.");
    }
    transact_insert(*m_storage, tracking, t);
    m_add_tracking_sig(direction, t);
}

void Trackings::remove_tracking(const TrackingDirection& direction, Transaction& t)
{
    auto it = m_storage->find(direction);
    if (it == m_storage->end())
    {
        throw std::logic_error("Tracking not found.");
    }
    transact_erase(*m_storage, it, t);
    m_remove_tracking_sig(direction, t);
}

std::shared_ptr<Tracking> Trackings::get_tracking(const TrackingDirection& direction) const
{
    auto it = m_storage->find(direction);
    if (it == m_storage->end())
    {
        throw std::logic_error("Tracking not found.");
    }
    return *it;
}

std::shared_ptr<Tracking> Trackings::find_tracking(const TrackingDirection& direction) const
{
    auto it = m_storage->find(direction);
    if (it == m_storage->end())
    {
        return nullptr;
    }
    return *it;
}

std::vector<std::shared_ptr<Tracking>> Trackings::find_by_input(ReportTypeRef input_ref) const
{
    auto range = m_storage->find_range<InputIndex>(input_ref.get_uuid());
    return{range.begin(), range.end()};
}

std::vector<std::shared_ptr<Tracking>> Trackings::find_by_output(StreamTypeRef output_ref) const
{
    auto range = m_storage->find_range<OutputIndex>(output_ref.get_uuid());
    return{range.begin(), range.end()};
}

}} //namespace TR { namespace Core {