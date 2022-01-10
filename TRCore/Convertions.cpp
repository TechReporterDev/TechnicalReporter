#include "stdafx.h"
#include "Convertions.h"
#include "Transaction.h"
#include "StreamTypes.h"
namespace TR { namespace Core {

ConvertionDirection::ConvertionDirection(StreamTypeRef input_ref, StreamTypeRef output_ref):
    m_input_ref(input_ref),
    m_output_ref(output_ref)
{
}

bool operator < (const ConvertionDirection& left, const ConvertionDirection& right)
{
    if (left.m_input_ref < right.m_input_ref)
        return true;

    if (right.m_input_ref < left.m_input_ref)
        return false;

    return left.m_output_ref < right.m_output_ref;
}

bool operator == (const ConvertionDirection& left, const ConvertionDirection& right)
{
    return left.m_input_ref == right.m_input_ref && left.m_output_ref == right.m_output_ref;
}

namespace {
struct DirectionIndex: stl_tools::unique_storage_index<ConvertionDirection>
{
    static ConvertionDirection get_key(const std::shared_ptr<Convertion>& conversion)
    {
        return conversion->get_direction();
    }
};

struct InputIndex: stl_tools::single_storage_index<StreamTypeUUID>
{
    static StreamTypeUUID get_key(const std::shared_ptr<Convertion>& conversion)
    {
        return conversion->get_direction().m_input_ref.get_uuid();
    }
};

struct OutputIndex: stl_tools::single_storage_index<StreamTypeUUID>
{
    static StreamTypeUUID get_key(const std::shared_ptr<Convertion>& conversion)
    {
        return conversion->get_direction().m_output_ref.get_uuid();
    }
};
} //namespace {

class Convertions::Storage: public stl_tools::storage<std::shared_ptr<Convertion>, DirectionIndex, InputIndex, OutputIndex>
{
};

Convertions::Convertions():
    m_stream_types(nullptr),
    m_storage(std::make_unique<Storage>())
{
}

Convertions::~Convertions()
{
}

void Convertions::set_stream_types(StreamTypes* stream_types)
{
    m_stream_types = stream_types;
    m_stream_types->connect_remove_stream_type([this](StreamTypeUUID stream_type_uuid, Transaction& t){
        if (!m_storage->find_range<InputIndex>(stream_type_uuid).empty())
        {
            throw Exception(L"Can`t remove stream type until selection exists");
        }

        if (!m_storage->find_range<OutputIndex>(stream_type_uuid).empty())
        {
            throw Exception(L"Can`t remove stream type until selection exists");
        }
    }, CONVERTIONS_PRIORITY);
}

void Convertions::add_convertion(std::shared_ptr<Convertion> convertion, Transaction& t)
{
    auto direction = convertion->get_direction();
    if (m_storage->find(direction) != m_storage->end())
    {
        throw std::logic_error("Selection already exists.");
    }
    transact_insert(*m_storage, convertion, t);
    m_add_convertion_sig(direction, t);
}

void Convertions::remove_convertion(const ConvertionDirection& direction, Transaction& t)
{
    auto it = m_storage->find(direction);
    if (it == m_storage->end())
    {
        throw std::logic_error("Convertion not found.");
    }
    transact_erase(*m_storage, it, t);
    m_remove_convertion_sig(direction, t);
}

std::shared_ptr<Convertion> Convertions::get_convertion(const ConvertionDirection& direction) const
{
    auto it = m_storage->find(direction);
    if (it == m_storage->end())
    {
        throw std::logic_error("Convertion not found.");
    }
    return *it;
}

std::shared_ptr<Convertion> Convertions::find_convertion(const ConvertionDirection& direction) const
{
    auto it = m_storage->find(direction);
    if (it == m_storage->end())
    {
        return nullptr;
    }
    return *it;
}

std::vector<std::shared_ptr<Convertion>> Convertions::find_by_input(StreamTypeRef input_ref) const
{
    auto range = m_storage->find_range<InputIndex>(input_ref.get_uuid());
    return{range.begin(), range.end()};
}

std::vector<std::shared_ptr<Convertion>> Convertions::find_by_output(StreamTypeRef output_ref) const
{
    auto range = m_storage->find_range<OutputIndex>(output_ref.get_uuid());
    return{range.begin(), range.end()};
}

}} //namespace TR { namespace Core {