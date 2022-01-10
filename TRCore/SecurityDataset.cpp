#include "stdafx.h"
#include "SecurityDataset.h"
#include "Transaction.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
namespace TR {  namespace Core {

namespace {
struct IdentityIndex: stl_tools::unique_storage_index<AccessEntryIdentity>
{
    static key_type get_key(const AccessEntryData& access_entry_data)
    {
        return access_entry_data.m_identity;
    }
};

struct SourceIndex: stl_tools::single_storage_index<SourceKey>
{
    static key_type get_key(const AccessEntryData& access_entry_data)
    {
        return access_entry_data.m_identity.m_source_key;
    }
};

struct UserIndex: stl_tools::single_storage_index<Key>
{
    static key_type get_key(const AccessEntryData& access_entry_data)
    {
        return access_entry_data.m_identity.m_user_key;
    }
};
} //namespace {

class AccessEntryDataset::Cache: public stl_tools::storage<AccessEntryData, IdentityIndex, SourceIndex, UserIndex>
{
};

AccessEntryDataset::AccessEntryDataset(Database& db):
    m_db(db),
    m_cache(std::make_unique<Cache>())
{
    ReadOnlyTransaction t(m_db);
    for (auto& access_entry_data : m_db.query<AccessEntryData>())
    {
        m_cache->insert(access_entry_data);
    }
}

AccessEntryDataset::~AccessEntryDataset()
{
}

boost::optional<AccessEntryData> AccessEntryDataset::get(Key user_key, SourceKey source_key) const
{
    auto position = m_cache->find({source_key, user_key});
    if (position != m_cache->end())
    {
        return *position;
    }
    return boost::none;
}

void AccessEntryDataset::put(AccessEntryData access_entry_data, Transaction& t)
{
    auto position = m_cache->find(access_entry_data.m_identity);
    if (position != m_cache->end())
    {
        m_db.update(access_entry_data);
        transact_update(*m_cache, access_entry_data, t);
    }
    else
    {
        m_db.persist(access_entry_data);
        transact_insert(*m_cache, access_entry_data, t);
    }
}

void AccessEntryDataset::clear(Key user_key, SourceKey source_key, Transaction& t)
{
    auto position = m_cache->find({source_key, user_key});
    if (position != m_cache->end())
    {
        transact_erase(*m_cache, position, t);
        m_db.erase<AccessEntryData>({source_key, user_key});
    }
}

std::vector<AccessEntryData> AccessEntryDataset::query_source_equal(SourceKey source_key)
{
    auto entries = m_cache->find_range<SourceIndex>(source_key);
    return{entries.begin(), entries.end()};
}

void AccessEntryDataset::erase_source_equal(const std::vector<SourceKey>& source_keys, Transaction& t)
{
    for (auto source_key : source_keys)
    {
        transact_erase_range(*m_cache, m_cache->find_range<SourceIndex>(source_key), t);
    }
    m_db.erase_query<AccessEntryData>(odb::query<AccessEntryData>::identity.source_key.value.in_range(source_keys.begin(), source_keys.end()));
}

void AccessEntryDataset::erase_source_equal(SourceKey source_key, Transaction& t)
{
    transact_erase_range(*m_cache, m_cache->find_range<SourceIndex>(source_key), t);
    m_db.erase_query<AccessEntryData>(odb::query<AccessEntryData>::identity.source_key.value == source_key);
}

void AccessEntryDataset::erase_user_equal(Key user_key, Transaction& t)
{
    transact_erase_range(*m_cache, m_cache->find_range<UserIndex>(user_key), t);
    m_db.erase_query<AccessEntryData>(odb::query<AccessEntryData>::identity.user_key == user_key);
}

}} //namespace TR { namespace Core {