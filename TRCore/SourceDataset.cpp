#include "stdafx.h"
#include "SourceDataset.h"
#include "Transaction.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
#include "SourceData-odb.hxx"
namespace TR {  namespace Core {

namespace {
struct SourceKeyIndex: stl_tools::unique_storage_index<SourceKey>
{
    static key_type get_key(const SourceData& source_data)
    {
        return SourceKey(source_data.m_key);
    }
};

struct ParentIndex: stl_tools::single_storage_index<boost::optional<SourceKey>>
{
    static key_type get_key(const SourceData& source_data)
    {
        return source_data.m_parent_key;
    }
};

struct RoleIndex: stl_tools::single_storage_index<RoleKey>
{
    static key_type get_key(const SourceData& source_data)
    {
        return RoleKey(source_data.m_role_key);
    }
};

struct SourceTypeIndex: stl_tools::single_storage_index<SourceTypeUUID>
{
    static key_type get_key(const SourceData& source_data)
    {
        return SourceTypeUUID(source_data.m_source_type_uuid);
    }
};

} //namespace {

class SourceDataset::Cache: public stl_tools::storage<SourceData, SourceKeyIndex, ParentIndex, RoleIndex, SourceTypeIndex>
{
};

SourceDataset::SourceDataset(Database& db):
    m_db(db),
    m_cache(std::make_unique<Cache>())
{
    ReadOnlyTransaction t(m_db);
    for (auto& source_data : m_db.query<SourceData>())
    {
        m_cache->insert(source_data);
    }
}

SourceDataset::~SourceDataset()
{
}

SourceData SourceDataset::load(SourceKey source_key) const
{
    auto position = m_cache->find(source_key);
    if (position == m_cache->end())
    {
        throw std::logic_error("Source not found");
    }
    return *position;
}

boost::optional<SourceData> SourceDataset::find(SourceKey source_key) const
{
    auto position = m_cache->find(source_key);
    if (position == m_cache->end())
    {
        return boost::none;
    }
    return *position;
}

SourceKey SourceDataset::persist(SourceData source_data, Transaction& t)
{
    source_data.m_key = m_db.persist(source_data);
    transact_insert(*m_cache, source_data, t);
    return SourceKey(source_data.m_key);
}

void SourceDataset::update(const SourceData& source_data, Transaction& t)
{
    m_db.update(source_data);
    transact_update(*m_cache, source_data, t);
}

std::vector<SourceData> SourceDataset::query() const
{
    return {m_cache->begin(), m_cache->end()};
}

std::vector<SourceData> SourceDataset::query_parent_equal(SourceKey parent) const
{
    auto childs = m_cache->find_range<ParentIndex>(parent);
    return {childs.begin(), childs.end()};
}

std::vector<SourceData> SourceDataset::query_role_equal(RoleKey role) const
{
    auto actors = m_cache->find_range<RoleIndex>(role);
    return {actors.begin(), actors.end()};
}

std::vector<SourceData> SourceDataset::query_source_type_equal(SourceTypeUUID source_type_uuid) const
{
    auto sources = m_cache->find_range<SourceTypeIndex>(source_type_uuid);
    return{sources.begin(), sources.end()};
}

void SourceDataset::erase(const std::vector<SourceKey>& source_keys, Transaction& t)
{   
    for (auto source_key : source_keys)
    {
        auto position = m_cache->find(source_key);
        if (position == m_cache->end())
        {
            throw std::logic_error("Source not found");
        }
        transact_erase(*m_cache, position, t);
    }

    m_db.erase_query<SourceData>(odb::query<SourceData>::key.in_range(source_keys.begin(), source_keys.end()));
}

}} //namespace TR { namespace Core {