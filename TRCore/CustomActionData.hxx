#pragma once
#include <boost\optional.hpp>
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "ServicesValues.hxx"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

struct CustomActionData
{
	CustomActionData():
		m_uuid(stl_tools::null_uuid()),
		m_action_uuid(stl_tools::null_uuid())
	{
	}

	CustomActionData(std::string name, UUID	uuid, UUID action_uuid, std::string params_doc, std::string params_def, boost::optional<ReportTypeUUID> output_uuid):
		m_name(std::move(name)),
		m_uuid(uuid),
		m_action_uuid(action_uuid),
		m_params_doc(std::move(params_doc)),
		m_params_def(std::move(params_def)),
		m_output_uuid(output_uuid)
	{
	}

	std::string							m_name;
	UUID								m_uuid;
	ActionUUID							m_action_uuid;

	std::string							m_params_doc;
	std::string							m_params_def;

	boost::optional<ReportTypeUUID>		m_output_uuid;
};

#pragma db object(CustomActionData)
#pragma db member(CustomActionData::m_uuid) id
#pragma warning(pop)

}} //namespace CI { namespace Core {