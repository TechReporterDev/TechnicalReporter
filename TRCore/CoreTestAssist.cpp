#include "stdafx.h"
#include "CoreTestAssist.h"
#include "Core.h"
#include <set>

std::unique_ptr<Executive> get_executive()
{
    return std::make_unique<Executive>(
        std::make_shared<DirectExecutor>(),
        std::make_unique<DirectExecutor>(),
        std::make_unique<DirectExecutor>()
    );
}

std::unique_ptr<CoreImpl> clean_and_install()
{
    TR::Core::uninstall(L"root", L"root", L"localhost", L"tech_reporter_test");
    TR::Core::install(L"root", L"root", L"localhost", L"tech_reporter_test");
    auto core_impl = std::make_unique<CoreImpl>(connect_database(L"root", L"root", L"localhost", L"tech_reporter_test"));
    
    Transaction t(*core_impl->m_db);
    core_impl->run(get_executive(), t);
    t.commit();

    return core_impl;
}

const SourceTypeUUID fake_source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
const ReportTypeUUID fake_report_type_uuid = ReportTypeUUID(stl_tools::gen_uuid());
const std::string empty_config_def =
    "<?xml version=\"1.0\"?>"
    "<settings class=\"COLLECTION\" caption=\"Settings\" description=\"Settings Description\">"
    "</settings>";

const std::string empty_params_def =
    "<?xml version=\"1.0\"?>"
    "<params class=\"COLLECTION\" caption=\"Params\" description=\"Params Description\">"
    "</params>";

SourceTypeMockup::SourceTypeMockup(SourceTypeUUID source_type_uuid, std::string config_def):
    SourceType(source_type_uuid, L"FakeSource", XML::parse_properties_def(config_def), stl_tools::null_uuid())
{
}

void SourceTypeMockup::push_download(ReportTypeRef report_type_ref, DownloadFunctor download_functor)
{
    m_downloads[report_type_ref].push_back(std::move(download_functor));
}

void SourceTypeMockup::push_action(ActionRef action_ref, ActionFunctor action_functor)
{
    m_actions[action_ref].push_back(action_functor);
}

void SourceTypeMockup::set_action_reloads(ActionRef action_ref, std::vector<ReportTypeRef> reload_refs)
{
    m_reloads[action_ref] = reload_refs;
}

std::vector<ReportTypeRef> SourceTypeMockup::get_downloads() const
{
    std::vector<ReportTypeRef> downloads;
    for (auto& download_ref : m_downloads | boost::adaptors::map_keys)
    {
        downloads.push_back(download_ref);
    }
    return downloads;
}

std::vector<ActionRef> SourceTypeMockup::get_actions() const
{
    std::vector<ActionRef> actions;
    for (auto& action_ref : m_actions | boost::adaptors::map_keys)
    {
        actions.push_back(action_ref);
    }
    return actions;
}

std::vector<ReportTypeRef> SourceTypeMockup::get_reloads(ActionRef action_ref) const
{
    return m_reloads[action_ref];
}

SourceType::DownloadFunctor SourceTypeMockup::prepare_download(ReportTypeRef report_type_ref, SourceRef) const
{
    auto& downloads = m_downloads[report_type_ref];
    _ASSERT(!downloads.empty());

    auto download_functor = std::move(downloads.front());
    downloads.pop_front();
    return download_functor;
}

std::vector<StreamTypeRef> SourceTypeMockup::get_streams() const
{
    return{};
}

const StreamTransport& SourceTypeMockup::get_transport(StreamTypeRef stream_type_ref) const
{
    _ASSERT(false);
    throw std::logic_error("Invalid stream type");
}

SourceType::ActionFunctor SourceTypeMockup::prepare_action(ActionRef action_ref, SourceRef source_ref) const
{
    auto& actions = m_actions[action_ref];
    _ASSERT(!actions.empty());

    auto action_functor = std::move(actions.front());
    actions.pop_front();
    return action_functor;
}

void push_download(SourceTypeMockup& source_type_mockup, ReportTypeRef report_type_ref, std::string text)
{
    auto& text_report_type = dynamic_cast<const TR::Core::TextReportType&>(*report_type_ref);
    auto make_content = text_report_type.prepare_make_content_from_string();    
    source_type_mockup.push_download(report_type_ref, [text, make_content](const XmlDoc&) mutable {
        return make_content(std::move(text));
    });
}

void push_action(SourceTypeMockup& source_type_mockup, ActionRef action_ref, std::string output)
{
    auto make_content = (*action_ref).prepare_make_content();
    Blob blob(output.begin(), output.end());
    source_type_mockup.push_action(action_ref, [blob, make_content](const XmlDoc& params, const XmlDoc&){
        return make_content(std::move(blob), params);
    });
}

void push_action(SourceTypeMockup& source_type_mockup, ActionRef action_ref)
{
    source_type_mockup.push_action(action_ref, [](const XmlDoc&, const XmlDoc&){
        return nullptr;
    });
}

ActionMockup::ActionMockup(ActionUUID uuid, std::string params_def, OutputTypeFactory output_type_factory):
    Action(uuid, L"ActionMockup", XML::parse_properties_def(params_def)),
    m_output_type_factory(std::move(output_type_factory))
{
}

Action::MakeContent ActionMockup::prepare_make_content() const
{
    if (!m_output_type_factory)
    {
        return nullptr;
    }

    auto output_type_factory = m_output_type_factory;
    return [output_type_factory](Blob blob, const XmlDoc& params){
        auto output_type = output_type_factory(ReportTypeUUID(stl_tools::gen_uuid()), L"", params);
        return output_type->make_content(blob);
    };
}

std::unique_ptr<ReportType> ActionMockup::make_report_type(UUID uuid, std::wstring name, const XmlDoc& params) const
{
    if (!m_output_type_factory)
    {
        return nullptr;
    }

    return m_output_type_factory(uuid, std::move(m_name), params);
}

ActionMockup::OutputTypeFactory text_output_type_factory()
{
    return [](UUID uuid, std::wstring name, const XmlDoc& params){
        return std::make_unique<PlainTextReportType>(uuid, std::move(name));
    };
}

ActionDelegateMockup::ActionDelegateMockup(std::vector<ReportTypeRef> input_refs, std::vector<ActionRef> host_action_refs, ActionRef guest_action_ref):
    ActionDelegate(std::move(input_refs), std::move(host_action_refs), guest_action_ref)
{
}
    
void ActionDelegateMockup::push_action(ActionFunctor action_functor)
{
    m_actions.push_back(std::move(action_functor));
}

ActionDelegate::ActionFunctor ActionDelegateMockup::prepare_action(SourceRef source_ref) const
{
    _ASSERT(!m_actions.empty());

    auto action_functor = std::move(m_actions.front());
    m_actions.pop_front();
    return action_functor;
}

void push_action(ActionDelegateMockup& action_delegate_mockup, std::string output)
{
    auto& action = *action_delegate_mockup.get_guest_action_ref();
    auto make_content = action.prepare_make_content();
    Blob blob(output.begin(), output.end());
    action_delegate_mockup.push_action([blob, make_content](const XmlDoc& params, std::vector<std::shared_ptr<Content>>, std::vector<ActionDelegate::HostActionFunctor>){
        return make_content(std::move(blob), params);
    });
}

void push_action(ActionDelegateMockup& action_delegate_mockup, boost::none_t none)
{
    action_delegate_mockup.push_action([](const XmlDoc&, std::vector<std::shared_ptr<Content>>, std::vector<ActionDelegate::HostActionFunctor>){
        return nullptr;
    });
}

ActionShortcutMockup::ActionShortcutMockup(UUID shortcut_uuid, ActionRef host_action_ref, boost::optional<ReportTypeRef> output_ref):
    ActionShortcut(shortcut_uuid, L"ActionShortcutMockup", host_action_ref, output_ref)
{
}

void ActionShortcutMockup::push_params(std::unique_ptr<XmlDoc> params_doc)
{
    m_params.push(std::move(params_doc));
}
        
std::unique_ptr<XmlDoc> ActionShortcutMockup::get_action_params(SourceRef source) const
{
    _ASSERT(!m_params.empty());

    auto params = std::move(m_params.front());
    m_params.pop();
    return params;
}

SelectionMockup::SelectionMockup(SelectionDirection direction, Functor f, UUID uuid):
    Selection(uuid),
    m_plan(
        std::vector<SelectionInput>(direction.m_input_refs.begin(), direction.m_input_refs.end()),
        direction.m_output_ref),
    m_f(f)
{
}

SelectionMockup::SelectionMockup(SelectionPlan plan, Functor f, UUID uuid):
    Selection(uuid),
    m_plan(plan),
    m_f(f)
{
}

SelectionPlan SelectionMockup::get_plan() const
{
    return m_plan;
}

Selection::Functor SelectionMockup::prepare_functor(SourceRef) const
{
    return m_f;
}

GroupingMockup::GroupingMockup(GroupingDirection direction, Functor f, UUID uuid):
    Grouping(uuid),
    m_direction(std::move(direction)),
    m_f(f)
{
}

GroupingMockup::GroupingMockup(ReportTypeRef input_ref, ReportTypeRef output_ref, Functor f, UUID uuid):
    GroupingMockup({input_ref, output_ref}, std::move(f), uuid)
{
}

void GroupingMockup::add_functor(SourceRef ref, const std::vector<SourceRef>& refs, Functor f)
{
    m_fs[std::make_pair(ref, refs)] = f;
}

GroupingDirection GroupingMockup::get_direction() const
{
    return m_direction;
}

Grouping::Functor GroupingMockup::prepare_functor(SourceRef ref, const std::vector<SourceRef>& refs) const
{
    auto position = m_fs.find(std::make_pair(ref, refs));
    if (position != m_fs.end())
    {
        return position->second;
    }

    if (m_f)
    {
        return m_f;
    }

    return [](const std::vector<std::shared_ptr<Content>>&) -> std::unique_ptr<Content>
    {
        return nullptr;
    };
}