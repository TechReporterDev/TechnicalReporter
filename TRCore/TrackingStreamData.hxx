#pragma once
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "ServicesValues.hxx"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

struct TrackingStreamData
{
	TrackingStreamData():
		m_key(0)
	{
	}

	TrackingStreamData(Key key, std::string name, ReportTypeUUID input_uuid, std::string input_xpath, StreamTypeUUID output_uuid):
		m_key(key),
		m_name(std::move(name)),
		m_input_uuid(input_uuid),
		m_input_xpath(std::move(input_xpath)),
		m_output_uuid(output_uuid)
	{
	}

	Key					m_key;
	std::string			m_name;
	ReportTypeUUID		m_input_uuid;
	std::string			m_input_xpath;
	StreamTypeUUID		m_output_uuid;
};

#pragma db object(TrackingStreamData)
#pragma db member(TrackingStreamData::m_key) id auto
#pragma warning(pop)

}} //namespace CI { namespace Core {