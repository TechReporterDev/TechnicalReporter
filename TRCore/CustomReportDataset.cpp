#include "stdafx.h"
#include "CustomReportDataset.h"
#include "Transaction.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
namespace TR {  namespace Core {

namespace {
struct QueryJoiningIndex: stl_tools::unique_storage_index<QueryJoiningKey>
{
    static key_type get_key(const QueryJoiningData& query_joining_data)
    {
        return QueryJoiningKey(query_joining_data.m_key);
    }
};

struct InputIndex: stl_tools::single_storage_index<ReportTypeUUID>
{
    static key_type get_key(const QueryJoiningData& query_joining_data)
    {
        return query_joining_data.m_input_uuid;
    }
};

struct OutputIndex: stl_tools::unique_storage_index<ReportTypeUUID>
{
    static key_type get_key(const QueryJoiningData& query_joining_data)
    {
        return query_joining_data.m_output_uuid;
    }
};

struct SubjectQueryIndex: stl_tools::unique_storage_index<SubjectQueryIdentity>
{
    static key_type get_key(const SubjectQueryView& subject_query_view)
    {
        return subject_query_view.m_identity;
    }

    static key_type get_key(const SubjectJoinedQueryView& subject_joined_query_view)
    {
        return subject_joined_query_view.m_identity;
    }
};

struct SubjectIndex: stl_tools::single_storage_index<SubjectIdentity>
{
    static key_type get_key(const SubjectQueryView& subject_query_view)
    {
        return subject_query_view.m_identity.m_subject;
    }

    static key_type get_key(const SubjectJoinedQueryView& subject_joined_query_view)
    {
        return subject_joined_query_view.m_identity.m_subject;
    }
};

struct QueryJoiningKeyIndex: stl_tools::single_storage_index<QueryJoiningKey>
{
    static key_type get_key(const SubjectQueryView& subject_query_view)
    {
        return subject_query_view.m_identity.m_query_joining_key;
    }

    static key_type get_key(const SubjectJoinedQueryView& subject_joined_query_view)
    {
        return subject_joined_query_view.m_identity.m_query_joining_key;
    }
};
} //namespace {

class QueryJoiningDataset::Cache: public stl_tools::storage<QueryJoiningData, QueryJoiningIndex, InputIndex, OutputIndex>
{
};

QueryJoiningDataset::QueryJoiningDataset(Database& db):
    m_db(db),
    m_cache(std::make_unique<Cache>())
{
    ReadOnlyTransaction t(m_db);
    for (auto& query_joining_data : m_db.query<QueryJoiningData>())
    {
        m_cache->insert(query_joining_data);
    }
}

QueryJoiningDataset::~QueryJoiningDataset()
{
}

QueryJoiningData QueryJoiningDataset::load(QueryJoiningKey query_joining_key) const
{
    auto position = m_cache->find(query_joining_key);
    if (position == m_cache->end())
    {
        throw std::logic_error("Query not found");
    }
    return *position;
}

boost::optional<QueryJoiningData> QueryJoiningDataset::find_by_output(ReportTypeUUID output_uuid) const
{
    auto position = m_cache->find<OutputIndex>(output_uuid);
    if (position == m_cache->end())
    {
        return boost::none;
    }
    return *position;
}

QueryJoiningKey QueryJoiningDataset::persist(QueryJoiningData query_joining_data, Transaction& t)
{
    query_joining_data.m_key = m_db.persist(query_joining_data);
    transact_insert(*m_cache, query_joining_data, t);
    return QueryJoiningKey(query_joining_data.m_key);
}

void QueryJoiningDataset::erase(QueryJoiningKey query_joining_key, Transaction& t)
{
    auto position = m_cache->find(query_joining_key);
    if (position == m_cache->end())
    {
        throw std::logic_error("Query not found");
    }
    transact_erase(*m_cache, position, t);
    m_db.erase<QueryJoiningData>(query_joining_key);
}

// multiple records
std::vector<QueryJoiningData> QueryJoiningDataset::query() const
{
    return{m_cache->begin(), m_cache->end()};
}

std::vector<QueryJoiningData> QueryJoiningDataset::query_input_equal(ReportTypeUUID input_uuid) const
{
    auto range = m_cache->find_range<InputIndex>(input_uuid);
    return{range.begin(), range.end()};
}

class SubjectQueryDataset::Cache: public stl_tools::storage<SubjectQueryView, SubjectQueryIndex, SubjectIndex, QueryJoiningKeyIndex>
{
};

SubjectQueryDataset::SubjectQueryDataset(Database& db):
    m_db(db),
    m_cache(std::make_unique<Cache>())
{
    ReadOnlyTransaction t(m_db);
    for (auto& subject_query_view : m_db.query<SubjectQueryView>())
    {
        m_cache->insert(subject_query_view);
    }
}


SubjectQueryDataset::~SubjectQueryDataset()
{
}

boost::optional<SubjectQueryData> SubjectQueryDataset::get(const SubjectQueryIdentity& identity) const
{
    if (m_cache->find(identity) == m_cache->end())
    {
        return boost::none;
    }

    SubjectQueryData subject_query_data;
    m_db.load<SubjectQueryData>(identity, subject_query_data);
    return subject_query_data;
}

bool SubjectQueryDataset::exist(const SubjectQueryIdentity& identity) const
{
    return m_cache->find(identity) != m_cache->end();
}

void SubjectQueryDataset::put(const SubjectQueryData& subject_query_data, Transaction& t)
{
    if (m_cache->find(subject_query_data.m_identity) != m_cache->end())
    {
        m_db.update(subject_query_data);
    }
    else
    {
        transact_insert(*m_cache, SubjectQueryView{subject_query_data.m_identity}, t);
        m_db.persist(subject_query_data);
    }
}

void SubjectQueryDataset::clear(const SubjectQueryIdentity& identity, Transaction& t) const
{
    auto position = m_cache->find(identity);
    if (position != m_cache->end())
    {
        transact_erase(*m_cache, position, t);
        m_db.erase<SubjectQueryData>(identity);
    }
}

void SubjectQueryDataset::erase_source_equal(const std::vector<SourceKey>& source_keys, Transaction& t)
{
    for (auto source_key : source_keys)
    {
        transact_erase_range(*m_cache, m_cache->find_range<SubjectIndex>(source_key), t);
    }
    m_db.erase_query<SubjectQueryData>(odb::query<SubjectQueryData>::identity.subject.value1.value.in_range(source_keys.begin(), source_keys.end()));
}

void SubjectQueryDataset::erase_subject_equal(const SubjectIdentity& subject_identity, Transaction& t)
{
    transact_erase_range(*m_cache, m_cache->find_range<SubjectIndex>(subject_identity), t);
    m_db.erase_query<SubjectQueryData>(
        odb::query<SubjectQueryData>::identity.subject.value1.value == subject_identity.m_value1 &&
        odb::query<SubjectQueryData>::identity.subject.value2.value == subject_identity.m_value2);
}

void SubjectQueryDataset::erase_query_joining_equal(QueryJoiningKey query_joining_key, Transaction& t)
{
    transact_erase_range(*m_cache, m_cache->find_range<QueryJoiningKeyIndex>(query_joining_key), t);
    m_db.erase_query<SubjectQueryData>(
        odb::query<SubjectQueryData>::identity.query_joining_key.value == query_joining_key);
}

class SubjectJoinedQueryDataset::Cache: public stl_tools::storage<SubjectJoinedQueryView, SubjectQueryIndex, SubjectIndex, QueryJoiningKeyIndex>
{
};

SubjectJoinedQueryDataset::SubjectJoinedQueryDataset(Database& db):
    m_db(db),
    m_cache(std::make_unique<Cache>())
{
    ReadOnlyTransaction t(m_db);
    for (auto& subject_joined_query_view : m_db.query<SubjectJoinedQueryView>())
    {
        m_cache->insert(subject_joined_query_view);
    }
}


SubjectJoinedQueryDataset::~SubjectJoinedQueryDataset()
{
}

boost::optional<SubjectJoinedQueryData> SubjectJoinedQueryDataset::get(const SubjectQueryIdentity& identity) const
{
    if (m_cache->find(identity) == m_cache->end())
    {
        return boost::none;
    }

    SubjectJoinedQueryData subject_joined_query_data;
    m_db.load<SubjectJoinedQueryData>(identity, subject_joined_query_data);
    return subject_joined_query_data;
}

bool SubjectJoinedQueryDataset::exist(const SubjectQueryIdentity& identity) const
{
    return m_cache->find(identity) != m_cache->end();
}

void SubjectJoinedQueryDataset::put(const SubjectJoinedQueryData& subject_joined_query_data, Transaction& t)
{
    if (m_cache->find(subject_joined_query_data.m_identity) != m_cache->end())
    {
        m_db.update(subject_joined_query_data);
    }
    else
    {
        transact_insert(*m_cache, SubjectJoinedQueryView{subject_joined_query_data.m_identity}, t);
        m_db.persist(subject_joined_query_data);
    }
}

void SubjectJoinedQueryDataset::clear(const SubjectQueryIdentity& identity, Transaction& t) const
{
    auto position = m_cache->find(identity);
    if (position != m_cache->end())
    {
        transact_erase(*m_cache, position, t);
        m_db.erase<SubjectJoinedQueryData>(identity);
    }
}

void SubjectJoinedQueryDataset::erase_source_equal(const std::vector<SourceKey>& source_keys, Transaction& t)
{
    for (auto source_key : source_keys)
    {
        transact_erase_range(*m_cache, m_cache->find_range<SubjectIndex>(source_key), t);
    }
    m_db.erase_query<SubjectJoinedQueryData>(odb::query<SubjectJoinedQueryData>::identity.subject.value1.value.in_range(source_keys.begin(), source_keys.end()));
}

void SubjectJoinedQueryDataset::erase_subject_equal(const SubjectIdentity& subject_identity, Transaction& t)
{
    transact_erase_range(*m_cache, m_cache->find_range<SubjectIndex>(subject_identity), t);
    m_db.erase_query<SubjectJoinedQueryData>(
        odb::query<SubjectJoinedQueryData>::identity.subject.value1.value == subject_identity.m_value1 &&
        odb::query<SubjectJoinedQueryData>::identity.subject.value2.value == subject_identity.m_value2);
}

void SubjectJoinedQueryDataset::erase_query_joining_equal(QueryJoiningKey query_joining_key, Transaction& t)
{
    transact_erase_range(*m_cache, m_cache->find_range<QueryJoiningKeyIndex>(query_joining_key), t);
    m_db.erase_query<SubjectJoinedQueryData>(
        odb::query<SubjectJoinedQueryData>::identity.query_joining_key.value == query_joining_key);
}

}} //namespace TR { namespace Core {