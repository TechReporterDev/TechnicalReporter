#pragma once
#include "TRCore\SourceTypes.h"
#include "TRCore\Syslog.h"
#include "stddecl.h"

namespace TR { namespace External {

class SyslogInputTransport: public SyslogTransport
{
public:
    using SourceIdentifier = std::function<std::string(const XmlDoc& source_config)>;

    SyslogInputTransport(StreamTypeRef stream_type_ref, std::unique_ptr<XsltStylesheet> source_identifier, std::unique_ptr<xml_tools::XmlGrammarDoc> grammar_doc, std::unique_ptr<XsltStylesheet> format_stylesheet);

    virtual std::string         get_source_addr(const XmlDoc& source_config) const override;
    virtual SyslogParser        prepare_parser() const override;

private:    
    std::unique_ptr<XsltStylesheet> m_source_identifier;
    std::shared_ptr<XmlGrammarDoc> m_grammar_doc;
    std::shared_ptr<XsltStylesheet> m_format_stylesheet;
};

}} //namespace TR {namespace External {