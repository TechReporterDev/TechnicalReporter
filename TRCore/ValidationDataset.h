#pragma once
#include "BackgndFwd.h"
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "ServicesValues.hxx"
#include "ValidationData-odb.hxx"
namespace TR { namespace Core {

class ValidationDataset
{
public:
    ValidationDataset(Database& db);
    ~ValidationDataset();

    // single record
    ValidationData                      load(ValidationKey validation_key) const;
    boost::optional<ValidationData>     find(ValidationKey validation_key) const;
    std::vector<ValidationData>         find_by_input(ReportTypeUUID input_uuid) const;
    boost::optional<ValidationData>     find_by_output(ReportTypeUUID output_uuid) const;
    ValidationKey                       persist(ValidationData validation_data, Transaction& t);
    void                                erase(ValidationKey validation_key, Transaction& t);

    // multipla records
    std::vector<ValidationData>         query() const;

private:
    class Cache;

    Database& m_db;
    std::unique_ptr<Cache> m_cache;
};

class SubjectValidationDataset
{
public:
    SubjectValidationDataset(Database& db);
    ~SubjectValidationDataset();

    // single record
    SubjectValidationData               get(const SubjectIdentity& subject_identity, ValidationKey validation_key) const;
    void                                put(const SubjectValidationData& subject_validation_data, Transaction& t);

    // multiple records
    void                                erase_source_equal(const std::vector<SourceKey>& source_keys, Transaction& t);
    void                                erase_subject_equal(const SubjectIdentity& subject_identity, Transaction& t);
    void                                erase_validation_equal(ValidationKey validation_key, Transaction& t);

private:
    class SubjectValidationCache;

    Database& m_db;
    std::unique_ptr<SubjectValidationCache> m_cache;
};

}} //namespace TR { namespace Core {