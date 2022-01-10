#pragma once
#include <boost\optional.hpp>
#include "RegistryValues.hxx"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

struct RoleData
{
	RoleData():
		m_key(0),
		m_parent_key(0)
	{
	}

	RoleData(Key key, std::string name, boost::optional<RoleKey> parent_key):
		m_key(key),
		m_name(std::move(name)),
		m_parent_key(parent_key)
	{
	}

	Key							m_key;
	std::string					m_name;
	boost::optional<RoleKey>	m_parent_key;
};

#pragma db value(Blob) type("LONGBLOB")
#pragma db object(RoleData)
#pragma db member(RoleData::m_key) id auto
#pragma warning(pop)

}} //namespace CI { namespace Core {