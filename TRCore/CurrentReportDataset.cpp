#include "stdafx.h"
#include "CurrentReportDataset.h"
#include "Transaction.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
namespace TR {  namespace Core {

namespace {

struct IdentityIndex: stl_tools::unique_storage_index<CurrentReportIdentity>
{
    static key_type get_key(const CurrentReportData& current_report_data)
    {
        return current_report_data.m_identity;
    }
};

struct SourceIndex: stl_tools::single_storage_index<SourceKey>
{
    static key_type get_key(const CurrentReportData& current_report_data)
    {
        return current_report_data.m_identity.m_source_key;
    }
};

struct ReportTypeIndex: stl_tools::single_storage_index<ReportTypeUUID>
{
    static key_type get_key(const CurrentReportData& current_report_data)
    {
        return current_report_data.m_identity.m_report_type_uuid;
    }
};

using UUIDIndex = stl_tools::memdata_index_traits<UUID, CurrentReportData, &CurrentReportData::m_uuid, stl_tools::storage_index_cathegory::unique_index>;

} //namespace {

class CurrentReportDataset::CurrentReportCache: public stl_tools::storage<CurrentReportData, IdentityIndex, SourceIndex, ReportTypeIndex, UUIDIndex>
{
};

CurrentReportDataset::CurrentReportDataset(Database& db):
    m_db(db),
    m_cache(std::make_unique<CurrentReportCache>())
{
    ReadOnlyTransaction t(m_db);
    for (auto& current_report_data : m_db.query<CurrentReportData>())
    {
        m_cache->insert(current_report_data);
    }
}

CurrentReportDataset::~CurrentReportDataset()
{
}

CurrentReportData CurrentReportDataset::load(SourceKey source_key, ReportTypeUUID report_type_uuid) const
{
    auto position = m_cache->find({source_key, report_type_uuid});
    if (position == m_cache->end())
    {
        throw std::logic_error("Current report not found");
    }
    return *position;
}

boost::optional<CurrentReportData> CurrentReportDataset::find(SourceKey source_key, ReportTypeUUID report_type_uuid) const
{
    auto position = m_cache->find({source_key, report_type_uuid});
    if (position == m_cache->end())
    {
        return boost::none;
    }
    return *position;
}

boost::optional<CurrentReportData> CurrentReportDataset::find(UUID uuid) const
{
    auto position = m_cache->find<UUIDIndex>(uuid);
    if (position == m_cache->end())
    {
        return boost::none;
    }
    return *position;
}

void CurrentReportDataset::persist(const CurrentReportData& current_report_data, Transaction& t)
{
    m_db.persist(current_report_data);
    transact_insert(*m_cache, current_report_data, t);
}

void CurrentReportDataset::update(const CurrentReportData& current_report_data, Transaction& t)
{
    m_db.update(current_report_data);
    transact_update(*m_cache, current_report_data, t);
}

std::vector<CurrentReportData> CurrentReportDataset::query_source_equal(SourceKey source_key) const
{
    auto current_reports = m_cache->find_range<SourceIndex>(source_key);
    return{current_reports.begin(), current_reports.end()};
}

void CurrentReportDataset::erase_source_equal(const std::vector<SourceKey>& source_keys, Transaction& t)
{
    for (auto source_key : source_keys)
    {
        transact_erase_range(*m_cache, m_cache->find_range<SourceIndex>(source_key), t);
    }
    m_db.erase_query<CurrentReportData>(odb::query<CurrentReportData>::identity.source_key.value.in_range(source_keys.begin(), source_keys.end()));
}

void CurrentReportDataset::erase_report_type_equal(ReportTypeUUID report_type_uuid, Transaction& t)
{
    transact_erase_range(*m_cache, m_cache->find_range<ReportTypeIndex>(report_type_uuid), t);
    m_db.erase_query<CurrentReportData>(odb::query<CurrentReportData>::identity.report_type_uuid.value == report_type_uuid);
}

}} //namespace TR { namespace Core {