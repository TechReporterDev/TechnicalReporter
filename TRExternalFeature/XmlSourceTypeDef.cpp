#include "stdafx.h"
#include "XmlSourceTypeDef.h"
namespace TR { namespace External {
using namespace xml_tools;

const XmlSourceTypeDef::XmlDownloadNode* XmlSourceTypeDef::next_download_node(const XmlDownloadNode* download_node)
{
    _ASSERT(download_node && node_name_equal(*download_node, "download"));
    return static_cast<const XmlDownloadNode*>(find_sibling(*download_node, "download"));
}

const XmlSourceTypeDef::XmlStreamNode* XmlSourceTypeDef::next_stream_node(const XmlStreamNode* stream_node)
{
    _ASSERT(stream_node && node_name_equal(*stream_node, "stream"));
    return static_cast<const XmlStreamNode*>(find_sibling(*stream_node, "stream"));
}

const XmlSourceTypeDef::XmlActionNode* XmlSourceTypeDef::next_action_node(const XmlActionNode* action_node)
{
    _ASSERT(action_node && node_name_equal(*action_node, "action"));
    return static_cast<const XmlActionNode*>(find_sibling(*action_node, "action"));
}

ReportTypeUUID XmlSourceTypeDef::XmlDownloadNode::get_report_type_uuid() const
{
    auto report_type_uuid_node = find_child(*this, "uuid");
    if (!report_type_uuid_node)
    {
        throw std::logic_error("Invalid source type definition document");
    }
    return ReportTypeUUID(stl_tools::gen_uuid(get_node_wtext(*report_type_uuid_node)));
}

const XmlNode& XmlSourceTypeDef::XmlDownloadNode::get_command_node() const
{
    auto command_node = find_child(*this, "command");
    if (!command_node)
    {
        throw std::logic_error("Invalid source type definition document");
    }
    return *command_node;
}

bool XmlSourceTypeDef::XmlDownloadNode::must_discard_empty() const
{
    if (auto attr = find_attribute(*this, "discard-empty"))
    {
        if (attr_value_equal(*attr, "yes"))
        {
            return true;
        }
        else if (attr_value_equal(*attr, "no"))
        {
            return false;
        }
        else
        {
            throw std::logic_error("Invalid source type definition document");
        }
    }

    return false;
}

StreamTypeUUID XmlSourceTypeDef::XmlStreamNode::get_stream_type_uuid() const
{
    auto stream_type_uuid_node = find_child(*this, "uuid");
    if (!stream_type_uuid_node)
    {
        throw std::logic_error("Invalid source type definition document");
    }
    return StreamTypeUUID(stl_tools::gen_uuid(get_node_wtext(*stream_type_uuid_node)));
}

XmlSourceTypeDef::XmlStreamNode::Protocol XmlSourceTypeDef::XmlStreamNode::get_protocol() const
{
    auto protocol_node = find_child(*this, "protocol");
    if (!protocol_node)
    {
        throw std::logic_error("Invalid source type definition document");
    }

    if (node_text_equal(*protocol_node, "SYSLOG"))
    {
        return SYSLOG;
    }
    else
    {
        _ASSERT(false);
    }

    throw std::logic_error("Invalid source type definition document");
}

const XmlNode& XmlSourceTypeDef::XmlStreamNode::get_address_node() const
{
    auto address_node = find_child(*this, "address");
    if (!address_node)
    {
        throw std::logic_error("Invalid source type definition document");
    }
    return *address_node;
}

const XmlNode& XmlSourceTypeDef::XmlStreamNode::get_parser_node() const
{
    auto parser_node = find_child(*this, "regex_parser");
    if (!parser_node)
    {
        throw std::logic_error("Invalid source type definition document");
    }

    auto parser_root = begin_child(*parser_node);
    if (!parser_root)
    {
        throw std::logic_error("Invalid source type definition document");
    }

    return *parser_root;
}

const XmlNode& XmlSourceTypeDef::XmlStreamNode::get_format_node() const
{
    auto format_node = find_child(*this, "format_script");
    if (!format_node)
    {
        throw std::logic_error("Invalid source type definition document");
    }

    auto xslt_root = begin_child(*format_node);
    if (!xslt_root)
    {
        throw std::logic_error("Invalid source type definition document");
    }

    return *xslt_root;
}

ActionUUID XmlSourceTypeDef::XmlActionNode::get_action_uuid() const
{
    auto action_uuid_node = find_child(*this, "uuid");
    if (!action_uuid_node)
    {
        throw std::logic_error("Invalid source type definition document");
    }
    return ActionUUID(stl_tools::gen_uuid(get_node_wtext(*action_uuid_node)));
}

const XmlNode& XmlSourceTypeDef::XmlActionNode::get_command_node() const
{
    auto command_node = find_child(*this, "command");
    if (!command_node)
    {
        throw std::logic_error("Invalid source type definition document");
    }
    return *command_node;
}

std::vector<ReportTypeUUID> XmlSourceTypeDef::XmlActionNode::get_reload_uuids() const
{
    auto reload_node = find_child(*this, "reload");
    if (!reload_node)
    {
        return{};
    }

    std::vector<ReportTypeUUID> reload_uuids;
    for (auto& report_type_uuid_node : get_child_nodes(*reload_node))
    {
        reload_uuids.emplace_back(stl_tools::gen_uuid(get_node_wtext(report_type_uuid_node)));
    }
    return reload_uuids;
}

UUID XmlSourceTypeDef::get_uuid() const
{
    auto uuid_node = find_child(*this, "uuid");
    if (!uuid_node)
    {
        throw std::logic_error("Invalid source type definition");
    }

    return stl_tools::gen_uuid(get_node_wtext(*uuid_node));
}

std::wstring XmlSourceTypeDef::get_name() const
{
    auto name_node = find_child(*this, "name");
    if (!name_node)
    {
        throw std::logic_error("Invalid source type definition");
    }

    return get_node_wtext(*name_node);
}

std::string XmlSourceTypeDef::get_settings() const
{
    auto settings_node = find_child(*this, "settings");
    if (!settings_node)
    {
        throw std::logic_error("Invalid source type definition");
    }

    auto settings = xml_tools::create_doc();
    xml_tools::set_root(*settings, xml_tools::clone_node(*settings_node));
    return as_string(*settings);
}

XmlSourceTypeDef::ConstDownloadNodeRange XmlSourceTypeDef::get_downloads() const
{
    auto downloads_node = find_child(*this, "downloads");
    if (!downloads_node)
    {
        throw std::logic_error("Invalid source type definition");
    }

    auto begin = ConstDownloadNodeIterator(static_cast<const XmlDownloadNode*>(find_child(*downloads_node, "download")));
    return ConstDownloadNodeRange(begin, ConstDownloadNodeIterator());
}

XmlSourceTypeDef::ConstStreamNodeRange XmlSourceTypeDef::get_streams() const
{
    auto streams_node = find_child(*this, "streams");
    if (!streams_node)
    {
        throw std::logic_error("Invalid source type definition");
    }

    auto begin = ConstStreamNodeIterator(static_cast<const XmlStreamNode*>(find_child(*streams_node, "stream")));
    return ConstStreamNodeRange(begin, ConstStreamNodeIterator());
}

XmlSourceTypeDef::ConstActionNodeRange XmlSourceTypeDef::get_actions() const
{
    auto actions_node = find_child(*this, "actions");
    if (!actions_node)
    {
        throw std::logic_error("Invalid source type definition document");
    }

    auto begin = ConstActionNodeIterator(static_cast<const XmlActionNode*>(find_child(*actions_node, "action")));
    return ConstActionNodeRange(begin, ConstActionNodeIterator());
}

}} //namespace TR { namespace External {