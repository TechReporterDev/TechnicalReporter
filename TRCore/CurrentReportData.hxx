#pragma once
#include <boost\optional.hpp>
#include "STLTools\uuid.h"
#include "BasisValues.hxx"
#include "RegistryValues.hxx"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

struct CurrentReportIdentity
{
	CurrentReportIdentity():
		m_source_key(0),
		m_report_type_uuid(stl_tools::null_uuid())
	{
	}

	CurrentReportIdentity(Key source_key, UUID report_type_uuid):
		m_source_key(source_key),
		m_report_type_uuid(report_type_uuid)
	{
	}

	SourceKey			m_source_key;
	ReportTypeUUID		m_report_type_uuid;
};

inline bool operator < (const CurrentReportIdentity& left, const CurrentReportIdentity& right)
{
	if (left.m_source_key < right.m_source_key)
	{
		return true;
	}

	if (left.m_source_key > right.m_source_key)
	{
		return false;
	}

	return left.m_report_type_uuid < right.m_report_type_uuid;
}

struct TransformationCircumstancesData
{
	TransformationCircumstancesData():
		m_transformation_uuid(stl_tools::null_uuid()),
		m_setup_uuid(stl_tools::null_uuid())
	{
	}

	TransformationCircumstancesData(UUID transformation_uuid, UUID setup_uuid, std::vector<UUID> input_uuids):
		m_transformation_uuid(std::move(transformation_uuid)),
		m_setup_uuid(std::move(setup_uuid)),
		m_input_uuids(std::move(input_uuids))
	{
	}

	UUID					m_transformation_uuid;
	UUID					m_setup_uuid;
	std::vector<UUID>		m_input_uuids;
};

struct CurrentReportData
{
	CurrentReportData():
		m_time(0),
		m_check_time(0),
		m_uuid(stl_tools::null_uuid())
	{
	}

	CurrentReportData(Key source_key, UUID report_type_uuid, boost::optional<Blob> diff, time_t time, time_t check_time, UUID uuid, TransformationCircumstancesData transformation_circumstances):
		m_identity(source_key, report_type_uuid),
		m_diff(std::move(diff)),
		m_time(time),
		m_check_time(check_time),
		m_uuid(uuid),
		m_transformation_circumstances(std::move(transformation_circumstances))
	{
	}

	CurrentReportIdentity			m_identity;
	boost::optional<Blob>			m_diff;
	time_t							m_time;
	time_t							m_check_time;
	UUID							m_uuid;
	TransformationCircumstancesData m_transformation_circumstances;
};

struct CurrentReportContentData
{
	CurrentReportContentData()
	{
	}

	CurrentReportContentData(Key source_key, UUID report_type_uuid, Blob content):
		m_identity(source_key, report_type_uuid),
		m_content(std::move(content))
	{
	}

	CurrentReportIdentity	m_identity;
	Blob					m_content;
};

#pragma db value(Blob) type("LONGBLOB")
#pragma db value(CurrentReportIdentity)
#pragma db value(TransformationCircumstancesData)
#pragma db object(CurrentReportData)
#pragma db member(CurrentReportData::m_identity) id
#pragma db object(CurrentReportContentData)
#pragma db member(CurrentReportContentData::m_identity) id

#pragma warning(pop)

}} //namespace CI { namespace Core {