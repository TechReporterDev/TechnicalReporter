#pragma once
#include "BackgndFwd.h"
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "SourceData-odb.hxx"
namespace TR { namespace Core {

class SourceDataset
{
public:
    SourceDataset(Database& db);
    ~SourceDataset();

    // single record
    SourceData                  load(SourceKey source_key) const;
    boost::optional<SourceData> find(SourceKey source_key) const;
    SourceKey                   persist(SourceData source_data, Transaction& t);
    void                        update(const SourceData& source_data, Transaction& t);
    
    // multiple records
    std::vector<SourceData>     query() const;
    std::vector<SourceData>     query_parent_equal(SourceKey parent) const;
    std::vector<SourceData>     query_role_equal(RoleKey role) const;
    std::vector<SourceData>     query_source_type_equal(SourceTypeUUID source_type_uuid) const;
    void                        erase(const std::vector<SourceKey>& source_keys, Transaction& t);
    
private:
    class Cache;

    Database& m_db;
    std::unique_ptr<Cache> m_cache;
};

}} //namespace TR { namespace Core {