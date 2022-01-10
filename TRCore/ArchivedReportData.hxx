#pragma once
#include <boost\optional.hpp>
#include "STLTools\uuid.h"
#include "BasisValues.hxx"
#include "RegistryValues.hxx"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

struct ArchivedReportData
{
	ArchivedReportData():
		m_key(0),
		m_source_key(0),
		m_report_type_uuid(stl_tools::null_uuid()),
		m_time(0),
		m_off_time(0)
	{
	}

	ArchivedReportData(Key key, SourceKey source_key, ReportTypeUUID report_type_uuid, boost::optional<Blob> diff, time_t time, time_t off_time):
		m_key(key),
		m_source_key(source_key),
		m_report_type_uuid(report_type_uuid),
		m_diff(std::move(diff)),
		m_time(time),
		m_off_time(off_time)
	{
	}
	
	Key						m_key;
	SourceKey				m_source_key;
	ReportTypeUUID			m_report_type_uuid;
	boost::optional<Blob>	m_diff;
	time_t					m_time;
	time_t					m_off_time;
};

struct ArchivedReportContentData
{
	ArchivedReportContentData():
		m_key(0)
	{
	}

	ArchivedReportContentData(Key key, Blob content):
		m_key(key),
		m_content(std::move(content))		
	{
	}

	Key						m_key;
	Blob					m_content;
};

#pragma db value(Blob) type("LONGBLOB")
#pragma db object(ArchivedReportData)
#pragma db member(ArchivedReportData::m_key) id auto
#pragma db member(ArchivedReportData::m_source_key) index
#pragma db member(ArchivedReportData::m_report_type_uuid) index
#pragma db member(ArchivedReportData::m_time) index
#pragma db member(ArchivedReportData::m_off_time) index

#pragma db object(ArchivedReportContentData)
#pragma db member(ArchivedReportContentData::m_key) id
#pragma warning(pop)

}} //namespace CI { namespace Core {