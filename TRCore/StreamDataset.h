#pragma once
#include "BackgndFwd.h"
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "StreamData-odb.hxx"
namespace TR { namespace Core {

class StreamDataset
{
public:
    StreamDataset(Database& db);
    ~StreamDataset();

    // single record
    StreamData                  get(SourceKey source_key, StreamTypeUUID stream_type_uuid) const;
    void                        put(StreamData stream_data, Transaction& t);

    void                        erase_source_equal(const std::vector<SourceKey>& source_keys, Transaction& t);
    void                        erase_stream_type_equal(StreamTypeUUID stream_type_uuid, Transaction& t);

private:
    class Cache;

    Database& m_db;
    std::unique_ptr<Cache> m_cache;
};

}} //namespace TR { namespace Core {