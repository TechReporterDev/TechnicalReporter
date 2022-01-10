#pragma once
#include "BackgndFwd.h"
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "CurrentReportData-odb.hxx"

namespace TR { namespace Core {
struct CurrentReportData;

class CurrentReportDataset
{
public:
    CurrentReportDataset(Database& db);
    ~CurrentReportDataset();

    // single record
    CurrentReportData                   load(SourceKey source_key, ReportTypeUUID report_type_uuid) const;
    boost::optional<CurrentReportData>  find(SourceKey source_key, ReportTypeUUID report_type_uuid) const;
    boost::optional<CurrentReportData>  find(UUID uuid) const;
    void                                persist(const CurrentReportData& current_report_data, Transaction& t);
    void                                update(const CurrentReportData& current_report_data, Transaction& t);

    // multiple records
    std::vector<CurrentReportData>      query_source_equal(SourceKey source_key) const;
    void                                erase_source_equal(const std::vector<SourceKey>& source_keys, Transaction& t);
    void                                erase_report_type_equal(ReportTypeUUID report_type_uuid, Transaction& t);

private:
    class CurrentReportCache;

    Database& m_db;
    std::unique_ptr<CurrentReportCache> m_cache;
};

}} //namespace T