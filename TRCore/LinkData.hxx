#pragma once
#include <utility>
#include "RegistryValues.hxx"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

typedef std::pair<SourceKey, SourceKey> LinkID;
#pragma db value(LinkID)

struct LinkData
{
	LinkData():
		m_parent_key(0),
		m_child_key(0)
	{
	}

	LinkData(SourceKey parent_key, SourceKey child_key):
		m_parent_key(parent_key),
		m_child_key(child_key)
	{
	}

	SourceKey	m_parent_key;
	SourceKey	m_child_key;
};

#pragma db object(LinkData)
#pragma db member(LinkData::m_parent_key) transient
#pragma db member(LinkData::m_child_key) transient
#pragma db member(LinkData::ID) virtual(LinkID) id \
             get(::TR::Core::LinkID(this.m_parent_key, this.m_child_key)) \
             set(this.m_parent_key = (?).first; this.m_child_key = (?).second)

#pragma warning(pop)

}} //namespace CI { namespace Core {