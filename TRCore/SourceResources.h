#pragma once
#include "Activity.h"
#include "RegistryDecl.h"
#include "Sources.h"
#include "Roles.h"
namespace TR { namespace Core {
class SourceResourceStorage;

struct SourceResource
{
    SourceResource(SourceRef source_ref, ResourceRef resource_ref, Activity activity, bool available, bool visible, bool significant, bool deferred, bool persistent, UUID setup_uuid);
    
    SourceRef       m_source_ref;
    ResourceRef     m_resource_ref;
    Activity        m_activity;
    bool            m_available;
    bool            m_visible;
    bool            m_significant;
    bool            m_deferred;
    bool            m_persistent;
    UUID            m_setup_uuid;
};

bool operator < (const SourceResource& left, const SourceResource& right);
bool operator == (const SourceResource& left, const SourceResource& right);
bool operator != (const SourceResource& left, const SourceResource& right);

class SourceResources
{
public:
    static const Priority SOURCE_RESOURCES_PRIORITY = 1200;
    using SourceResourceRange = boost::any_range<const SourceResource, boost::forward_traversal_tag>;
    
    SourceResources(Database& db, Basis& basis);
    ~SourceResources();

    void                                set_sources(Sources* sources);
    void                                set_links(Links* links);
    void                                set_roles(Roles* roles);
    void                                set_resource_policies(ResourcePolicies* resource_policies);
    void                                add_service_request(std::function<bool(SourceRef source_ref, ResourceRef resource_ref)>);
    void                                check_service_request(SourceRef source_ref, ResourceRef resource_ref, Transaction& t);
    void                                run(Executive* executive, Transaction& t);

    SourceResourceRange                 get_source_resources(SourceRef source_ref) const;
    SourceResource                      get_source_resource(SourceRef source_ref, ResourceRef resource_ref) const;
    boost::optional<SourceResource>     find_source_resource(SourceRef source_ref, ResourceRef resource_ref) const;
    std::vector<SourceResource>         get_inputs(const SourceResource& source_resource) const;
    std::vector<SourceResource>         get_depends(const SourceResource& source_resource) const;
                
    template<class T>
    void connect_update_source_resource(T slot, Priority priority) { m_update_source_resource_sig.connect(slot, priority); }

    template<class T>
    void connect_remove_source_resource(T slot, Priority priority) { m_remove_source_resource_sig.connect(slot, priority); }

private:
    void                                add_activity_output(SourceRef source_ref, const Activity& activity, Transaction& t);
    SourceResource                      research_activity_output(SourceRef source_ref, const Activity& activity);
    void                                remove_activity_output(SourceRef source_ref, const ActivityIdentity& activity_identity, Transaction& t);
    void                                refresh_source_resource(const SourceResource& current_source_resource, Transaction& t);

    bool                                is_required_by_service(SourceRef source_ref, ResourceRef resource_ref) const;

    void                                on_add_source(SourceRef source_ref, Transaction& t);
    void                                on_remove_sources(const std::vector<SourceKey>& removed_sources, Transaction& t);
    void                                on_change_subject_role(SubjectRef subject_ref, RoleRef prev_role, RoleRef next_role, Transaction& t);
    void                                on_add_activity(const SourceTypeActivity& source_type_activity, Transaction& t);
    void                                on_remove_activity(SourceTypeRef source_type_ref, ActivityIdentity activity_identity, Transaction& t);
    void                                on_update_policy(SubjectRef subject_ref, ResourceRef resource_ref, Transaction& t);

    Database& m_db;
    Basis& m_basis;
    SourceTypeActivities& m_source_type_activities;
    Sources* m_sources;
    Links* m_links;
    Roles* m_roles;
    ResourcePolicies* m_resource_policies;

    std::vector<std::function<bool(SourceRef source_ref, ResourceRef resource_ref)>> m_service_requests;
    OrderedSignal<void(const SourceResource& source_resource, Transaction& t)> m_update_source_resource_sig;
    OrderedSignal<void(SourceRef source_ref, ResourceRef resource_ref, Transaction& t)> m_remove_source_resource_sig;

    std::unique_ptr<SourceResourceStorage> m_storage;
    bool m_running;
};

}} //namespace TR { namespace Core {