#pragma once
#include "BasisValues.hxx"
#include "RegistryValues.hxx"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

struct UserData
{
	UserData():
		m_key(0)
	{
	}

	UserData(Key key, std::string name):
		m_key(key),
		m_name(std::move(name))
	{
	}

	Key							m_key;

	#pragma db type("VARCHAR(128)")
	std::string					m_name;	
};

struct AccessEntryIdentity
{
	AccessEntryIdentity():
		m_source_key(0),
		m_user_key(0)
	{

	}

	AccessEntryIdentity(SourceKey source_key, Key user_key):
		m_source_key(source_key),
		m_user_key(user_key)
	{
	}

	SourceKey	m_source_key;
	Key			m_user_key;
};

inline bool operator < (const AccessEntryIdentity& left, const AccessEntryIdentity& right)
{
	if (left.m_source_key < right.m_source_key)
	{
		return true;
	}
	else if (right.m_source_key < left.m_source_key)
	{
		return false;
	}

	return left.m_user_key < right.m_user_key;
}

struct AccessEntryData
{
	AccessEntryData():
		m_access_rights(0)
	{
	}

	AccessEntryData(SourceKey source_key, Key user_key, int	access_rights):
		m_identity(source_key, user_key),
		m_access_rights(access_rights)
	{
	}

	AccessEntryIdentity		m_identity;
	int						m_access_rights;
};

#pragma db value(Blob) type("LONGBLOB")
#pragma db object(UserData)
#pragma db member(UserData::m_key) id auto
#pragma db index(UserData::"name_index") unique member(m_name)

#pragma db value(AccessEntryIdentity)
#pragma db object(AccessEntryData)
#pragma db member(AccessEntryData::m_identity) id

#pragma warning(pop)

}} //namespace CI { namespace Core {