#pragma once
#include <vector>
#include <boost\optional.hpp>
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "ServicesValues.hxx"

namespace TR {	namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

struct CustomQueryData
{
	CustomQueryData():
		m_key(0),
		m_output_traits(0)
	{
	}

	CustomQueryData(Key key, std::string name, std::vector<ReportTypeUUID> input_uuids, XmlQueryDocText query_doc, XmlDefDocText query_def, ReportTypeUUID output_uuid, XmlDefDocText output_def, int output_traits):
		m_key(key),
		m_name(std::move(name)),
		m_input_uuids(std::move(input_uuids)),		
		m_query_doc(std::move(query_doc)),
		m_query_def(std::move(query_def)),
		m_output_uuid(output_uuid),
		m_output_def(std::move(output_def)),
		m_output_traits(output_traits)
	{
	}

	Key								m_key;
	std::string						m_name;
	std::vector<ReportTypeUUID>		m_input_uuids;
	XmlQueryDocText					m_query_doc;
	XmlDefDocText					m_query_def;
	ReportTypeUUID					m_output_uuid;
	XmlDefDocText					m_output_def;
	int								m_output_traits;
};

#pragma db object(CustomQueryData)
#pragma db member(CustomQueryData::m_key) id auto
#pragma warning(pop)

}} //namespace CI { namespace Core {