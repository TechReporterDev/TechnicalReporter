#pragma once
#include "..\STLTools\uuid.h"
#include "BasisTrivialTypes.h"
#include "RegistryTrivialTypes.h"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

struct StreamRecordData
{
    StreamRecordData():
        m_key(0),
        m_source_key(0),
        m_stream_type_uuid(stl_tools::null_uuid()),
        m_time(0)
    {
    }

    StreamRecordData(int key, Key source_key, StreamTypeUUID stream_type_uuid, std::string message, time_t time):
        m_key(key),
        m_source_key(source_key),
        m_stream_type_uuid(stream_type_uuid),
        m_message(std::move(message)),
        m_time(time)
    {
    }

    int                     m_key;
    SourceKey               m_source_key;
    StreamTypeUUID          m_stream_type_uuid;
    std::string             m_message;
    time_t                  m_time;
};

#pragma warning(pop)

}} //namespace CI { namespace Core {