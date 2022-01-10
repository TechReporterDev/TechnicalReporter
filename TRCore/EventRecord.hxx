#pragma once
#include "STLTools\uuid.h"
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "ServicesValues.hxx"
#include "AppValues.hxx"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

enum class EventType;
enum class EventSeverity;

struct EventRecordData
{
	EventRecordData():
		m_key(0),
		m_event_type(EventType(0)),
		m_severity(EventSeverity(0)),
		m_time(0)
    {
    }

	EventRecordData(Key	key, SourceKey source_key, ResourceIdentity resource_uuid, EventType event_type, EventSeverity severity, time_t time):
		m_key(key),
		m_source_key(source_key),
		m_resource_uuid(resource_uuid),
		m_event_type(event_type),
		m_severity(severity),
		m_time(time)
    {
    }

	Key					m_key;
	SourceKey			m_source_key;
	ResourceIdentity	m_resource_uuid;
	EventType			m_event_type;
	EventSeverity		m_severity;
	time_t				m_time;
};

#pragma db object(EventRecordData)
#pragma db member(EventRecordData::m_key) id
#pragma warning(pop)

}} //namespace CI { namespace Core {