#pragma once
#include "BackgndFwd.h"
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "ServicesValues.hxx"
#include "ComplianceData-odb.hxx"
namespace TR { namespace Core {
struct ComplianceData;
struct SubjectComplianceData;

class ComplianceDataset
{
public:
    ComplianceDataset(Database& db);
    ~ComplianceDataset();

    // single record
    ComplianceData                      load(ComplianceKey compliance_key) const;
    boost::optional<ComplianceData>     find(ComplianceKey compliance_key) const;
    boost::optional<ComplianceData>     find_by_output(ReportTypeUUID output_uuid) const;
    boost::optional<ComplianceData>     find_by_input(ReportTypeUUID input_uuid) const;
    ComplianceKey                       persist(ComplianceData compliance_data, Transaction& t);
    void                                erase(ComplianceKey compliance_key, Transaction& t);

    // multiple records
    std::vector<ComplianceData>         query() const;

private:
    class ComplianceCache;

    Database& m_db;
    std::unique_ptr<ComplianceCache> m_cache;
};

class SubjectComplianceDataset
{
public:
    SubjectComplianceDataset(Database& db);
    ~SubjectComplianceDataset();

    // single record
    SubjectComplianceData               get(const SubjectIdentity& subject_identity, ComplianceKey compliance_key) const;
    void                                put(const SubjectComplianceData& subject_compliance_data, Transaction& t);

    // multiple records
    void                                erase_source_equal(const std::vector<SourceKey>& source_keys, Transaction& t);
    void                                erase_subject_equal(const SubjectIdentity& subject_identity, Transaction& t);
    void                                erase_compliance_equal(ComplianceKey compliance_key, Transaction& t);

private:
    class SubjectComplianceCache;

    Database& m_db;
    std::unique_ptr<SubjectComplianceCache> m_cache;
};

}} //namespace TR { namespace Core {