#pragma once
#include "BackgndFwd.h"
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "ServicesValues.hxx"
#include "CustomReportData-odb.hxx"
namespace TR { namespace Core {

class QueryJoiningDataset
{
public:
    QueryJoiningDataset(Database& db);
    ~QueryJoiningDataset();

    // single record
    QueryJoiningData                        load(QueryJoiningKey query_joining_key) const;
    boost::optional<QueryJoiningData>       find_by_output(ReportTypeUUID output_uuid) const;
    QueryJoiningKey                         persist(QueryJoiningData query_joining_data, Transaction& t);
    void                                    erase(QueryJoiningKey query_joining_key, Transaction& t);

    // multiple records
    std::vector<QueryJoiningData>           query() const;
    std::vector<QueryJoiningData>           query_input_equal(ReportTypeUUID input_uuid) const;
    

private:
    class Cache;

    Database& m_db;
    std::unique_ptr<Cache> m_cache;
};

class SubjectQueryDataset
{
public:
    SubjectQueryDataset(Database& db);
    ~SubjectQueryDataset();

    // single record
    boost::optional<SubjectQueryData>               get(const SubjectQueryIdentity& identity) const;
    bool                                            exist(const SubjectQueryIdentity& identity) const;
    void                                            put(const SubjectQueryData& subject_query_data, Transaction& t);
    void                                            clear(const SubjectQueryIdentity& identity, Transaction& t) const;

    // multiple records
    void                                            erase_source_equal(const std::vector<SourceKey>& source_keys, Transaction& t);
    void                                            erase_subject_equal(const SubjectIdentity& subject_identity, Transaction& t);
    void                                            erase_query_joining_equal(QueryJoiningKey query_joining_key, Transaction& t);

private:
    class Cache;

    Database& m_db;
    std::unique_ptr<Cache> m_cache;
};

class SubjectJoinedQueryDataset
{
public:
    SubjectJoinedQueryDataset(Database& db);
    ~SubjectJoinedQueryDataset();

    // single record
    boost::optional<SubjectJoinedQueryData>         get(const SubjectQueryIdentity& identity) const;
    bool                                            exist(const SubjectQueryIdentity& identity) const;
    void                                            put(const SubjectJoinedQueryData& subject_query_data, Transaction& t);
    void                                            clear(const SubjectQueryIdentity& identity, Transaction& t) const;

    // multiple records
    void                                            erase_source_equal(const std::vector<SourceKey>& source_keys, Transaction& t);
    void                                            erase_subject_equal(const SubjectIdentity& subject_identity, Transaction& t);
    void                                            erase_query_joining_equal(QueryJoiningKey query_joining_key, Transaction& t);

private:
    class Cache;

    Database& m_db;
    std::unique_ptr<Cache> m_cache;
};

}} //namespace TR { namespace Core {