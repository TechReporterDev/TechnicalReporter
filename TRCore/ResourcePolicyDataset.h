#pragma once
#include "BackgndFwd.h"
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "ReportPolicyData-odb.hxx"
namespace TR { namespace Core {

class ResourcePolicyDataset
{
public:
    ResourcePolicyDataset(Database& db);
    ~ResourcePolicyDataset();

    // single record
    ResourcePolicyData  get(const SubjectIdentity& subject_identity, const ResourceIdentity& resource_identity) const;
    void                put(const ResourcePolicyData& resource_policy_data, Transaction& t);
    
    // multiple records
    void                erase_source_equal(const std::vector<SourceKey>& source_keys, Transaction& t);
    void                erase_subject_equal(const SubjectIdentity& subject_identity, Transaction& t);
    void                erase_resource_equal(const ResourceIdentity& resource_identity, Transaction& t);

private:
    class Cache;

    Database& m_db;
    std::unique_ptr<Cache> m_cache;
};

}} //namespace TR { namespace Core {