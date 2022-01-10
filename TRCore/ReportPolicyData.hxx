#pragma once
#include "BasisValues.hxx"
#include "RegistryValues.hxx"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

enum class EnablePolicy;
enum class Visibility;
enum class DeferPolicy;

struct ResourcePolicyIdentity
{
	ResourcePolicyIdentity()
	{
	}

	ResourcePolicyIdentity(SubjectIdentity subject, ResourceIdentity resource):
		m_subject(subject),
		m_resource(resource)
	{
	}

	SubjectIdentity		m_subject;
	ResourceIdentity	m_resource;
};

inline bool operator < (const ResourcePolicyIdentity& left, const ResourcePolicyIdentity& right)
{
	if (left.m_subject < right.m_subject)
	{
		return true;
	}

	if (left.m_subject > right.m_subject)
	{
		return false;
	}

	return left.m_resource < right.m_resource;
}

struct ResourcePolicyData
{
	ResourcePolicyData():
		m_enable_policy(EnablePolicy(0)),
		m_visibility(Visibility(0)),
		m_defer_policy(DeferPolicy(0))
	{
	}

	ResourcePolicyData(ResourcePolicyIdentity identity):
		m_identity(identity),
		m_enable_policy(EnablePolicy(0)),
		m_visibility(Visibility(0)),
		m_defer_policy(DeferPolicy(0)),
		m_setup_uuid(stl_tools::null_uuid())
	{
	}

	ResourcePolicyData(ResourcePolicyIdentity identity, EnablePolicy enable_policy, Visibility visibility, DeferPolicy defer_policy, UUID setup_uuid):
		m_identity(identity),
		m_enable_policy(enable_policy),
		m_visibility(visibility),
		m_defer_policy(defer_policy),
		m_setup_uuid(setup_uuid)
	{
	}

	bool is_default() const
	{
		return m_enable_policy == EnablePolicy(0) && m_visibility == Visibility(0) && m_defer_policy == DeferPolicy(0) && m_setup_uuid == stl_tools::null_uuid();
	}

	ResourcePolicyIdentity		m_identity;
	EnablePolicy				m_enable_policy;
	Visibility					m_visibility;
	DeferPolicy					m_defer_policy;
	UUID						m_setup_uuid;
};

#pragma db value(Blob) type("LONGBLOB")
#pragma db value(ResourcePolicyIdentity)
#pragma db object(ResourcePolicyData)
#pragma db member(ResourcePolicyData::m_identity) id
#pragma warning(pop)

}} //namespace CI { namespace Core {