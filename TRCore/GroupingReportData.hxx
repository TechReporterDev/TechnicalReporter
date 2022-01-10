#pragma once
#include <boost\optional.hpp>
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "ServicesValues.hxx"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

struct GroupingReportData
{
	GroupingReportData():
		m_key(0),
		m_output_traits(0)
	{
	}

	GroupingReportData(Key key, std::string name, ReportTypeUUID input_uuid, ReportTypeUUID output_uuid, int output_traits, ReportTypeUUID childs_grouping_uuid, boost::optional<SourceKey> bound_source_key):
		m_key(key),
		m_name(std::move(name)),
		m_input_uuid(input_uuid),
		m_output_uuid(output_uuid),
		m_output_traits(output_traits),
		m_childs_grouping_uuid(childs_grouping_uuid),
		m_bound_source_key(bound_source_key)
	{
	}

	Key								m_key;
	std::string						m_name;
	ReportTypeUUID					m_input_uuid;
	ReportTypeUUID					m_output_uuid;
	int								m_output_traits;
	ReportTypeUUID					m_childs_grouping_uuid;
	boost::optional<SourceKey>		m_bound_source_key;
};

#pragma db object(GroupingReportData)
#pragma db member(GroupingReportData::m_key) id auto
#pragma warning(pop)

}} //namespace CI { namespace Core {