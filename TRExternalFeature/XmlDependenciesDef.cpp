#include "stdafx.h"
#include "XmlDependenciesDef.h"
namespace TR { namespace External {
using namespace xml_tools;

UUID XmlResourceDef::get_resource_uuid() const
{
    auto uuid_node = find_child(*this, "uuid");
    if (!uuid_node)
    {
        throw std::logic_error("Invalid resource definition");
    }

    return stl_tools::gen_uuid(get_node_wtext(*uuid_node));
}

std::wstring XmlResourceDef::get_resource_name() const
{
    auto name_node = find_child(*this, "name");
    if (!name_node)
    {
        throw std::logic_error("Invalid resource definition");
    }

    return get_node_wtext(*name_node);
}

const XmlResourceDef* XmlDependencyDef::next_resource_def(const XmlResourceDef* resource_def)
{
    return static_cast<const XmlResourceDef*>(xml_tools::next_element(*resource_def));
}

UUID XmlDependencyDef::get_feature_uuid() const
{
    auto uuid_node = find_child(*this, "uuid");
    if (!uuid_node)
    {
        throw std::logic_error("Invalid dependency definition");
    }

    return stl_tools::gen_uuid(get_node_wtext(*uuid_node));
}

std::wstring XmlDependencyDef::get_feature_name() const
{
    auto name_node = find_child(*this, "name");
    if (!name_node)
    {
        throw std::logic_error("Invalid dependency definition");
    }

    return get_node_wtext(*name_node);
}

XmlDependencyDef::ConstResourceDefRange XmlDependencyDef::get_resources() const
{
    auto begin = ConstResourceDefIterator(static_cast<const XmlResourceDef*>(begin_element(*this)));
    return ConstResourceDefRange(begin, ConstResourceDefIterator());
}

const XmlDependencyDef* XmlDependenciesDef::next_dependency_def(const XmlDependencyDef* dependency_def)
{
    return static_cast<const XmlDependencyDef*>(xml_tools::find_sibling(*dependency_def, "feature"));
}

XmlDependenciesDef::ConstDependencyDefRange XmlDependenciesDef::get_dependencies() const
{
    auto begin = ConstDependencyDefIterator(static_cast<const XmlDependencyDef*>(find_child(*this, "feature")));
    return ConstDependencyDefRange(begin, ConstDependencyDefIterator());
}

}}// namespace TR { namespace External {