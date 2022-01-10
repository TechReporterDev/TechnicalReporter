#include "stdafx.h"
#include "SyslogInputTransport.h"
#include "XmlStreamDoc.h"
#include "W32Cmd.h"
using namespace xml_tools;
namespace TR { namespace External {

SyslogInputTransport::SyslogInputTransport(StreamTypeRef stream_type_ref, std::unique_ptr<XsltStylesheet> source_identifier, std::unique_ptr<xml_tools::XmlGrammarDoc> grammar_doc, std::unique_ptr<XsltStylesheet> format_stylesheet):
    SyslogTransport(stream_type_ref), 
    m_source_identifier(std::move(source_identifier)),
    m_grammar_doc(std::move(grammar_doc)),
    m_format_stylesheet(std::move(format_stylesheet))
{
}

std::string SyslogInputTransport::get_source_addr(const XmlDoc& source_config) const
{
    auto identity_doc = xml_tools::xslt_apply_stylesheet(*m_source_identifier, source_config);
    if (!identity_doc)
    {
        throw std::logic_error("Invalid syslog identity");
    }
    return get_node_text((XmlNode&)(*identity_doc));    
}

SyslogTransport::SyslogParser SyslogInputTransport::prepare_parser() const
{
    auto message_def = (*m_stream_type_ref).get_message_def();

    auto grammar_doc = m_grammar_doc;
    auto format_stylesheet = m_format_stylesheet;
    
    return [message_def, grammar_doc, format_stylesheet](const std::string& syslog_input){
        xml_tools::regex_parser parser(clone_doc(*grammar_doc));
        auto xml_input = parser.parse(syslog_input);
        auto stream_doc = stl_tools::static_pointer_cast<XmlStreamDoc>(xml_tools::xslt_apply_stylesheet(*format_stylesheet, *xml_input));

        std::vector<StreamMessage> messages;
        for (auto& message_node : stream_doc->get_messages())
        {
            auto body_doc = create_doc();
            set_root(*body_doc, clone_node(message_node.get_body()));
            XML::bind_definition(*body_doc, *message_def);

            messages.push_back({
                StreamMessage::Severety(message_node.get_severity()),
                message_node.get_event(),
                message_node.get_time().get_value_or(time(nullptr)),
                std::make_shared<GeneralRegularContent>(stl_tools::static_pointer_cast<XmlRegularDoc>(std::move(body_doc)), message_def)
            });
        }
        
        return messages;
    };
}

}} //namespace TR {namespace External {