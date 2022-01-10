#pragma once
#include "stddecl.h"
#include "TRCore\BasisTrivialTypes.h"
namespace TR { namespace External {

struct XmlMessageNode: XmlNode
{
    enum class Severety {INFO = 0, WARNING, FAILURE};

    Severety                    get_severity() const;
    boost::optional<time_t>     get_time() const;
    std::wstring                get_event() const;
    const XmlNode&              get_body() const;
};

struct XmlStreamDoc: XmlDoc
{
    static const XmlMessageNode* next_message_node(const XmlMessageNode* message_node);
    using ConstMessageNodeIterator = xml_tools::node_iterator<const XmlMessageNode, next_message_node>;
    using ConstMessageNodeRange = boost::iterator_range<ConstMessageNodeIterator>;

    ConstMessageNodeRange get_messages() const;
};

std::unique_ptr<XmlStreamDoc> parse_stream(const std::string& stream_xml);

}}// namespace TR { namespace External {