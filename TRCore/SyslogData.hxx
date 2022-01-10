#pragma once
#include "BasisValues.hxx"
#include "RegistryValues.hxx"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

struct SyslogIdentity
{
	SyslogIdentity()
	{
	}

	SyslogIdentity(SourceKey source_key, StreamTypeUUID stream_type_uuid):
		m_source_key(source_key),
		m_stream_type_uuid(stream_type_uuid)
	{
	}

	SourceKey		m_source_key;
	StreamTypeUUID	m_stream_type_uuid;
};

struct SyslogData
{
	SyslogData()
	{
	}

	SyslogData(SourceKey source_key, StreamTypeUUID stream_type_uuid, std::string address):
		m_identity(source_key, stream_type_uuid),
		m_address(std::move(address))
	{
	}

	SyslogIdentity	m_identity;
	std::string		m_address;
};

#pragma db value(SyslogIdentity)
#pragma db object(SyslogData)
#pragma db member(SyslogData::m_identity) id

#pragma warning(pop)
}} //namespace CI { namespace Core {