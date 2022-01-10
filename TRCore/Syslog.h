#pragma once
#include "AppDecl.h"
#include "AsioSyslog.h"
#include "SourceTypes.h"
#include "Sources.h"
#include "Reservoir.h"
namespace TR { namespace Core {

class SyslogTransport: public StreamTransport
{
public: 
    using SyslogParser = std::function<std::vector<StreamMessage>(const std::string& text)>;

    SyslogTransport(StreamTypeRef stream_type_ref);
    virtual std::string     get_source_addr(const XmlDoc& source_config) const = 0;
    virtual SyslogParser    prepare_parser() const = 0;
};

class Syslog
{
public:
    static const Priority SYSLOG_PRIORITY = 2000;

    Syslog(Database& db, CoreDomain& core_domain);  
    ~Syslog();
    void run(Executive* executive, Transaction& t);

private:
    class ParseSyslogManager;
    friend class ParseSyslogJob;    

    void on_add_source(SourceRef source_ref, Transaction& t);
    void on_update_source(SourceRef source_ref, Transaction& t);
    void on_remove_sources(const std::vector<SourceKey>& removed_sources, Transaction& t);
    void dispatch_message(const std::string& address, const std::string& message);
    
    Database* m_db;
    StreamTypes* m_stream_types;
    Reservoir* m_reservoir;
    Sources* m_sources;
    Executive* m_executive;
    std::unique_ptr<AsioSyslog> m_asio_syslog;
    std::unique_ptr<ParseSyslogManager> m_parse_syslog_manager;
};

}} //namespace TR { namespace Core {