#pragma once
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "ServicesValues.hxx"

namespace TR {namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

enum class ComplianceCheckingPolicy;
enum class ComplianceCheckingResult;

struct ComplianceData
{
	ComplianceData():
		m_key(0)
	{
	}

	ComplianceData(Key key, std::string	name, ReportTypeUUID input_uuid, ReportTypeUUID output_uuid):
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

struct SubjectComplianceIdentity
{
	SubjectComplianceIdentity()
	{
	}

	SubjectComplianceIdentity(SubjectIdentity subject, ComplianceKey compliance_key):
		m_subject(subject),
		m_compliance_key(compliance_key)
	{
	}

	SubjectIdentity		m_subject;
	ComplianceKey		m_compliance_key;	
};

inline bool operator < (const SubjectComplianceIdentity& left, const SubjectComplianceIdentity& right)
{
	if (left.m_subject < right.m_subject)
	{
		return true;
	}

	if (left.m_subject > right.m_subject)
	{
		return false;
	}

	return left.m_compliance_key < right.m_compliance_key;
}

struct SubjectComplianceData
{
	SubjectComplianceData():
		m_checking_policy(ComplianceCheckingPolicy(0)),
		m_checking_result(ComplianceCheckingResult(0)),
		m_input_uuid(stl_tools::null_uuid())
	{
	}

	SubjectComplianceData(SubjectComplianceIdentity identity, 
		ComplianceCheckingPolicy checking_policy = ComplianceCheckingPolicy(0), 
		ComplianceCheckingResult checking_result = ComplianceCheckingResult(0),
		UUID input_uuid = stl_tools::null_uuid()):
		m_identity(identity),
		m_checking_policy(checking_policy),
		m_checking_result(checking_result),
		m_input_uuid(input_uuid)
	{
	}

	bool is_default() const
	{
		return m_checking_policy == ComplianceCheckingPolicy(0) &&
			m_checking_result == ComplianceCheckingResult(0) &&
			m_input_uuid == stl_tools::null_uuid();
	}

	SubjectComplianceIdentity		m_identity;
	ComplianceCheckingPolicy		m_checking_policy;
	ComplianceCheckingResult		m_checking_result;
	UUID							m_input_uuid;
};

#pragma db object(ComplianceData)
#pragma db member(ComplianceData::m_key) id

#pragma db value(SubjectComplianceIdentity)
#pragma db object(SubjectComplianceData)
#pragma db member(SubjectComplianceData::m_identity) id
#pragma warning(pop)

}} //namespace CI { namespace Core {