#pragma once
#include <boost\optional.hpp>
#include "..\STLTools\uuid.h"
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "StreamRecordData.h"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

#pragma db object(StreamRecordData) definition
#pragma db member(StreamRecordData::m_key) id auto

#pragma warning(pop)

}} //namespace CI { namespace Core {