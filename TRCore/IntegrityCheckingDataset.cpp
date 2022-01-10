#include "stdafx.h"
#include "IntegrityCheckingDataset.h"
#include "Transaction.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
namespace TR {  namespace Core {

namespace {
struct IdentityIndex: stl_tools::unique_storage_index<IntegrityCheckingIdentity>
{
    static key_type get_key(const IntegrityCheckingData& integrity_checking_data)
    {
        return integrity_checking_data.m_identity;
    }
};

struct SubjectIndex: stl_tools::single_storage_index<SubjectIdentity>
{
    static key_type get_key(const IntegrityCheckingData& integrity_checking_data)
    {
        return integrity_checking_data.m_identity.m_subject;
    }
};

struct ReportTypeIndex: stl_tools::single_storage_index<ReportTypeUUID>
{
    static key_type get_key(const IntegrityCheckingData& integrity_checking_data)
    {
        return integrity_checking_data.m_identity.m_report_type_uuid;
    }
};
} //namespace {

class IntegrityCheckingDataset::Cache: public stl_tools::storage<IntegrityCheckingData, IdentityIndex, SubjectIndex, ReportTypeIndex>
{
};

IntegrityCheckingDataset::IntegrityCheckingDataset(Database& db):
    m_db(db),
    m_cache(std::make_unique<Cache>())
{
    ReadOnlyTransaction t(m_db);
    for (auto& integrity_checking_data : m_db.query<IntegrityCheckingData>())
    {
        m_cache->insert(integrity_checking_data);
    }
}

IntegrityCheckingDataset::~IntegrityCheckingDataset()
{
}

IntegrityCheckingData IntegrityCheckingDataset::get(const IntegrityCheckingIdentity& identity) const
{
    auto position = m_cache->find(identity);
    if (position != m_cache->end())
    {
        return *position;
    }
    return IntegrityCheckingData(identity);
}

void IntegrityCheckingDataset::put(const IntegrityCheckingData& integrity_checking_data, Transaction& t)
{
    auto position = m_cache->find(integrity_checking_data.m_identity);
    if (position != m_cache->end())
    {
        if (!integrity_checking_data.is_default())
        {
            m_db.update(integrity_checking_data);
            transact_update(*m_cache, integrity_checking_data, t);  
        }
        else
        {
            m_db.erase<IntegrityCheckingData>(integrity_checking_data.m_identity);
            transact_erase(*m_cache, position, t);
        }
    }
    else
    {
        if (!integrity_checking_data.is_default())
        {
            m_db.persist(integrity_checking_data);
            transact_insert(*m_cache, integrity_checking_data, t);          
        }
    }
}

void IntegrityCheckingDataset::erase_source_equal(const std::vector<SourceKey>& source_keys, Transaction& t)
{
    for (auto source_key : source_keys)
    {
        transact_erase_range(*m_cache, m_cache->find_range<SubjectIndex>(source_key), t);
    }
    m_db.erase_query<IntegrityCheckingData>(odb::query<IntegrityCheckingData>::identity.subject.value1.value.in_range(source_keys.begin(), source_keys.end()));
}

void IntegrityCheckingDataset::erase_subject_equal(const SubjectIdentity& subject_identity, Transaction& t)
{
    transact_erase_range(*m_cache, m_cache->find_range<SubjectIndex>(subject_identity), t);
    m_db.erase_query<IntegrityCheckingData>(
        odb::query<IntegrityCheckingData>::identity.subject.value1.value == subject_identity.m_value1 &&
        odb::query<IntegrityCheckingData>::identity.subject.value2.value == subject_identity.m_value2);
}

void IntegrityCheckingDataset::erase_report_type_equal(ReportTypeUUID report_type_uuid, Transaction& t)
{
    transact_erase_range(*m_cache, m_cache->find_range<ReportTypeIndex>(report_type_uuid), t);
    m_db.erase_query<IntegrityCheckingData>(
        odb::query<IntegrityCheckingData>::identity.report_type_uuid.value == report_type_uuid);
}

}} //namespace TR { namespace Core {