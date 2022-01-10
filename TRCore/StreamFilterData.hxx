#pragma once
#include <vector>
#include <boost\optional.hpp>
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "ServicesValues.hxx"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

struct StreamFilterData
{
	StreamFilterData():
		m_key(0)
	{
	}

	StreamFilterData(Key key, std::string name, StreamTypeUUID input_uuid, StreamTypeUUID output_uuid, XmlQueryDocText filter_doc, XmlDefDocText filter_def):
		m_key(key),
		m_name(std::move(name)),
		m_input_uuid(input_uuid),
		m_output_uuid(output_uuid),
		m_filter_doc(std::move(filter_doc)),
		m_filter_def(std::move(filter_def))
	{
	}

	Key m_key;
	std::string m_name;
	StreamTypeUUID m_input_uuid;
	StreamTypeUUID m_output_uuid;
	XmlQueryDocText m_filter_doc;
	XmlDefDocText m_filter_def;
};

#pragma db object(StreamFilterData)
#pragma db member(StreamFilterData::m_key) id auto
#pragma warning(pop)

}} //namespace CI { namespace Core {