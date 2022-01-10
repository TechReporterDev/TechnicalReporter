#include "stdafx.h"
#include "ResourcePolicies.h"
#include "Basis.h"
#include "Sources.h"
#include "Roles.h"
#include "Transaction.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
#include "ResourcePolicyDataset.h"
namespace TR { namespace Core {

ResourcePolicies::ResourcePolicies(Database& db, Basis& basis):
    m_db(db),
    m_report_types(basis.m_report_types),
    m_stream_types(basis.m_stream_types),
    m_sources(nullptr),
    m_roles(nullptr),
    m_resource_policy_dataset(std::make_unique<ResourcePolicyDataset>(m_db))
{
    m_report_types.connect_remove_report_type([this](ReportTypeUUID report_type_uuid, Transaction& t){
        on_remove_report_type(report_type_uuid, t);
    }, RESOURCE_POLICIES_PRIORITY);

    m_stream_types.connect_remove_stream_type([this](StreamTypeUUID stream_type_uuid, Transaction& t){
        on_remove_stream_type(stream_type_uuid, t);
    }, RESOURCE_POLICIES_PRIORITY);
}

ResourcePolicies::~ResourcePolicies()
{
}

void ResourcePolicies::set_sources(Sources* sources)
{
    static_assert(RESOURCE_POLICIES_PRIORITY > Sources::SOURCES_PRIORITY, "Wrong component priority");
    m_sources = sources;
    m_sources->connect_remove_source([this](SourceKey source_key, const std::vector<SourceKey>& removed_sources, Transaction& t){
        on_remove_sources(removed_sources, t);
    }, RESOURCE_POLICIES_PRIORITY);
}

void ResourcePolicies::set_roles(Roles* roles)
{
    static_assert(RESOURCE_POLICIES_PRIORITY > Roles::ROLES_PRIORITY, "Wrong component priority");
    m_roles = roles;
    m_roles->connect_remove_role([this](RoleKey role_key, Transaction& t){
        on_remove_role(role_key, t);
    }, RESOURCE_POLICIES_PRIORITY);
}

Policies ResourcePolicies::get_policies(SubjectRef subject_ref, ResourceRef resource_ref) const
{
    ReadOnlyTransaction t(m_db);
    auto resource_policy_data = m_resource_policy_dataset->get(pack(subject_ref), pack(resource_ref));
    return unpack_resource_policy(resource_policy_data);
}

void ResourcePolicies::set_policies(SubjectRef subject_ref, ResourceRef resource_ref, Policies policies, Transaction& t)
{
    if (get_policies(subject_ref, resource_ref) == policies)
    {
        return;
    }

    m_resource_policy_dataset->put(pack_resource_policy(subject_ref, resource_ref, policies), t);
    m_update_resource_policy_sig(subject_ref, resource_ref, t);
}

bool ResourcePolicies::is_enabled(SubjectRef subject_ref, ResourceRef resource_ref) const
{
    ReadOnlyTransaction t(m_db);
    auto policies = get_policies(subject_ref, resource_ref);

    if (policies.m_enable_policy == EnablePolicy::ENABLE)
    {
        return true;
    }

    if (policies.m_enable_policy == EnablePolicy::DISABLE)
    {
        return false;
    }

    if (refer_default_role(subject_ref))
    {
        return true;
    }

    return is_enabled(*get_base_ref(subject_ref), resource_ref);
}

EnablePolicy ResourcePolicies::get_enable_policy(SubjectRef subject_ref, ResourceRef resource_ref) const
{
    return get_policies(subject_ref, resource_ref).m_enable_policy;
}

void ResourcePolicies::set_enable_policy(SubjectRef subject_ref, ResourceRef resource_ref, EnablePolicy enable_policy, Transaction& t)
{
    auto policies = get_policies(subject_ref, resource_ref);
    if (policies.m_enable_policy == enable_policy)
    {
        return;
    }

    policies.m_enable_policy = enable_policy;
    m_resource_policy_dataset->put(pack_resource_policy(subject_ref, resource_ref, policies), t);
    m_update_resource_policy_sig(subject_ref, resource_ref, t);
}

bool ResourcePolicies::is_visible(SubjectRef subject_ref, ResourceRef resource_ref) const
{
    ReadOnlyTransaction t(m_db);
    auto policies = get_policies(subject_ref, resource_ref);

    if (policies.m_visibility == Visibility::VISIBLE)
    {
        return true;
    }

    if (policies.m_visibility == Visibility::INVISIBLE)
    {
        return false;
    }

    if (refer_default_role(subject_ref))
    {
        return true;
    }

    return is_visible(*get_base_ref(subject_ref), resource_ref);
}

Visibility ResourcePolicies::get_visibility(SubjectRef subject_ref, ResourceRef resource_ref) const
{
    return get_policies(subject_ref, resource_ref).m_visibility;
}

void ResourcePolicies::set_visibility(SubjectRef subject_ref, ResourceRef resource_ref, Visibility visibility, Transaction& t)
{
    auto policies = get_policies(subject_ref, resource_ref);
    if (policies.m_visibility == visibility)
    {
        return;
    }

    policies.m_visibility = visibility;
    m_resource_policy_dataset->put(pack_resource_policy(subject_ref, resource_ref, policies), t);
    m_update_resource_policy_sig(subject_ref, resource_ref, t);
}

bool ResourcePolicies::is_deferred(SubjectRef subject_ref, ResourceRef resource_ref) const
{
    ReadOnlyTransaction t(m_db);
    auto policies = get_policies(subject_ref, resource_ref);

    if (policies.m_defer_policy == DeferPolicy::DEFER)
    {
        return true;
    }

    if (policies.m_defer_policy == DeferPolicy::NOT_DEFER)
    {
        return false;
    }

    if (refer_default_role(subject_ref))
    {
        return false;
    }

    return is_deferred(*get_base_ref(subject_ref), resource_ref);
}

DeferPolicy ResourcePolicies::get_defer_policy(SubjectRef subject_ref, ResourceRef resource_ref) const
{
    return get_policies(subject_ref, resource_ref).m_defer_policy;
}

void ResourcePolicies::set_defer_policy(SubjectRef subject_ref, ResourceRef resource_ref, DeferPolicy defer_policy, Transaction& t)
{
    auto policies = get_policies(subject_ref, resource_ref);
    if (policies.m_defer_policy == defer_policy)
    {
        return;
    }

    policies.m_defer_policy = defer_policy;
    m_resource_policy_dataset->put(pack_resource_policy(subject_ref, resource_ref, policies), t);
    m_update_resource_policy_sig(subject_ref, resource_ref, t);
}

UUID ResourcePolicies::get_effective_setup_uuid(SubjectRef subject_ref, ResourceRef resource_ref) const
{
    ReadOnlyTransaction t(m_db);
    auto policies = get_policies(subject_ref, resource_ref);

    if (policies.m_setup_uuid != stl_tools::null_uuid())
    {
        return policies.m_setup_uuid;
    }    

    if (refer_default_role(subject_ref))
    {
        return stl_tools::null_uuid();
    }

    return get_effective_setup_uuid(*get_base_ref(subject_ref), resource_ref);
}

UUID ResourcePolicies::get_setup_uuid(SubjectRef subject_ref, ResourceRef resource_ref) const
{
    return get_policies(subject_ref, resource_ref).m_setup_uuid;
}

void ResourcePolicies::set_setup_uuid(SubjectRef subject_ref, ResourceRef resource_ref, UUID setup_uuid, Transaction& t)
{
    auto policies = get_policies(subject_ref, resource_ref);
    if (policies.m_setup_uuid == setup_uuid)
    {
        return;
    }

    policies.m_setup_uuid = setup_uuid;
    m_resource_policy_dataset->put(pack_resource_policy(subject_ref, resource_ref, policies), t);
    m_update_resource_policy_sig(subject_ref, resource_ref, t);
}

ResourcePolicyData ResourcePolicies::pack_resource_policy(SubjectRef subject_ref, ResourceRef resource_ref, const Policies& policies) const
{
    return {{pack(subject_ref), pack(resource_ref)}, policies.m_enable_policy, policies.m_visibility, policies.m_defer_policy, policies.m_setup_uuid };
}

Policies ResourcePolicies::unpack_resource_policy(const ResourcePolicyData& resource_policy_data) const
{
    return {resource_policy_data.m_enable_policy, resource_policy_data.m_visibility, resource_policy_data.m_defer_policy, resource_policy_data.m_setup_uuid };
}

void ResourcePolicies::on_remove_sources(const std::vector<SourceKey>& removed_sources, Transaction& t)
{
    m_resource_policy_dataset->erase_source_equal(removed_sources, t);
}

void ResourcePolicies::on_remove_role(RoleKey role_key, Transaction& t)
{
    m_resource_policy_dataset->erase_subject_equal(role_key, t);
}

void ResourcePolicies::on_remove_report_type(ReportTypeUUID report_type_uuid, Transaction& t)
{
    m_resource_policy_dataset->erase_resource_equal(report_type_uuid, t);
}

void ResourcePolicies::on_remove_stream_type(StreamTypeUUID stream_type_uuid, Transaction& t)
{
    m_resource_policy_dataset->erase_resource_equal(stream_type_uuid, t);
}

}} //namespace TR {namespace Core {