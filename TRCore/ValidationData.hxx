#pragma once
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "ServicesValues.hxx"

namespace TR {namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

enum class ValidationPolicy;
enum class ValidationResult;

struct ValidationData
{
	ValidationData():
		m_key(0)
	{
	}

	ValidationData(Key key, std::string	name, ReportTypeUUID input_uuid, ReportTypeUUID output_uuid):
		m_key(key),
		m_name(std::move(name)),
		m_input_uuid(input_uuid),
		m_output_uuid(output_uuid)
	{
	}

	Key					m_key;
	std::string			m_name;
	ReportTypeUUID		m_input_uuid;
	ReportTypeUUID		m_output_uuid;
};

struct SubjectValidationIdentity
{
	SubjectValidationIdentity()
	{
	}

	SubjectValidationIdentity(SubjectIdentity subject, ValidationKey validation_key):
		m_subject(subject),
		m_validation_key(validation_key)
	{
	}

	SubjectIdentity		m_subject;
	ValidationKey		m_validation_key;
};

inline bool operator < (const SubjectValidationIdentity& left, const SubjectValidationIdentity& right)
{
	if (left.m_subject < right.m_subject)
	{
		return true;
	}

	if (left.m_subject > right.m_subject)
	{
		return false;
	}

	return left.m_validation_key < right.m_validation_key;
}


struct SubjectValidationData
{
	SubjectValidationData():
		m_validation_policy(ValidationPolicy(0)),
		m_validation_result(ValidationResult(0)),
		m_input_uuid(stl_tools::null_uuid())
	{
	}

	SubjectValidationData(SubjectValidationIdentity identity,
			ValidationPolicy validation_policy = ValidationPolicy(0),
			ValidationResult validation_result = ValidationResult(0),
			UUID input_uuid = stl_tools::null_uuid()) :
		m_identity(identity),
		m_validation_policy(validation_policy),
		m_validation_result(validation_result),
		m_input_uuid(input_uuid)
	{
	}

	bool is_default() const
	{
		return m_validation_policy == ValidationPolicy(0) && m_validation_result == ValidationResult(0) && m_input_uuid == stl_tools::null_uuid();
	}

	SubjectValidationIdentity		m_identity;
	ValidationPolicy				m_validation_policy;
	ValidationResult				m_validation_result;
	UUID							m_input_uuid;
};

#pragma db object(ValidationData)
#pragma db member(ValidationData::m_key) id

#pragma db value(SubjectValidationIdentity)
#pragma db object(SubjectValidationData)
#pragma db member(SubjectValidationData::m_identity) id
#pragma warning(pop)

}
} //namespace CI { namespace Core {