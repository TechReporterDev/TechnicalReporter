#pragma once
#include "TRCore\SourceTypes.h"
#include "stddecl.h"

namespace TR {  namespace External {

struct DownloadCommand
{
    using MakeCommandFunctor = std::function<std::wstring(const XmlDoc& input)>;

    MakeCommandFunctor m_make_command_functor;
    bool m_discard_empty = false;
};

class CommandSourceType: public SourceType
{
public:
    using MakeCommandFunctor = std::function<std::wstring(const XmlDoc& input)>;

    CommandSourceType(SourceTypeUUID uuid, std::wstring name, std::unique_ptr<XML::XmlPropertiesDef> config_def, UUID family_uuid);
    void                                    add_download_command(ReportTypeRef download_ref, MakeCommandFunctor make_command_functor);
    void                                    add_download_command(ReportTypeRef download_ref, DownloadCommand download_command);

    void                                    add_stream_transport(std::unique_ptr<StreamTransport> stream_transport);
    void                                    add_action_command(ActionRef action_ref, MakeCommandFunctor make_command_functor, std::vector<ReportTypeRef> reloads);

    // override
    virtual std::vector<ReportTypeRef>      get_downloads() const override;
    virtual DownloadFunctor                 prepare_download(ReportTypeRef report_type_ref, SourceRef source_ref) const override;

    virtual std::vector<StreamTypeRef>      get_streams() const override;
    virtual const StreamTransport&          get_transport(StreamTypeRef stream_type_ref) const override;

    virtual std::vector<ActionRef>          get_actions() const override;
    virtual std::vector<ReportTypeRef>      get_reloads(ActionRef action_ref) const override;
    virtual ActionFunctor                   prepare_action(ActionRef action_ref, SourceRef source_ref) const override;
    
private:
    std::map<ReportTypeRef, DownloadCommand> m_download_commands;
    std::map<StreamTypeRef, std::unique_ptr<StreamTransport>> m_stream_transports;
    std::map<ActionRef, std::pair<MakeCommandFunctor, std::vector<ReportTypeRef>>> m_action_commands;
};

CommandSourceType::MakeCommandFunctor xslt_make_command(std::unique_ptr<XsltStylesheet> xslt_stylesheet);

}} //namespace TR { namespace External {
