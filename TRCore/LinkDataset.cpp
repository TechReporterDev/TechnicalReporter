#include "stdafx.h"
#include "LinkDataset.h"
#include "Transaction.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
namespace TR {  namespace Core {

namespace {
struct LinkIndex: stl_tools::unique_storage_index<LinkID>
{
    static LinkID get_key(const LinkData& link_data)
    {
        return LinkID(link_data.m_parent_key, link_data.m_child_key);
    }
};

struct ParentIndex: stl_tools::single_storage_index<SourceKey>
{
    static SourceKey get_key(const LinkData& link_data)
    {
        return link_data.m_parent_key;
    }
};

struct ChildIndex: stl_tools::single_storage_index<SourceKey>
{
    static SourceKey get_key(const LinkData& link_data)
    {
        return link_data.m_child_key;
    }
};
} //namespace {

class LinkDataset::Cache: public stl_tools::storage<LinkData, LinkIndex, ParentIndex, ChildIndex>
{
};

LinkDataset::LinkDataset(Database& db):
    m_db(db),
    m_cache(std::make_unique<Cache>())
{
    ReadOnlyTransaction t(m_db);
    for (auto& link_data : m_db.query<LinkData>())
    {
        m_cache->insert(link_data);
    }
}

LinkDataset::~LinkDataset()
{
}

void LinkDataset::persist(LinkData link_data, Transaction& t)
{
    m_db.persist(link_data);
    transact_insert(*m_cache, link_data, t);
}

void LinkDataset::erase(LinkID link_id, Transaction& t)
{
    auto position = m_cache->find(link_id);
    if (position == m_cache->end())
    {
        throw std::logic_error("Link not found");
    }
    transact_erase(*m_cache, position, t);
    m_db.erase<LinkData>(link_id);
}

std::vector<LinkData> LinkDataset::query() const
{
    return {m_cache->begin(), m_cache->end()};
}

std::vector<LinkData> LinkDataset::query_parent_equal(SourceKey parent_key) const
{
    auto childs = m_cache->find_range<ParentIndex>(parent_key);
    return {childs.begin(), childs.end()};
}

std::vector<LinkData> LinkDataset::query_child_equal(SourceKey child_key) const
{
    auto childs = m_cache->find_range<ChildIndex>(child_key);
    return {childs.begin(), childs.end()};
}

void LinkDataset::erase_parent_equal(const std::vector<SourceKey>& removed_sources, Transaction& t)
{
    for (auto& removed_source : removed_sources)
    {
        transact_erase_range(*m_cache, m_cache->find_range<ParentIndex>(removed_source), t);
    }

    m_db.erase_query<LinkData>(odb::query<LinkData>::ID.first.value.in_range(removed_sources.begin(), removed_sources.end()));
}

}} //namespace TR { namespace Core {