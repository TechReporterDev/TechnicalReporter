#pragma once
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "ServicesValues.hxx"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

enum class IntegrityCheckingPolicy;
enum class IntegrityCheckingResult;
enum class IntegrityCheckingPatternChoise;

struct IntegrityCheckingIdentity
{
	IntegrityCheckingIdentity()
	{
	}

	IntegrityCheckingIdentity(SubjectIdentity subject, ReportTypeUUID report_type_uuid):
		m_subject(subject),
		m_report_type_uuid(report_type_uuid)
	{
	}

	SubjectIdentity			m_subject;
	ReportTypeUUID			m_report_type_uuid;
};

inline bool operator < (const IntegrityCheckingIdentity& left, const IntegrityCheckingIdentity& right)
{
	if (left.m_subject < right.m_subject)
	{
		return true;
	}

	if (left.m_subject > right.m_subject)
	{
		return false;
	}

	return left.m_report_type_uuid < right.m_report_type_uuid;
}

struct IntegrityCheckingInputData
{
	IntegrityCheckingInputData() :
		m_pattern_uuid(stl_tools::null_uuid()),
		m_target_report_uuid(stl_tools::null_uuid())
	{
	}

	IntegrityCheckingInputData(UUID pattern_uuid, UUID target_report_uuid) :
		m_pattern_uuid(pattern_uuid),
		m_target_report_uuid(target_report_uuid)
	{
	}

	UUID m_pattern_uuid;
	UUID m_target_report_uuid;
};

struct IntegrityCheckingData
{
	IntegrityCheckingData():
		m_checking_policy(IntegrityCheckingPolicy(0)),
		m_pattern_choise(IntegrityCheckingPatternChoise(0)),
		m_custom_pattern_uuid(stl_tools::null_uuid()),
		m_result(IntegrityCheckingResult(0))
	{
	}

	IntegrityCheckingData(IntegrityCheckingIdentity identity, 
			IntegrityCheckingPolicy checking_policy = IntegrityCheckingPolicy(0), 
			IntegrityCheckingPatternChoise pattern_choise = IntegrityCheckingPatternChoise(0),
			UUID custom_pattern_uuid = stl_tools::null_uuid(),
			IntegrityCheckingResult result = IntegrityCheckingResult(0),
			IntegrityCheckingInputData input = IntegrityCheckingInputData()):
		m_identity(identity),
		m_checking_policy(checking_policy),
		m_pattern_choise(pattern_choise),
		m_custom_pattern_uuid(custom_pattern_uuid),
		m_result(result),
		m_input(input)
	{
	}

	bool is_default() const
	{
		return m_checking_policy == IntegrityCheckingPolicy(0) &&
			m_pattern_choise == IntegrityCheckingPatternChoise(0) &&
			m_custom_pattern_uuid == stl_tools::null_uuid() &&
			m_result == IntegrityCheckingResult(0) &&
			m_input.m_pattern_uuid == stl_tools::null_uuid() && 
			m_input.m_target_report_uuid == stl_tools::null_uuid();
	}

	IntegrityCheckingIdentity		m_identity;
	IntegrityCheckingPolicy			m_checking_policy;
	IntegrityCheckingPatternChoise	m_pattern_choise;
	UUID							m_custom_pattern_uuid;
	IntegrityCheckingResult			m_result;
	IntegrityCheckingInputData		m_input;
};

struct IntegrityCheckingPatternData
{
	IntegrityCheckingPatternData()
	{
	}

	IntegrityCheckingPatternData(IntegrityCheckingIdentity identity, Blob pattern):
		m_identity(identity),
		m_pattern(std::move(pattern))
	{
	}

	IntegrityCheckingIdentity		m_identity;
	Blob							m_pattern;	
};

#pragma db value(Blob) type("LONGBLOB")
#pragma db value(IntegrityCheckingIdentity)
#pragma db value(IntegrityCheckingInputData)

#pragma db object(IntegrityCheckingData)
#pragma db member(IntegrityCheckingData::m_identity) id

#pragma db object(IntegrityCheckingPatternData)
#pragma db member(IntegrityCheckingPatternData::m_identity) id

#pragma warning(pop)

}} //namespace CI { namespace Core {