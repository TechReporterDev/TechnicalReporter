#include "stdafx.h"
#include "StreamDataset.h"
#include "Transaction.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
namespace TR {  namespace Core {

namespace {
struct IdentityIndex: stl_tools::unique_storage_index<StreamIdentity>
{
    static key_type get_key(const StreamData& stream_data)
    {
        return stream_data.m_identity;
    }
};

struct SourceIndex: stl_tools::single_storage_index<SourceKey>
{
    static key_type get_key(const StreamData& stream_data)
    {
        return stream_data.m_identity.m_source_key;
    }
};

struct StreamTypeIndex: stl_tools::single_storage_index<StreamTypeUUID>
{
    static key_type get_key(const StreamData& stream_data)
    {
        return stream_data.m_identity.m_stream_type_uuid;
    }
};
} //namespace {

class StreamDataset::Cache: public stl_tools::storage<StreamData, IdentityIndex, SourceIndex, StreamTypeIndex>
{
};

StreamDataset::StreamDataset(Database& db):
    m_db(db),
    m_cache(std::make_unique<Cache>())
{
    ReadOnlyTransaction t(m_db);
    for (auto& stream_data : m_db.query<StreamData>())
    {
        m_cache->insert(stream_data);
    }
}

StreamDataset::~StreamDataset()
{
}

StreamData StreamDataset::get(SourceKey source_key, StreamTypeUUID stream_type_uuid) const
{
    auto position = m_cache->find({ source_key, stream_type_uuid });
    if (position != m_cache->end())
    {
        return *position;
    }
    return StreamData({ source_key, stream_type_uuid });
}

void StreamDataset::put(StreamData stream_data, Transaction& t)
{
    auto position = m_cache->find(stream_data.m_identity);
    if (position != m_cache->end())
    {
        m_db.update(stream_data);
        transact_update(*m_cache, stream_data, t);
    }
    else
    {
        m_db.persist(stream_data);
        transact_insert(*m_cache, stream_data, t);
    }
}

void StreamDataset::erase_source_equal(const std::vector<SourceKey>& source_keys, Transaction& t)
{
    for (auto source_key : source_keys)
    {
        transact_erase_range(*m_cache, m_cache->find_range<SourceIndex>(source_key), t);
    }
    m_db.erase_query<StreamData>(odb::query<StreamData>::identity.source_key.value.in_range(source_keys.begin(), source_keys.end()));
}

void StreamDataset::erase_stream_type_equal(StreamTypeUUID stream_type_uuid, Transaction& t)
{
    transact_erase_range(*m_cache, m_cache->find_range<StreamTypeIndex>(stream_type_uuid), t);
    m_db.erase_query<StreamData>(odb::query<StreamData>::identity.stream_type_uuid.value == stream_type_uuid);
}

}} //namespace TR { namespace Core {