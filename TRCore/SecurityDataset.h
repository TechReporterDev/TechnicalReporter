#pragma once
#include "BackgndFwd.h"
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "ServicesValues.hxx"
#include "SecurityData-odb.hxx"
namespace TR { namespace Core {

class AccessEntryDataset
{
public:
    AccessEntryDataset(Database& db);
    ~AccessEntryDataset();

    //single record
    boost::optional<AccessEntryData>    get(Key user_key, SourceKey source_key) const;
    void                                put(AccessEntryData access_entry_data, Transaction& t);
    void                                clear(Key user_key, SourceKey source_key, Transaction& t);

    //multiple records
    std::vector<AccessEntryData>        query_source_equal(SourceKey source_key);
    void                                erase_source_equal(const std::vector<SourceKey>& source_keys, Transaction& t);
    void                                erase_source_equal(SourceKey source_key, Transaction& t);
    void                                erase_user_equal(Key user_key, Transaction& t);     

private:
    class Cache;

    Database& m_db;
    std::unique_ptr<Cache> m_cache;
};

}} //namespace TR { namespace Core {