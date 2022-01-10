#pragma once
#include <boost\optional.hpp>
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "ServicesValues.hxx"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

struct QueryJoiningData
{
	QueryJoiningData():
		m_key(0)
	{
	}

	QueryJoiningData(QueryJoiningKey key, ReportTypeUUID input_uuid, ReportTypeUUID output_uuid, boost::optional<XmlQueryDocText> default_query_xml):
		m_key(key),
		m_input_uuid(std::move(input_uuid)),
		m_output_uuid(std::move(output_uuid)),
		m_default_query_xml(std::move(default_query_xml))
	{
	}

	Key									m_key;
	ReportTypeUUID						m_input_uuid;
	ReportTypeUUID						m_output_uuid;
	boost::optional<XmlQueryDocText>	m_default_query_xml;
};

struct SubjectQueryIdentity
{
	SubjectQueryIdentity()
	{
	}

	SubjectQueryIdentity(SubjectIdentity subject, QueryJoiningKey query_joining_key):
		m_subject(subject),
		m_query_joining_key(query_joining_key)
	{
	}

	SubjectIdentity		m_subject;
	QueryJoiningKey		m_query_joining_key;
};

inline bool operator < (const SubjectQueryIdentity& left, const SubjectQueryIdentity& right)
{
	if (left.m_subject < right.m_subject)
	{
		return true;
	}

	if (right.m_subject < left.m_subject)
	{
		return false;
	}

	return left.m_query_joining_key < right.m_query_joining_key;
}

struct SubjectQueryData
{
	SubjectQueryData():
		m_uuid(stl_tools::null_uuid())
	{
	}

	SubjectQueryData(SubjectQueryIdentity identity, XmlQueryDocText query_xml, UUID uuid):
		m_identity(identity),
		m_query_xml(std::move(query_xml)),
		m_uuid(uuid)
	{
	}

	SubjectQueryIdentity			m_identity;
	XmlQueryDocText					m_query_xml;
	UUID							m_uuid;
};

#pragma db view object(SubjectQueryData)
struct SubjectQueryView
{
	SubjectQueryIdentity			m_identity;
};

struct SubjectJoinedQueryData
{
	SubjectJoinedQueryData():
		m_uuid(stl_tools::null_uuid()),
		m_base_query_uuid(stl_tools::null_uuid()),
		m_subject_query_uuid(stl_tools::null_uuid())
	{
	}

	SubjectJoinedQueryData(SubjectQueryIdentity identity, XmlQueryDocText query_xml, UUID uuid, UUID base_query_uuid, UUID subject_query_uuid):
		m_identity(identity),
		m_query_xml(std::move(query_xml)),
		m_uuid(uuid),
		m_base_query_uuid(base_query_uuid),
		m_subject_query_uuid(subject_query_uuid)
	{
	}

	SubjectQueryIdentity			m_identity;
	XmlQueryDocText					m_query_xml;
	UUID							m_uuid;

	// joined queries
	UUID							m_base_query_uuid;
	UUID							m_subject_query_uuid;	
};

#pragma db view object(SubjectJoinedQueryData)
struct SubjectJoinedQueryView
{
	SubjectQueryIdentity			m_identity;
};

#pragma db object(QueryJoiningData)
#pragma db member(QueryJoiningData::m_key) id auto
#pragma db value(SubjectQueryIdentity)
#pragma db object(SubjectQueryData)
#pragma db member(SubjectQueryData::m_identity) id
#pragma db object(SubjectJoinedQueryData)
#pragma db member(SubjectJoinedQueryData::m_identity) id
#pragma warning(pop)

}} //namespace CI { namespace Core {