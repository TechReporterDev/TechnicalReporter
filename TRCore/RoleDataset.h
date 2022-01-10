#pragma once
#include "BackgndFwd.h"
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "RoleData-odb.hxx"
namespace TR { namespace Core {

class RoleDataset
{
public:
    RoleDataset(Database& db);
    ~RoleDataset();

    // single record
    RoleData                    load(RoleKey role_key) const;
    boost::optional<RoleData>   find(RoleKey role_key) const;
    RoleKey                     persist(RoleData role_data, Transaction& t);
    void                        update(const RoleData& role_data, Transaction& t);
    void                        erase(RoleKey role_key, Transaction& t);

    // multiple records
    std::vector<RoleData>       query() const;
    std::vector<RoleData>       query_parent_equal(RoleKey parent) const;
    
private:
    class Cache;

    Database& m_db;
    std::unique_ptr<Cache> m_cache;
};

}} //namespace TR { namespace Core {