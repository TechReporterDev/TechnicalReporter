#include "stdafx.h"
#include "ComplianceDataset.h"
#include "Transaction.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
namespace TR {  namespace Core {

namespace {
struct ComplianceIndex: stl_tools::unique_storage_index<ComplianceKey>
{
    static key_type get_key(const ComplianceData& compliance_data)
    {
        return ComplianceKey(compliance_data.m_key);
    }
};

struct ComplianceOutputIndex: stl_tools::unique_storage_index<ReportTypeUUID>
{
    static key_type get_key(const ComplianceData& compliance_data)
    {
        return compliance_data.m_output_uuid;
    }
};

struct ComplianceInputIndex: stl_tools::unique_storage_index<ReportTypeUUID>
{
    static key_type get_key(const ComplianceData& compliance_data)
    {
        return compliance_data.m_input_uuid;
    }
};

struct SubjectComplianceIndex: stl_tools::unique_storage_index<SubjectComplianceIdentity>
{
    static key_type get_key(const SubjectComplianceData& subject_compliance_data)
    {
        return subject_compliance_data.m_identity;
    }
};

struct SubjectIndex: stl_tools::single_storage_index<SubjectIdentity>
{
    static key_type get_key(const SubjectComplianceData& compliance_compliance_data)
    {
        return compliance_compliance_data.m_identity.m_subject;
    }
};

struct ComplianceKeyIndex: stl_tools::single_storage_index<ComplianceKey>
{
    static key_type get_key(const SubjectComplianceData& compliance_data)
    {
        return compliance_data.m_identity.m_compliance_key;
    }
};

} //namespace {

class ComplianceDataset::ComplianceCache: public stl_tools::storage<ComplianceData, ComplianceIndex, ComplianceOutputIndex, ComplianceInputIndex>
{
};

ComplianceDataset::ComplianceDataset(Database& db):
    m_db(db),
    m_cache(std::make_unique<ComplianceCache>())
{
    ReadOnlyTransaction t(m_db);
    for (auto& compliance_data : m_db.query<ComplianceData>())
    {
        m_cache->insert(compliance_data);
    }
}

ComplianceDataset::~ComplianceDataset()
{
}

ComplianceData ComplianceDataset::load(ComplianceKey compliance_key) const
{
    auto position = m_cache->find(compliance_key);
    if (position == m_cache->end())
    {
        throw std::logic_error("Compliance not found");
    }
    return *position;
}

boost::optional<ComplianceData> ComplianceDataset::find(ComplianceKey compliance_key) const
{
    auto position = m_cache->find(compliance_key);
    if (position == m_cache->end())
    {
        return boost::none;
    }
    return *position;
}

boost::optional<ComplianceData> ComplianceDataset::find_by_output(ReportTypeUUID output_uuid) const
{
    auto position = m_cache->find<ComplianceOutputIndex>(output_uuid);
    if (position == m_cache->end())
    {
        return boost::none;
    }
    return *position;
}

boost::optional<ComplianceData> ComplianceDataset::find_by_input(ReportTypeUUID input_uuid) const
{
    auto position = m_cache->find<ComplianceInputIndex>(input_uuid);
    if (position == m_cache->end())
    {
        return boost::none;
    }
    return *position;
}

ComplianceKey ComplianceDataset::persist(ComplianceData compliance_data, Transaction& t)
{
    compliance_data.m_key = m_db.persist(compliance_data);
    transact_insert(*m_cache, compliance_data, t);
    return ComplianceKey(compliance_data.m_key);
}

void ComplianceDataset::erase(ComplianceKey compliance_key, Transaction& t)
{
    auto position = m_cache->find(compliance_key);
    if (position == m_cache->end())
    {
        throw std::logic_error("Compliance not found");
    }
    transact_erase(*m_cache, position, t);
    m_db.erase<ComplianceData>(compliance_key);
}

std::vector<ComplianceData> ComplianceDataset::query() const
{
    return {m_cache->begin(), m_cache->end()};
}

class SubjectComplianceDataset::SubjectComplianceCache: public stl_tools::storage<SubjectComplianceData, SubjectComplianceIndex, SubjectIndex, ComplianceKeyIndex>
{
};

SubjectComplianceDataset::SubjectComplianceDataset(Database& db):
    m_db(db),
    m_cache(std::make_unique<SubjectComplianceCache>())
{
    ReadOnlyTransaction t(m_db);
    for (auto& subject_compliance_data : m_db.query<SubjectComplianceData>())
    {
        m_cache->insert(subject_compliance_data);
    }
}

SubjectComplianceDataset::~SubjectComplianceDataset()
{
}

SubjectComplianceData SubjectComplianceDataset::get(const SubjectIdentity& subject_identity, ComplianceKey compliance_key) const
{
    auto position = m_cache->find({subject_identity, compliance_key});
    if (position != m_cache->end())
    {
        return *position;
    }
    return SubjectComplianceData({subject_identity, compliance_key});
}

void SubjectComplianceDataset::put(const SubjectComplianceData& subject_compliance_data, Transaction& t)
{
    auto position = m_cache->find(subject_compliance_data.m_identity);
    if (position != m_cache->end())
    {
        if (!subject_compliance_data.is_default())
        {
            m_db.update(subject_compliance_data);
            transact_update(*m_cache, subject_compliance_data, t);  
        }
        else
        {
            m_db.erase<SubjectComplianceData>(subject_compliance_data.m_identity);
            transact_erase(*m_cache, position, t);
        }
    }
    else
    {
        if (!subject_compliance_data.is_default())
        {
            m_db.persist(subject_compliance_data);
            transact_insert(*m_cache, subject_compliance_data, t);          
        }
    }
}

void SubjectComplianceDataset::erase_source_equal(const std::vector<SourceKey>& source_keys, Transaction& t)
{
    for (auto source_key : source_keys)
    {
        transact_erase_range(*m_cache, m_cache->find_range<SubjectIndex>(source_key), t);
    }
    m_db.erase_query<SubjectComplianceData>(odb::query<SubjectComplianceData>::identity.subject.value1.value.in_range(source_keys.begin(), source_keys.end()));
}

void SubjectComplianceDataset::erase_subject_equal(const SubjectIdentity& subject_identity, Transaction& t)
{
    transact_erase_range(*m_cache, m_cache->find_range<SubjectIndex>(subject_identity), t);
    m_db.erase_query<SubjectComplianceData>(
        odb::query<SubjectComplianceData>::identity.subject.value1.value == subject_identity.m_value1 &&
        odb::query<SubjectComplianceData>::identity.subject.value2.value == subject_identity.m_value2);

}

void SubjectComplianceDataset::erase_compliance_equal(ComplianceKey compliance_key, Transaction& t)
{
    transact_erase_range(*m_cache, m_cache->find_range<ComplianceKeyIndex>(compliance_key), t);
    m_db.erase_query<SubjectComplianceData>(
        odb::query<SubjectComplianceData>::identity.compliance_key.value == compliance_key);
}

}} //namespace TR { namespace Core {