#pragma once
#include <boost\optional.hpp>
#include "..\STLTools\uuid.h"
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "StreamRecordData.h"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

struct StreamInputData
{
	StreamInputData():
		m_key(0),
		m_source_key(0),
		m_stream_type_uuid(stl_tools::null_uuid())
	{
	}

	StreamInputData(int key, Key source_key, StreamTypeUUID stream_type_uuid, StreamRecordData input_record):
		m_key(key),
		m_source_key(source_key),
		m_stream_type_uuid(stream_type_uuid),
		m_input_record(std::move(input_record))
	{
	}

	int						m_key;
	SourceKey				m_source_key;
	StreamTypeUUID			m_stream_type_uuid;
	StreamRecordData		m_input_record;
};

#pragma db value(StreamRecordData) definition
#pragma db object(StreamInputData)
#pragma db member(StreamInputData::m_key) id auto

#pragma warning(pop)

}} //namespace CI { namespace Core {