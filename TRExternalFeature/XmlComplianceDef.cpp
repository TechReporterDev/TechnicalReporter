#include "stdafx.h"
#include "XmlComplianceDef.h"
namespace TR { namespace External {
using namespace xml_tools;

std::wstring XmlComplianceDef::get_name() const
{
    auto name_node = find_child(*this, "name");
    if (!name_node)
    {
        throw std::logic_error("Invalid compliance definition");
    }
    return get_node_wtext(*name_node);
}

ReportTypeUUID XmlComplianceDef::get_input_uuid() const
{
    auto input_uuid_node = find_child(*this, "input_uuid");
    if (!input_uuid_node)
    {
        throw std::logic_error("Invalid compliance definition");
    }   

    return ReportTypeUUID(stl_tools::gen_uuid(get_node_wtext(*input_uuid_node)));
}

ReportTypeUUID XmlComplianceDef::get_output_uuid() const
{
    auto output_uuid_node = find_child(*this, "output_uuid");
    if (!output_uuid_node)
    {
        throw std::logic_error("Invalid compliance definition");
    }

    return ReportTypeUUID(stl_tools::gen_uuid(get_node_wtext(*output_uuid_node)));
}

}}// namespace TR { namespace External {