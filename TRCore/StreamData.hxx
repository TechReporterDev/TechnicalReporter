#pragma once
#include <boost\optional.hpp>
#include "STLTools\uuid.h"
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
namespace TR { namespace Core {

#pragma warning(push)
#pragma warning(disable:4068)

struct StreamIdentity
{
	StreamIdentity()
	{
	}

	StreamIdentity(SourceKey source_key, StreamTypeUUID stream_type_uuid):
		m_source_key(source_key),
		m_stream_type_uuid(stream_type_uuid)
	{
	}

	SourceKey			m_source_key;
	StreamTypeUUID		m_stream_type_uuid;
};

inline bool operator < (const StreamIdentity& left, const StreamIdentity& right)
{
	if (left.m_source_key < right.m_source_key)
	{
		return true;
	}

	if (left.m_source_key > right.m_source_key)
	{
		return false;
	}

	return left.m_stream_type_uuid < right.m_stream_type_uuid;
}

struct StreamData
{
	StreamData():
		m_first(stl_tools::null_uuid()),
		m_last(stl_tools::null_uuid()),
		m_input(stl_tools::null_uuid())
	{
	}

	StreamData(StreamIdentity identity) :
		m_identity(identity),
		m_first(stl_tools::null_uuid()),
		m_last(stl_tools::null_uuid()),
		m_input(stl_tools::null_uuid())
	{
	}

	StreamIdentity		m_identity;
	UUID				m_first;
	UUID				m_last;
	UUID				m_input;
};

#pragma db value(StreamIdentity)
#pragma db object(StreamData)
#pragma db member(StreamData::m_identity) id

#pragma warning(pop)

}} //namespace CI { namespace Core {