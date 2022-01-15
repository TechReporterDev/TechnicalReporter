#pragma once
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include <boost\optional.hpp>

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

struct SourceData
{
	SourceData():
		m_key(0),
		m_role_key(0),
		m_source_type_uuid(stl_tools::null_uuid())
	{
	}

	SourceData(Key key, std::string name, boost::optional<SourceKey> parent_key, Key role_key, SourceTypeUUID source_type_uuid, std::string config):
		m_key(key), 
		m_name(std::move(name)),
		m_parent_key(parent_key),
		m_role_key(role_key),
		m_source_type_uuid(source_type_uuid),		
		m_config(std::move(config))
	{		
	}

	Key									m_key;
	std::string							m_name;	
	boost::optional<SourceKey>			m_parent_key;
	Key									m_role_key;
	SourceTypeUUID						m_source_type_uuid;
	std::string							m_config;
};

#pragma db value(Blob) type("LONGBLOB")
#pragma db object(SourceData)
#pragma db member(SourceData::m_key) id auto
#pragma warning(pop)

}} //namespace CI { namespace Core {