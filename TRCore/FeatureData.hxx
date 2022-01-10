#pragma once
#include "TrivialTypes.h"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

struct FeatureData
{
	FeatureData():
		m_installed(0)
	{
	}

	FeatureData(UUID uuid, bool	installed):
		m_uuid(uuid),
		m_installed(installed)
	{
	}

	UUID	m_uuid;
	bool	m_installed;
};

#pragma db object(FeatureData)
#pragma db member(FeatureData::m_uuid) id
#pragma warning(pop)

}} //namespace CI { namespace Core {