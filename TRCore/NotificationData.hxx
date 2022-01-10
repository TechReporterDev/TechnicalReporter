#pragma once
#include "STLTools\uuid.h"
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "ServicesValues.hxx"
#include "AppValues.hxx"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)
	
enum class NotificationSeverity;
enum class NotificationCause;

struct NotificationIdentity
{
	NotificationIdentity():
		m_user_key(0),
		m_cause(NotificationCause(0))
    {
    }

	NotificationIdentity(Key user_key, NotificationCause cause, SubjectIdentity subject, ResourceIdentity resource):
		m_user_key(user_key),
		m_cause(cause),
		m_subject(subject),
		m_resource(resource)
    {
    }

	Key m_user_key;
	NotificationCause m_cause;
	SubjectIdentity m_subject;
	ResourceIdentity m_resource;
};

struct NotificationData
{
	NotificationData():
		m_severity(NotificationSeverity(0))
    {
    }

	NotificationData(NotificationIdentity identity, NotificationSeverity severity = NotificationSeverity(0)):
		m_identity(identity),
		m_severity(severity)
    {
	}

	NotificationIdentity m_identity;
	NotificationSeverity m_severity;
};

#pragma db value(NotificationIdentity)
#pragma db object(NotificationData)
#pragma db member(NotificationData::m_identity) id
#pragma warning(pop)

}} //namespace CI { namespace Core {