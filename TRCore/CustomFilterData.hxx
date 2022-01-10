#pragma once
#include <vector>
#include <boost\optional.hpp>
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "ServicesValues.hxx"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

struct CustomFilterData
{
	CustomFilterData():
		m_key(0)
	{
	}

	CustomFilterData(Key key, std::string name, ReportTypeUUID input_uuid, ReportTypeUUID output_uuid, Blob node_filters):
		m_key(key),
		m_name(std::move(name)),
		m_input_uuid(input_uuid),
		m_output_uuid(output_uuid),
		m_node_filters(std::move(node_filters))
	{
	}

	Key					m_key;
	std::string			m_name;
	ReportTypeUUID		m_input_uuid;
	ReportTypeUUID		m_output_uuid;
	Blob				m_node_filters;
};

#pragma db value(Blob) type("LONGBLOB")
#pragma db object(CustomFilterData)
#pragma db member(CustomFilterData::m_key) id auto
#pragma warning(pop)

}} //namespace CI { namespace Core {