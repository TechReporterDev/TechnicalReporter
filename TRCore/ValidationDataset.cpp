#include "stdafx.h"
#include "ValidationDataset.h"
#include "Transaction.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
namespace TR {  namespace Core {

namespace {
struct ValidationIndex: stl_tools::unique_storage_index<ValidationKey>
{
    static key_type get_key(const ValidationData& validation_data)
    {
        return ValidationKey(validation_data.m_key);
    }

    static key_type get_key(const SubjectValidationData& subject_validation_data)
    {
        return subject_validation_data.m_identity.m_validation_key;
    }
};

struct ValidationInputIndex: stl_tools::single_storage_index<ReportTypeUUID>
{
    static key_type get_key(const ValidationData& validation_data)
    {
        return validation_data.m_input_uuid;
    }
};

struct ValidationOutputIndex: stl_tools::unique_storage_index<ReportTypeUUID>
{
    static key_type get_key(const ValidationData& validation_data)
    {
        return validation_data.m_output_uuid;
    }
};

struct SubjectValidationIndex: stl_tools::unique_storage_index<SubjectValidationIdentity>
{
    static key_type get_key(const SubjectValidationData& subject_validation_data)
    {
        return subject_validation_data.m_identity;
    }
};

struct SubjectIndex: stl_tools::single_storage_index<SubjectIdentity>
{
    static key_type get_key(const SubjectValidationData& subject_validation_data)
    {
        return subject_validation_data.m_identity.m_subject;
    }
};

struct ValidationKeyIndex: stl_tools::single_storage_index<ValidationKey>
{
    static key_type get_key(const SubjectValidationData& subject_validation_data)
    {
        return subject_validation_data.m_identity.m_validation_key;
    }
};

} //namespace {

class ValidationDataset::Cache: public stl_tools::storage<ValidationData, ValidationIndex, ValidationInputIndex, ValidationOutputIndex>
{
};

ValidationDataset::ValidationDataset(Database& db):
    m_db(db),
    m_cache(std::make_unique<Cache>())
{
    ReadOnlyTransaction t(m_db);
    for (auto& validation_data : m_db.query<ValidationData>())
    {
        m_cache->insert(validation_data);
    }
}

ValidationDataset::~ValidationDataset()
{
}

ValidationData ValidationDataset::load(ValidationKey validation_key) const
{
    auto position = m_cache->find(validation_key);
    if (position == m_cache->end())
    {
        throw std::logic_error("Validation not found");
    }
    return *position;
}

boost::optional<ValidationData> ValidationDataset::find(ValidationKey validation_key) const
{
    auto position = m_cache->find(validation_key);
    if (position == m_cache->end())
    {
        return boost::none;
    }
    return *position;
}

std::vector<ValidationData> ValidationDataset::find_by_input(ReportTypeUUID input_uuid) const
{
    std::vector<ValidationData> validations;
    for(const auto& validation_data : m_cache->find_range<ValidationInputIndex>(input_uuid))
    {
        validations.push_back(validation_data);
    }
    return validations;
}

boost::optional<ValidationData> ValidationDataset::find_by_output(ReportTypeUUID output_uuid) const
{
    auto position = m_cache->find<ValidationOutputIndex>(output_uuid);
    if (position == m_cache->end())
    {
        return boost::none;
    }
    return *position;
}

ValidationKey ValidationDataset::persist(ValidationData validation_data, Transaction& t)
{
    validation_data.m_key = m_db.persist(validation_data);
    transact_insert(*m_cache, validation_data, t);
    return ValidationKey(validation_data.m_key);
}

void ValidationDataset::erase(ValidationKey validation_key, Transaction& t)
{
    auto position = m_cache->find(validation_key);
    if (position == m_cache->end())
    {
        throw std::logic_error("Validation not found");
    }
    transact_erase(*m_cache, position, t);
    m_db.erase<ValidationData>(validation_key);
}

std::vector<ValidationData> ValidationDataset::query() const
{
    return{m_cache->begin(), m_cache->end()};
}

class SubjectValidationDataset::SubjectValidationCache: public stl_tools::storage<SubjectValidationData, SubjectValidationIndex, SubjectIndex, ValidationKeyIndex>
{
};

SubjectValidationDataset::SubjectValidationDataset(Database& db):
    m_db(db),
    m_cache(std::make_unique<SubjectValidationCache>())
{
    ReadOnlyTransaction t(m_db);
    for (auto& subject_validation_data : m_db.query<SubjectValidationData>())
    {
        m_cache->insert(subject_validation_data);
    }
}

SubjectValidationDataset::~SubjectValidationDataset()
{
}

SubjectValidationData SubjectValidationDataset::get(const SubjectIdentity& subject_identity, ValidationKey validation_key) const
{
    auto position = m_cache->find({subject_identity, validation_key});
    if (position != m_cache->end())
    {
        return *position;
    }
    return SubjectValidationData({subject_identity, validation_key});
}

void SubjectValidationDataset::put(const SubjectValidationData& subject_validation_data, Transaction& t)
{
    auto position = m_cache->find(subject_validation_data.m_identity);
    if (position != m_cache->end())
    {
        if (!subject_validation_data.is_default())
        {
            m_db.update(subject_validation_data);
            transact_update(*m_cache, subject_validation_data, t);  
        }
        else
        {
            m_db.erase<SubjectValidationData>(subject_validation_data.m_identity);
            transact_erase(*m_cache, position, t);
        }
    }
    else
    {
        if (!subject_validation_data.is_default())
        {
            m_db.persist(subject_validation_data);
            transact_insert(*m_cache, subject_validation_data, t);          
        }
    }
}

void SubjectValidationDataset::erase_source_equal(const std::vector<SourceKey>& source_keys, Transaction& t)
{
    for (auto source_key : source_keys)
    {
        transact_erase_range(*m_cache, m_cache->find_range<SubjectIndex>(source_key), t);
    }
    m_db.erase_query<SubjectValidationData>(odb::query<SubjectValidationData>::identity.subject.value1.value.in_range(source_keys.begin(), source_keys.end()));
}

void SubjectValidationDataset::erase_subject_equal(const SubjectIdentity& subject_identity, Transaction& t)
{
    transact_erase_range(*m_cache, m_cache->find_range<SubjectIndex>(subject_identity), t);
    m_db.erase_query<SubjectValidationData>(
        odb::query<SubjectValidationData>::identity.subject.value1.value == subject_identity.m_value1 &&
        odb::query<SubjectValidationData>::identity.subject.value2.value == subject_identity.m_value2);

}

void SubjectValidationDataset::erase_validation_equal(ValidationKey validation_key, Transaction& t)
{
    transact_erase_range(*m_cache, m_cache->find_range<ValidationKeyIndex>(validation_key), t);
    m_db.erase_query<SubjectValidationData>(
        odb::query<SubjectValidationData>::identity.validation_key.value == validation_key);
}

}} //namespace TR { namespace Core {