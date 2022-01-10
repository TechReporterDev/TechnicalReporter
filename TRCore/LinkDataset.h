#pragma once
#include "BackgndFwd.h"
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "LinkData-odb.hxx"
namespace TR { namespace Core {

class LinkDataset
{
public:
    LinkDataset(Database& db);
    ~LinkDataset();

    // single record
    void                        persist(LinkData link_data, Transaction& t);
    void                        erase(LinkID link_id, Transaction& t);

    // multiple records
    std::vector<LinkData>       query() const;
    std::vector<LinkData>       query_parent_equal(SourceKey parent_key) const;
    std::vector<LinkData>       query_child_equal(SourceKey child_key) const;
    void                        erase_parent_equal(const std::vector<SourceKey>& removed_sources, Transaction& t);
    
private:
    class Cache;

    Database& m_db;
    std::unique_ptr<Cache> m_cache;
};

}} //namespace TR { namespace Core {