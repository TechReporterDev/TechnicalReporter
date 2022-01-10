#include "stdafx.h"
#include "ArchivingDataset.h"
#include "Transaction.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
namespace TR {  namespace Core {

namespace {
struct IdentityIndex: stl_tools::unique_storage_index<ArchivingSettingsIdentity>
{
    static key_type get_key(const ArchivingSettingsData& archiving_settings_data)
    {
        return archiving_settings_data.m_identity;
    }
};

struct SubjectIndex: stl_tools::single_storage_index<SubjectIdentity>
{
    static key_type get_key(const ArchivingSettingsData& archiving_settings_data)
    {
        return archiving_settings_data.m_identity.m_subject;
    }
};

struct ReportTypeIndex: stl_tools::single_storage_index<ReportTypeUUID>
{
    static key_type get_key(const ArchivingSettingsData& archiving_settings_data)
    {
        return archiving_settings_data.m_identity.m_report_type_uuid;
    }
};
} //namespace {

class ArchivingDataset::Cache: public stl_tools::storage<ArchivingSettingsData, IdentityIndex, SubjectIndex, ReportTypeIndex>
{
};

ArchivingDataset::ArchivingDataset(Database& db):
    m_db(db),
    m_cache(std::make_unique<Cache>())
{
    ReadOnlyTransaction t(m_db);
    for (auto& archiving_data : m_db.query<ArchivingSettingsData>())
    {
        m_cache->insert(archiving_data);
    }
}

ArchivingDataset::~ArchivingDataset()
{
}

ArchivingSettingsData ArchivingDataset::get(const SubjectIdentity& subject_identity, ReportTypeUUID report_type_uuid) const
{
    auto position = m_cache->find({subject_identity, report_type_uuid});
    if (position != m_cache->end())
    {
        return *position;
    }
    return ArchivingSettingsData({subject_identity, report_type_uuid});
}

void ArchivingDataset::put(const ArchivingSettingsData& archiving_data, Transaction& t)
{
    auto position = m_cache->find(archiving_data.m_identity);
    if (position != m_cache->end())
    {
        if (!archiving_data.is_default())
        {
            m_db.update(archiving_data);
            transact_update(*m_cache, archiving_data, t);
        }
        else
        {
            m_db.erase<ArchivingSettingsData>(archiving_data.m_identity);
            transact_erase(*m_cache, position, t);
        }
    }
    else
    {
        if (!archiving_data.is_default())
        {
            m_db.persist(archiving_data);
            transact_insert(*m_cache, archiving_data, t);
        }
    }
}

void ArchivingDataset::erase_source_equal(const std::vector<SourceKey>& source_keys, Transaction& t)
{
    for (auto source_key : source_keys)
    {
        transact_erase_range(*m_cache, m_cache->find_range<SubjectIndex>(source_key), t);
    }
    m_db.erase_query<ArchivingSettingsData>(odb::query<ArchivingSettingsData>::identity.subject.value1.value.in_range(source_keys.begin(), source_keys.end()));
}

void ArchivingDataset::erase_subject_equal(const SubjectIdentity& subject_identity, Transaction& t)
{
    transact_erase_range(*m_cache, m_cache->find_range<SubjectIndex>(subject_identity), t);
    m_db.erase_query<ArchivingSettingsData>(
        odb::query<ArchivingSettingsData>::identity.subject.value1.value == subject_identity.m_value1 &&
        odb::query<ArchivingSettingsData>::identity.subject.value2.value == subject_identity.m_value2);
}

void ArchivingDataset::erase_report_type_equal(ReportTypeUUID report_type_uuid, Transaction& t)
{
    transact_erase_range(*m_cache, m_cache->find_range<ReportTypeIndex>(report_type_uuid), t);
    m_db.erase_query<ArchivingSettingsData>(
        odb::query<ArchivingSettingsData>::identity.report_type_uuid.value == report_type_uuid);
}

}} //namespace TR { namespace Core {