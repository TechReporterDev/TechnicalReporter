#pragma once
#include "BackgndFwd.h"
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "ServicesValues.hxx"
#include "IntegrityCheckingData-odb.hxx"
namespace TR { namespace Core {

class IntegrityCheckingDataset
{
public:
    IntegrityCheckingDataset(Database& db);
    ~IntegrityCheckingDataset();

    // single record
    IntegrityCheckingData               get(const IntegrityCheckingIdentity& identity) const;
    void                                put(const IntegrityCheckingData& integrity_checking_data, Transaction& t);

    // multiple records
    void                                erase_source_equal(const std::vector<SourceKey>& source_keys, Transaction& t);
    void                                erase_subject_equal(const SubjectIdentity& subject_identity, Transaction& t);
    void                                erase_report_type_equal(ReportTypeUUID report_type_uuid, Transaction& t);

private:
    class Cache;

    Database& m_db;
    std::unique_ptr<Cache> m_cache;
};

}} //namespace TR { namespace Core {