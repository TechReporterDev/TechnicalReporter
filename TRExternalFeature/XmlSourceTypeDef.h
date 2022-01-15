#pragma once
#include "TRCore\BasisTrivialTypes.h"
#include "stddecl.h"
namespace TR { namespace External {

struct XmlSourceTypeDef: XmlNode
{
    struct XmlDownloadNode: XmlNode
    {
        ReportTypeUUID                      get_report_type_uuid() const;
        const XmlNode&                      get_command_node() const;
        bool                                must_discard_empty() const;
    };

    struct XmlStreamNode: XmlNode
    {
        enum Protocol { SYSLOG };
        StreamTypeUUID                      get_stream_type_uuid() const;
        Protocol                            get_protocol() const;
        const XmlNode&                      get_address_node() const;
        const XmlNode&                      get_parser_node() const;
        const XmlNode&                      get_format_node() const;
    };

    struct XmlActionNode: XmlNode
    {
        ActionUUID                          get_action_uuid() const;
        const XmlNode&                      get_command_node() const;
        std::vector<ReportTypeUUID>         get_reload_uuids() const;
    };

    static const XmlDownloadNode* next_download_node(const XmlDownloadNode* download_node);
    using ConstDownloadNodeIterator = xml_tools::node_iterator<const XmlDownloadNode, next_download_node>;
    using ConstDownloadNodeRange = boost::iterator_range<ConstDownloadNodeIterator>;

    static const XmlStreamNode* next_stream_node(const XmlStreamNode* stream_node);
    using ConstStreamNodeIterator = xml_tools::node_iterator<const XmlStreamNode, next_stream_node>;
    using ConstStreamNodeRange = boost::iterator_range<ConstStreamNodeIterator>;

    static const XmlActionNode* next_action_node(const XmlActionNode* action_node);
    using ConstActionNodeIterator = xml_tools::node_iterator<const XmlActionNode, next_action_node>;
    using ConstActionNodeRange = boost::iterator_range<ConstActionNodeIterator>;

    SourceTypeUUID                  get_uuid() const;
    std::wstring                    get_name() const;
    std::string                     get_settings() const;
    ConstDownloadNodeRange          get_downloads() const;
    ConstStreamNodeRange            get_streams() const;
    ConstActionNodeRange            get_actions() const;
};

}}// namespace TR { namespace External {