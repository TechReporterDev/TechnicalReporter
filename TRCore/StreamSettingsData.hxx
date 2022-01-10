#pragma once
#include <boost\optional.hpp>
#include "STLTools\uuid.h"
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
namespace TR { namespace Core {

#pragma warning(push)
#pragma warning(disable:4068)

enum class StreamLength;

struct StreamSettingsIdentity
{
	StreamSettingsIdentity()
	{
	}

	StreamSettingsIdentity(SubjectIdentity subject, StreamTypeUUID stream_type_uuid):
		m_subject(subject),
		m_stream_type_uuid(stream_type_uuid)
	{
	}

	SubjectIdentity		m_subject;
	StreamTypeUUID		m_stream_type_uuid;
};

struct StreamSettingsData
{
	StreamSettingsData():
		m_length(StreamLength(0))
	{
	}

	StreamSettingsData(StreamSettingsIdentity stream_settings_identity, StreamLength length):
		m_stream_settings_identity(stream_settings_identity),
		m_length(length)
	{
	}

	StreamSettingsIdentity	m_stream_settings_identity;
	StreamLength			m_length;
};

#pragma db value(StreamSettingsIdentity)
#pragma db object(StreamSettingsData)
#pragma db member(StreamSettingsData::m_stream_settings_identity) id

#pragma warning(pop)

}} //namespace CI { namespace Core {