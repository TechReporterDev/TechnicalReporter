#include "stdafx.h"
#include "RoleDataset.h"
#include "Transaction.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
namespace TR {  namespace Core {

namespace {
struct RoleKeyIndex: stl_tools::unique_storage_index<RoleKey>
{
    static key_type get_key(const RoleData& role_data)
    {
        return RoleKey(role_data.m_key);
    }
};

struct ParentIndex: stl_tools::single_storage_index<boost::optional<RoleKey>>
{
    static key_type get_key(const RoleData& role_data)
    {
        return role_data.m_parent_key;
    }
};

} //namespace {

class RoleDataset::Cache: public stl_tools::storage<RoleData, RoleKeyIndex, ParentIndex>
{
};

RoleDataset::RoleDataset(Database& db):
    m_db(db),
    m_cache(std::make_unique<Cache>())
{
    ReadOnlyTransaction t(m_db);
    for (auto& role_data : m_db.query<RoleData>())
    {
        m_cache->insert(role_data);
    }
}

RoleDataset::~RoleDataset()
{
}

RoleData RoleDataset::load(RoleKey role_key) const
{
    auto position = m_cache->find(role_key);
    if (position == m_cache->end())
    {
        throw std::logic_error("Role not found");
    }
    return *position;
}

boost::optional<RoleData> RoleDataset::find(RoleKey role_key) const
{
    auto position = m_cache->find(role_key);
    if (position == m_cache->end())
    {
        return boost::none;
    }
    return *position;
}

RoleKey RoleDataset::persist(RoleData role_data, Transaction& t)
{
    role_data.m_key = m_db.persist(role_data);
    transact_insert(*m_cache, role_data, t);
    return RoleKey(role_data.m_key);
}

void RoleDataset::update(const RoleData& role_data, Transaction& t)
{
    m_db.update(role_data);
    transact_update(*m_cache, role_data, t);
}

void RoleDataset::erase(RoleKey role_key, Transaction& t)
{
    auto position = m_cache->find(role_key);
    if (position == m_cache->end())
    {
        throw std::logic_error("Role not found");
    }
    transact_erase(*m_cache, position, t);
    m_db.erase<RoleData>(role_key);
}

std::vector<RoleData> RoleDataset::query() const
{
    return {m_cache->begin(), m_cache->end()};
}

std::vector<RoleData> RoleDataset::query_parent_equal(RoleKey parent) const
{
    auto childs = m_cache->find_range<ParentIndex>(parent);
    return {childs.begin(), childs.end()};
}

}} //namespace TR { namespace Core {