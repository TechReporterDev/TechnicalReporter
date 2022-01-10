#pragma once
#include "RegistryDecl.h"
namespace TR { namespace Core {
class ResourcePolicyDataset;
struct ResourcePolicyData;

enum class EnablePolicy { DEFAULT_POLICY = 0, ENABLE, DISABLE };
enum class DeferPolicy { DEFAULT_POLICY = 0, DEFER, NOT_DEFER };
enum class Visibility { DEFAULT_VISIBILITY = 0, VISIBLE, INVISIBLE };

struct Policies
{
    Policies():
        m_enable_policy(EnablePolicy::DEFAULT_POLICY),
        m_defer_policy(DeferPolicy::DEFAULT_POLICY),
        m_visibility(Visibility::DEFAULT_VISIBILITY),
        m_setup_uuid(stl_tools::null_uuid())
    {
    }

    Policies(EnablePolicy enable_policy, Visibility visibility, DeferPolicy defer_policy, UUID setup_uuid = stl_tools::null_uuid()):
        m_enable_policy(enable_policy),
        m_visibility(visibility),
        m_defer_policy(defer_policy),
        m_setup_uuid(std::move(setup_uuid))
    {
    }

    EnablePolicy    m_enable_policy;
    Visibility      m_visibility;
    DeferPolicy     m_defer_policy;
    UUID            m_setup_uuid;
};

inline bool operator == (const Policies& left, const Policies& right)
{
    return left.m_enable_policy == right.m_enable_policy &&
        left.m_visibility == right.m_visibility &&
        left.m_defer_policy == right.m_defer_policy &&
        left.m_setup_uuid == right.m_setup_uuid;
}

class ResourcePolicies
{   
public:
    static const Priority RESOURCE_POLICIES_PRIORITY = 700;

    ResourcePolicies(Database& db, Basis& basis);
    ~ResourcePolicies();
    void                    set_sources(Sources* sources);
    void                    set_roles(Roles* roles);

    Policies                get_policies(SubjectRef subject_ref, ResourceRef resource_ref) const;
    void                    set_policies(SubjectRef subject_ref, ResourceRef resource_ref, Policies policies, Transaction& t);

    bool                    is_enabled(SubjectRef subject_ref, ResourceRef resource_ref) const;
    EnablePolicy            get_enable_policy(SubjectRef subject_ref, ResourceRef resource_ref) const;
    void                    set_enable_policy(SubjectRef subject_ref, ResourceRef resource_ref, EnablePolicy enable_policy, Transaction& t);

    bool                    is_visible(SubjectRef subject_ref, ResourceRef resource_ref) const;
    Visibility              get_visibility(SubjectRef subject_ref, ResourceRef resource_ref) const;
    void                    set_visibility(SubjectRef subject_ref, ResourceRef resource_ref, Visibility visibility, Transaction& t);

    bool                    is_deferred(SubjectRef subject_ref, ResourceRef resource_ref) const;
    DeferPolicy             get_defer_policy(SubjectRef subject_ref, ResourceRef resource_ref) const;
    void                    set_defer_policy(SubjectRef subject_ref, ResourceRef resource_ref, DeferPolicy defer_policy, Transaction& t);

    UUID                    get_effective_setup_uuid(SubjectRef subject_ref, ResourceRef resource_ref) const;
    UUID                    get_setup_uuid(SubjectRef subject_ref, ResourceRef resource_ref) const;
    void                    set_setup_uuid(SubjectRef subject_ref, ResourceRef resource_ref, UUID setup_uuid, Transaction& t);

    template<class T>
    void connect_update_resource_policy(T slot, Priority priority) { m_update_resource_policy_sig.connect(slot, priority); }

private:
    ResourcePolicyData      pack_resource_policy(SubjectRef subject_ref, ResourceRef resource_ref, const Policies& policies) const;
    Policies                unpack_resource_policy(const ResourcePolicyData& resource_policy_data) const;

    void                    on_remove_sources(const std::vector<SourceKey>& removed_sources, Transaction& t);
    void                    on_remove_role(RoleKey role_key, Transaction& t);
    void                    on_remove_report_type(ReportTypeUUID report_type_uuid, Transaction& t);
    void                    on_remove_stream_type(StreamTypeUUID stream_type_uuid, Transaction& t);

    OrderedSignal<void(SubjectRef subject_ref, ResourceRef resource_ref, Transaction& t)> m_update_resource_policy_sig;
    Database& m_db;
    ReportTypes& m_report_types;
    StreamTypes& m_stream_types;
    Sources* m_sources;
    Roles* m_roles;

    std::unique_ptr<ResourcePolicyDataset> m_resource_policy_dataset;
};

}} //namespace TR { namespace Core {