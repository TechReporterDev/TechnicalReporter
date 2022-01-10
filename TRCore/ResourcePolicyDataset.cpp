#include "stdafx.h"
#include "ResourcePolicyDataset.h"
#include "Transaction.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
namespace TR {  namespace Core {

namespace {
struct IdentityIndex: stl_tools::unique_storage_index<ResourcePolicyIdentity>
{
    static key_type get_key(const ResourcePolicyData& resource_policy_data)
    {
        return resource_policy_data.m_identity;
    }
};

struct SubjectIndex: stl_tools::single_storage_index<SubjectIdentity>
{
    static key_type get_key(const ResourcePolicyData& resource_policy_data)
    {
        return resource_policy_data.m_identity.m_subject;
    }
};

struct ResourceIndex: stl_tools::single_storage_index<ResourceIdentity>
{
    static key_type get_key(const ResourcePolicyData& resource_policy_data)
    {
        return resource_policy_data.m_identity.m_resource;
    }
};
} //namespace {

class ResourcePolicyDataset::Cache: public stl_tools::storage<ResourcePolicyData, IdentityIndex, SubjectIndex, ResourceIndex>
{
};

ResourcePolicyDataset::ResourcePolicyDataset(Database& db):
    m_db(db),
    m_cache(std::make_unique<Cache>())
{
    ReadOnlyTransaction t(m_db);
    for (auto& resource_policy_data : m_db.query<ResourcePolicyData>())
    {
        m_cache->insert(resource_policy_data);
    }
}

ResourcePolicyDataset::~ResourcePolicyDataset()
{
}

ResourcePolicyData ResourcePolicyDataset::get(const SubjectIdentity& subject_identity, const ResourceIdentity& resource_identity) const
{
    auto position = m_cache->find({subject_identity, resource_identity});
    if (position != m_cache->end())
    {
        return *position;
    }
    return ResourcePolicyData({subject_identity, resource_identity});
}

void ResourcePolicyDataset::put(const ResourcePolicyData& resource_policy_data, Transaction& t)
{
    auto position = m_cache->find(resource_policy_data.m_identity);
    if (position != m_cache->end())
    {
        if (!resource_policy_data.is_default())
        {
            m_db.update(resource_policy_data);
            transact_update(*m_cache, resource_policy_data, t); 
        }
        else
        {
            m_db.erase<ResourcePolicyData>(resource_policy_data.m_identity);
            transact_erase(*m_cache, position, t);
        }
    }
    else
    {
        if (!resource_policy_data.is_default())
        {
            m_db.persist(resource_policy_data);
            transact_insert(*m_cache, resource_policy_data, t);         
        }
    }
}

void ResourcePolicyDataset::erase_source_equal(const std::vector<SourceKey>& source_keys, Transaction& t)
{
    for (auto source_key : source_keys)
    {
        transact_erase_range(*m_cache, m_cache->find_range<SubjectIndex>(source_key), t);
    }
    m_db.erase_query<ResourcePolicyData>(odb::query<ResourcePolicyData>::identity.subject.value1.value.in_range(source_keys.begin(), source_keys.end()));
}

void ResourcePolicyDataset::erase_subject_equal(const SubjectIdentity& subject_identity, Transaction& t)
{
    transact_erase_range(*m_cache, m_cache->find_range<SubjectIndex>(subject_identity), t);
    m_db.erase_query<ResourcePolicyData>(
        odb::query<ResourcePolicyData>::identity.subject.value1.value == subject_identity.m_value1 &&
        odb::query<ResourcePolicyData>::identity.subject.value2.value == subject_identity.m_value2);
}

void ResourcePolicyDataset::erase_resource_equal(const ResourceIdentity& resource_identity, Transaction& t)
{
    transact_erase_range(*m_cache, m_cache->find_range<ResourceIndex>(resource_identity), t);
    m_db.erase_query<ResourcePolicyData>(
        odb::query<ResourcePolicyData>::identity.resource.value1.value == resource_identity.m_value1 &&
        odb::query<ResourcePolicyData>::identity.resource.value2.value == resource_identity.m_value2);
}

}} //namespace TR { namespace Core {