#pragma once
#include "BasisValues.hxx"
#include "RegistryValues.hxx"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

enum class ArchivingPolicy;
enum class ArchivedExpiration;

struct ArchivingSettingsIdentity
{
	ArchivingSettingsIdentity()
	{
	}

	ArchivingSettingsIdentity(SubjectIdentity subject, ReportTypeUUID report_type_uuid):
		m_subject(subject),
		m_report_type_uuid(report_type_uuid)
	{
	}

	SubjectIdentity		m_subject;
	ReportTypeUUID		m_report_type_uuid;
};

inline bool operator < (const ArchivingSettingsIdentity& left, const ArchivingSettingsIdentity& right)
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

inline bool operator == (const ArchivingSettingsIdentity& left, const ArchivingSettingsIdentity& right)
{
	return left.m_subject == right.m_subject && left.m_report_type_uuid == right.m_report_type_uuid;
}

struct ArchivingSettingsData
{
	ArchivingSettingsData():
		m_archiving_policy(ArchivingPolicy(0)),
		m_archived_expiration(ArchivedExpiration(0))
	{
	}

	ArchivingSettingsData(ArchivingSettingsIdentity identity, ArchivingPolicy archiving_policy = ArchivingPolicy(0), ArchivedExpiration archived_expiration = ArchivedExpiration(0)):
		m_identity(identity),
		m_archiving_policy(archiving_policy),
		m_archived_expiration(archived_expiration)
	{
	}

	bool is_default() const
	{
		return m_archiving_policy == ArchivingPolicy(0) && m_archived_expiration == ArchivedExpiration(0);
	}

	ArchivingSettingsIdentity	m_identity;
	ArchivingPolicy				m_archiving_policy;
	ArchivedExpiration			m_archived_expiration;
};

#pragma db value(Blob) type("LONGBLOB")
#pragma db value(ArchivingSettingsIdentity)
#pragma db object(ArchivingSettingsData)
#pragma db member(ArchivingSettingsData::m_identity) id
#pragma warning(pop)

}} //namespace CI { namespace Core {