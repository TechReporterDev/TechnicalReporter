#include "stdafx.h"
#include "SuitsFeature.h"
#include "CoreImpl.h"
namespace TR { namespace Core {
using namespace stl_tools;
using namespace XML;

const SourceTypeUUID SuitRootSourceType::SOURCE_TYPE_UUID = SourceTypeUUID(gen_uuid(L"94508C9F-2011-4B23-9646-6188F06D5F71"));
const auto SUIT_ROOT_CONFIG_DEF = 
    "<?xml version=\"1.0\"?>"
    "<settings style=\"COLLECTION\" caption=\"Suits settings\" description=\"Suits settings description\">"
    "</settings>";

SuitRootSourceType::SuitRootSourceType():
    SourceType(SOURCE_TYPE_UUID, L"Suits", parse_properties_def(SUIT_ROOT_CONFIG_DEF), null_uuid())
{
}

std::vector<ReportTypeRef> SuitRootSourceType::get_downloads() const
{
    return {};
}

SourceType::DownloadFunctor SuitRootSourceType::prepare_download(ReportTypeRef report_type_ref, SourceRef source_ref) const
{   
    _ASSERT(false);
    throw std::logic_error("Invalid report type");
}

std::vector<StreamTypeRef> SuitRootSourceType::get_streams() const
{
    return {};
}

const StreamTransport& SuitRootSourceType::get_transport(StreamTypeRef stream_type_ref) const
{
    _ASSERT(false);
    throw std::logic_error("Invalid stream type");
}

std::vector<ActionRef> SuitRootSourceType::get_actions() const
{
    return {};
}

std::vector<ReportTypeRef> SuitRootSourceType::get_reloads(ActionRef action_ref) const
{
    _ASSERT(false);
    return {};
}

SourceType::ActionFunctor SuitRootSourceType::prepare_action(ActionRef action_ref, SourceRef source_ref) const
{
    _ASSERT(false);
    throw std::logic_error("Invalid action type");
}

const SourceTypeUUID SuitSourceType::SOURCE_TYPE_UUID = SourceTypeUUID(gen_uuid(L"14DBA14A-532C-4202-95CE-28241D61870B"));
const auto SUIT_CONFIG_DEF = 
    "<?xml version=\"1.0\"?>"
    "<settings style=\"COLLECTION\" caption=\"Suit settings\" description=\"Suit settings description\">"
    "</settings>";

SuitSourceType::SuitSourceType():
    SourceType(SOURCE_TYPE_UUID, L"Suit", parse_properties_def(SUIT_CONFIG_DEF), null_uuid())
{
}

std::vector<ReportTypeRef> SuitSourceType::get_downloads() const
{
    return {};
}

SourceType::DownloadFunctor SuitSourceType::prepare_download(ReportTypeRef report_type_ref, SourceRef source_ref) const
{   
    _ASSERT(false);
    throw std::logic_error("Invalid report type");
}

std::vector<StreamTypeRef> SuitSourceType::get_streams() const
{
    return {};
}

const StreamTransport& SuitSourceType::get_transport(StreamTypeRef stream_type_ref) const
{
    _ASSERT(false);
    throw std::logic_error("Invalid stream type");
}

std::vector<ActionRef> SuitSourceType::get_actions() const
{
    return {};
}

std::vector<ReportTypeRef> SuitSourceType::get_reloads(ActionRef action_ref) const
{
    _ASSERT(false);
    return {};
}

SourceType::ActionFunctor SuitSourceType::prepare_action(ActionRef action_ref, SourceRef source_ref) const
{
    _ASSERT(false);
    throw std::logic_error("Invalid action type");
}

auto suits_feature_uuid = gen_uuid(L"C628DF33-5179-41E8-AE69-847654FFAB5C");
SuitsFeature::SuitsFeature():
    ComposedFeature(suits_feature_uuid, L"Suits Feature")
{
    add_source_type_factory(std::make_unique<CustomSourceTypeFactory>(SuitRootSourceType::SOURCE_TYPE_UUID, L"Suit root", [](const CoreImpl& core_impl){
        return std::make_unique<SuitRootSourceType>();  
    }));

    add_source_type_factory(std::make_unique<CustomSourceTypeFactory>(SuitSourceType::SOURCE_TYPE_UUID, L"Suit", [](const CoreImpl& core_impl){
        return std::make_unique<SuitSourceType>();
    }));
}

UUID SuitsFeature::get_feature_uuid()
{
    return suits_feature_uuid;
}

void SuitsFeature::install(CoreImpl& core_impl, bool restore, Transaction& t)
{
    ComposedFeature::install(core_impl, restore, t);

    if (!restore)
    {
        auto& suits_source_type = core_impl.m_core_domain.m_basis.m_source_types.get_source_type(SuitRootSourceType::SOURCE_TYPE_UUID);
        auto suits_root = core_impl.m_core_domain.m_registry.m_sources.add_source({
            L"Suits",
            boost::none,
            core_impl.m_core_domain.m_registry.m_roles.get_default_role(),
            suits_source_type,
            suits_source_type.get_default_config()
        }, t);

        core_impl.m_security.set_access_rights(1, suits_root, Access::FULL_ACCESS, t);
    }   
}

void SuitsFeature::uninstall(CoreImpl& core_impl, Transaction& t)
{
    for (auto& source : core_impl.m_core_domain.m_registry.m_sources.get_sources())
    {
        if (source.get_source_type().get_uuid() == SuitRootSourceType::SOURCE_TYPE_UUID)
        {
            core_impl.m_core_domain.m_registry.m_sources.remove_source(source.get_ref().get_key(), t);
        }
    }

    ComposedFeature::uninstall(core_impl, t);
}

}} //namespace TR { namespace Core {