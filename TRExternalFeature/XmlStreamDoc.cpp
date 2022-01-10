#include "stdafx.h"
#include "XmlStreamDoc.h"
#include "boost\lexical_cast.hpp"
namespace TR { namespace External {
using namespace xml_tools;

XmlMessageNode::Severety XmlMessageNode::get_severity() const
{
    auto severity_node = find_child(*this, "severity");
    if (!severity_node)
    {
        throw std::logic_error("Invalid stream record node");
    }

    if (node_text_equal(*severity_node, "INFO"))
    {
        return Severety::INFO;
    }
    else if (node_text_equal(*severity_node, "WARNING"))
    {
        return Severety::WARNING;
    }
    else if (node_text_equal(*severity_node, "FAILURE"))
    {
        return Severety::FAILURE;
    }
    else
    {
        _ASSERT(false);
        throw std::logic_error("Invalid stream record node");
    }
}

boost::optional<time_t> XmlMessageNode::get_time() const
{
    if (auto time_node = find_child(*this, "time"))
    {
        auto time_string = get_node_text(*time_node);
        return boost::lexical_cast<time_t>(time_string);
    }
    return boost::none;
}

std::wstring XmlMessageNode::get_event() const
{
    auto event_node = find_child(*this, "event");
    if (!event_node)
    {
        throw std::logic_error("Invalid stream record node");
    }
    return get_node_wtext(*event_node);
}

const XmlNode& XmlMessageNode::get_body() const
{
    auto message_node = find_child(*this, "body");
    if (!message_node)
    {
        throw std::logic_error("Invalid stream record node");
    }

    auto message_root = begin_child(*message_node);
    if (!message_root)
    {
        throw std::logic_error("Invalid stream record node");
    }

    return *message_root;
}

const XmlMessageNode* XmlStreamDoc::next_message_node(const XmlMessageNode* message_node)
{
    _ASSERT(message_node && node_name_equal(*message_node, "message"));
    return static_cast<const XmlMessageNode*>(find_sibling(*message_node, "message"));
}

XmlStreamDoc::ConstMessageNodeRange XmlStreamDoc::get_messages() const
{
    auto root = get_root(*this);
    if (!root)
    {
        throw std::logic_error("Invalid stream document");
    }

    auto begin = ConstMessageNodeIterator(static_cast<const XmlMessageNode*>(find_child(*root, "message")));
    return ConstMessageNodeRange(begin, ConstMessageNodeIterator());
}

std::unique_ptr<XmlStreamDoc> parse_stream(const std::string& stream_xml)
{
    return stl_tools::static_pointer_cast<XmlStreamDoc>(xml_tools::parse(stream_xml));
}

}}// namespace TR { namespace External {