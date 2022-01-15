#pragma once
#include "CoreImpl.h"
#include "stddecl.h"
using namespace TR;
using namespace TR::Core;

std::unique_ptr<CoreImpl> clean_and_install();

extern const SourceTypeUUID fake_source_type_uuid;
extern const ReportTypeUUID fake_report_type_uuid;
extern const std::string empty_config_def;
extern const std::string empty_params_def;

class SourceTypeMockup: public SourceType
{
public:
    SourceTypeMockup(SourceTypeUUID source_type_uuid = fake_source_type_uuid, std::string config_def = empty_config_def);
    
    void push_download(ReportTypeRef report_type_ref, DownloadFunctor download_functor);
    void push_action(ActionRef action_ref, ActionFunctor action_functor);
    void set_action_reloads(ActionRef action_ref, std::vector<ReportTypeRef> reload_refs);

    // SourceType override
    virtual std::vector<ReportTypeRef>  get_downloads() const override;
    virtual DownloadFunctor             prepare_download(ReportTypeRef report_type_ref, SourceRef source_ref) const override;

    virtual std::vector<StreamTypeRef>  get_streams() const override;
    virtual const StreamTransport&      get_transport(StreamTypeRef stream_type_ref) const override;
    
    virtual std::vector<ActionRef>      get_actions() const override;
    virtual std::vector<ReportTypeRef>  get_reloads(ActionRef action_ref) const override;
    virtual ActionFunctor               prepare_action(ActionRef action_ref, SourceRef source_ref) const override;
    
private:
    mutable std::map<ReportTypeRef, std::deque<DownloadFunctor>> m_downloads;
    mutable std::map<ActionRef, std::deque<ActionFunctor>> m_actions;
    mutable std::map<ActionRef, std::vector<ReportTypeRef>> m_reloads;
};

void push_download(SourceTypeMockup& source_type_mockup, ReportTypeRef report_type_ref, std::string text);
void push_action(SourceTypeMockup& source_type_mockup, ActionRef action_ref, std::string output);
void push_action(SourceTypeMockup& source_type_mockup, ActionRef action_ref);

class ActionMockup: public Action
{
public:
    using OutputTypeFactory = std::function<std::unique_ptr<ReportType>(UUID uuid, std::wstring name, const XmlDoc& params)>;
    ActionMockup(ActionUUID uuid, std::string params_def = empty_params_def, OutputTypeFactory output_type_factory = nullptr);

    // Action override 
    virtual MakeContent                     prepare_make_content() const;
    virtual std::unique_ptr<ReportType>     make_report_type(UUID uuid, std::wstring name, const XmlDoc& params) const;

private:
    OutputTypeFactory m_output_type_factory;
};

ActionMockup::OutputTypeFactory text_output_type_factory();

class ActionDelegateMockup: public ActionDelegate
{
public:
    ActionDelegateMockup(std::vector<ReportTypeRef> input_refs, std::vector<ActionRef> host_action_refs, ActionRef guest_action_ref);
    void push_action(ActionFunctor action_functor);
        
    // ActionDelegate override
    virtual ActionFunctor prepare_action(SourceRef source_ref) const override;

private:
    mutable std::deque<ActionFunctor> m_actions;
};
    
void push_action(ActionDelegateMockup& action_delegate_mockup, std::string output);
void push_action(ActionDelegateMockup& action_delegate_mockup, boost::none_t none);

class ActionShortcutMockup: public ActionShortcut
{
public:
    ActionShortcutMockup(UUID shortcut_uuid, ActionRef host_action_ref, boost::optional<ReportTypeRef> output_ref = boost::none);
    void push_params(std::unique_ptr<XmlDoc> params_doc);
        
    // ActionShortcut override
    virtual std::unique_ptr<XmlDoc> get_action_params(SourceRef source) const override;

private:
    mutable std::queue<std::unique_ptr<XmlDoc>> m_params;
};

class SelectionMockup: public Selection
{
public:
    SelectionMockup(SelectionDirection direction, Functor f = nullptr, UUID uuid = stl_tools::gen_uuid());
    SelectionMockup(SelectionPlan plan, Functor f = nullptr, UUID uuid = stl_tools::gen_uuid());

    // Selection override
    virtual SelectionPlan      get_plan() const override;
    virtual Functor            prepare_functor(SourceRef) const override;

protected:
    SelectionPlan m_plan;
    Functor m_f;
};

class GroupingMockup: public Grouping
{
public:
    GroupingMockup(GroupingDirection direction, Functor f = nullptr, UUID uuid = stl_tools::gen_uuid());
    GroupingMockup(ReportTypeRef input_ref, ReportTypeRef output_ref, Functor f = nullptr, UUID uuid = stl_tools::gen_uuid());
    void add_functor(SourceRef source_ref, const std::vector<SourceRef>& refs, Functor f);
    
    // Grouping override
    virtual GroupingDirection   get_direction() const override;
    virtual Functor             prepare_functor(SourceRef source_ref, const std::vector<SourceRef>&) const override;

protected:
    GroupingDirection m_direction;
    Functor m_f;
    std::map<std::pair<SourceRef, std::vector<SourceRef>>, Functor> m_fs;
};

#define CLEAN_AND_INSTALL() \
    auto core_impl = clean_and_install();\
    auto& db = *core_impl->m_db;\
    auto& core_domain = core_impl->m_core_domain;\
    auto& report_types = core_domain.m_basis.m_report_types;\
    auto& stream_types = core_domain.m_basis.m_stream_types;\
    auto& source_types = core_domain.m_basis.m_source_types;\
    auto& selections = core_domain.m_basis.m_transformations.m_selections;\
    auto& groupings = core_domain.m_basis.m_transformations.m_groupings;\
    auto& actions = core_domain.m_basis.m_actions;\
    auto& action_delegates = core_domain.m_basis.m_action_delegates;\
    auto& action_shortcuts = core_domain.m_basis.m_action_shortcuts;\
    auto& source_type_activities = core_domain.m_basis.m_source_type_activities;\
    auto& sources = core_domain.m_registry.m_sources;\
    auto& roles = core_domain.m_registry.m_roles;\
    auto& resource_policies = core_domain.m_registry.m_resource_policies;\
    auto& source_resources = core_domain.m_registry.m_source_resources;\
    auto& collector = core_domain.m_registry.m_collector;\
    auto& group_source_type = source_types.get_source_type(GroupSourceType::s_uuid);\
    auto default_role = roles.get_default_role();\
    auto root = sources.get_root();