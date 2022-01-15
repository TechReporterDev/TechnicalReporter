#include "stdafx.h"
#include "CommandSourceType.h"
#include "TRCore\RegistryRefs.h"
#include "TRCore\ReportTypes.h"
#include "TRCore\StreamTypes.h"
#include "TRCore\Actions.h"
#include "TRCore\Content.h"
#include <boost\algorithm\string.hpp>
#include <boost\regex.hpp>
#include <boost\iterator_adaptors.hpp>
#include "W32Cmd.h"
namespace TR { namespace External {

namespace { 
void compress_spaces(std::wstring& command)
{
    std::wstring::iterator position = command.begin();
    for (auto& alpha : command)
    {
        if (std::isspace(alpha))
        {
            if (position == command.begin() || *(position - 1) == L' ')
            {
                continue;
            }
            alpha = L' ';
        }
        *(position++) = alpha;
    }
    if (position != command.begin() && *(position - 1) == L' ')
    {
        --position;
    }
    command.resize(position - command.begin());
}
} //namespace {

CommandSourceType::CommandSourceType(SourceTypeUUID uuid, std::wstring name, std::unique_ptr<XML::XmlPropertiesDef> config_def, UUID family_uuid):
    SourceType(uuid, std::move(name), std::move(config_def), family_uuid)
{
}

void CommandSourceType::add_download_command(ReportTypeRef download_ref, MakeCommandFunctor make_command_functor)
{
    m_download_commands.insert({ download_ref, DownloadCommand{std::move(make_command_functor)} });
}

void CommandSourceType::add_download_command(ReportTypeRef download_ref, DownloadCommand download_command)
{
    m_download_commands.insert({ download_ref, std::move(download_command) });
}

void CommandSourceType::add_stream_transport(std::unique_ptr<StreamTransport> stream_transport)
{
    auto stream_type_ref = stream_transport->get_stream_type();
    m_stream_transports.insert({stream_type_ref, std::move(stream_transport)});
}

void CommandSourceType::add_action_command(ActionRef action_ref, MakeCommandFunctor make_command_functor, std::vector<ReportTypeRef> reloads)
{
    m_action_commands.insert({action_ref, {std::move(make_command_functor), reloads}});
}

std::vector<ReportTypeRef> CommandSourceType::get_downloads() const
{
    std::vector<ReportTypeRef> downloads;
    boost::copy(m_download_commands | boost::adaptors::map_keys, std::back_inserter(downloads));
    return downloads;
}

Blob::const_iterator skip_back(const Blob& blob, const std::string& what)
{
    auto blob_rbegin = blob.rbegin();
    auto blob_rend = blob.rend();
    
    for (auto ch : what | boost::adaptors::reversed)
    {
        if (blob_rbegin == blob_rend)
        {
            break;
        }

        if (*blob_rbegin != ch)
        {
            break;
        }

        ++blob_rbegin;
    }

    return blob_rbegin.base();
}

bool is_empty(const Blob& blob)
{
    return skip_back(blob, "\r\n") == blob.end();    
}

SourceType::DownloadFunctor CommandSourceType::prepare_download(ReportTypeRef report_type_ref, SourceRef) const
{
    auto found = m_download_commands.find(report_type_ref);
    if (found == m_download_commands.end())
    {
        throw std::logic_error("Invalid report type for download");
    }

    auto download_command = found->second;
    auto make_content = (*report_type_ref).prepare_make_content_from_blob();
    return [download_command, make_content](const XmlDoc& source_config) -> std::unique_ptr<Content> {
        Blob input;
        boost::copy(xml_tools::as_string(source_config), std::back_inserter(input));
        auto command_line = download_command.m_make_command_functor(source_config);
        auto blob = exec_command(command_line, input);
        if (download_command.m_discard_empty && is_empty(blob))
        {
            return nullptr;
        }

        return make_content(std::move(blob));
    };
}

std::vector<StreamTypeRef> CommandSourceType::get_streams() const
{
    std::vector<StreamTypeRef> streams;
    boost::copy(m_stream_transports | boost::adaptors::map_keys, std::back_inserter(streams));
    return streams;
}

const StreamTransport& CommandSourceType::get_transport(StreamTypeRef stream_type_ref) const
{
    auto found = m_stream_transports.find(stream_type_ref);
    if (found == m_stream_transports.end())
    {
        throw std::logic_error("Invalid report type for download");
    }
    return *found->second;
}

std::vector<ActionRef> CommandSourceType::get_actions() const
{
    std::vector<ActionRef> action_refs;
    boost::copy(m_action_commands | boost::adaptors::map_keys, std::back_inserter(action_refs));
    return action_refs;
}

std::vector<ReportTypeRef> CommandSourceType::get_reloads(ActionRef action_ref) const
{
    auto position = m_action_commands.find(action_ref);
    if (position == m_action_commands.end())
    {
        throw std::exception("Action not found");
    }

    return position->second.second;
}

SourceType::ActionFunctor CommandSourceType::prepare_action(ActionRef action_ref, SourceRef source_ref) const
{
    auto position = m_action_commands.find(action_ref);
    if (position == m_action_commands.end())
    {
        throw std::logic_error("Unknown action found");
    }
    auto make_command_functor = position->second.first;
    auto make_content = (*action_ref).prepare_make_content();

    return [make_command_functor, make_content](const XmlDoc& params, const XmlDoc& source_config){
        auto input_doc = xml_tools::compose(clone_doc(params), clone_doc(source_config), "input");
        auto command_line = make_command_functor(*input_doc);
    
        Blob input;
        boost::copy(xml_tools::as_string(*input_doc), std::back_inserter(input));       
        Blob output = exec_command(command_line, input);

        if (!make_content)
        {
            return std::unique_ptr<Content>();
        }
        return make_content(std::move(output), params);
    };
}

CommandSourceType::MakeCommandFunctor xslt_make_command(std::unique_ptr<XsltStylesheet> stylesheet)
{
    return [stylesheet = std::shared_ptr<XsltStylesheet>(std::move(stylesheet))](const XmlDoc& params_doc){
        auto command_doc = xml_tools::xslt_apply_stylesheet(*stylesheet, params_doc);
        if (!command_doc)
        {
            throw std::logic_error("Invalid command for download");
        }
        auto command_line = get_node_wtext((XmlNode&)(*command_doc));
        compress_spaces(command_line);
        return command_line;
    };
}

}} //namespace TR { namespace External {