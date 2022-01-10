#include "stdafx.h"
#include "XmlStreamTypeDef.h"
namespace TR {  namespace External {
using namespace xml_tools;

StreamTypeUUID XmlStreamTypeDef::get_uuid() const
{
    auto uuid_node = find_child(*this, "uuid");
    if (!uuid_node)
    {
        throw std::logic_error("Invalid stream type definition");
    }

    return StreamTypeUUID(stl_tools::gen_uuid(get_node_wtext(*uuid_node)));
}

std::wstring XmlStreamTypeDef::get_name() const
{
    auto name_node = find_child(*this, "name");
    if (!name_node)
    {
        throw std::logic_error("Invalid stream type definition");
    }

    return get_node_wtext(*name_node);
}


const XmlDefNode& XmlStreamTypeDef::get_message_def() const
{
    auto message_def = find_child(*this, "message_definition");
    if (!message_def)
    {
        throw std::logic_error("Invalid stream type definition");
    }

    auto message_def_root = begin_child(*message_def);
    if (!message_def_root)
    {
        throw std::logic_error("Invalid stream type definition");
    }

    return static_cast<const XmlDefNode&>(*message_def_root);
}

}} // namespace TR { namespace External {