#include "stdafx.h"
#include "SourceResources.h"
#include "Basis.h"
#include "Sources.h"
#include "Links.h"
#include "Roles.h"
#include "ResourcePolicies.h"
#include "Executive.h"
#include "Transaction.h"
namespace TR { namespace Core {

SourceResource::SourceResource(SourceRef source_ref, ResourceRef resource_ref, Activity activity, bool available, bool visible, bool significant, bool deferred, bool persistent, UUID setup_uuid):
    m_source_ref(source_ref),
    m_resource_ref(resource_ref),
    m_activity(activity),
    m_available(available),
    m_visible(visible),
    m_significant(significant),
    m_deferred(deferred),
    m_persistent(persistent),
    m_setup_uuid(std::move(setup_uuid))
{
}

bool operator < (const SourceResource& left, const SourceResource& right)
{
    if (left.m_source_ref < right.m_source_ref)
        return true;

    if (right.m_source_ref < left.m_source_ref)
        return false;

    if (left.m_resource_ref < right.m_resource_ref)
        return true;

    if (right.m_resource_ref < left.m_resource_ref)
        return false;

    if (left.m_activity < right.m_activity)
        return true;

    if (right.m_activity < left.m_activity)
        return false;

    if (left.m_available < right.m_available)
        return true;

    if (right.m_available < left.m_available)
        return false;

    if (left.m_visible < right.m_visible)
        return true;

    if (right.m_visible < left.m_visible)
        return false;

    if (left.m_significant < right.m_significant)
        return true;

    if (right.m_significant < left.m_significant)
        return false;

    if (left.m_deferred < right.m_deferred)
        return true;

    if (right.m_deferred < left.m_deferred)
        return false;

    if (left.m_persistent < right.m_persistent)
        return true;

    if (right.m_persistent < left.m_persistent)
        return false;

    if (left.m_setup_uuid < right.m_setup_uuid)
        return true;

    if (right.m_setup_uuid < left.m_setup_uuid)
        return false;

    return false;
}

bool operator == (const SourceResource& left, const SourceResource& right)
{
    return !(left < right || right < left);
}

bool operator != (const SourceResource& left, const SourceResource& right)
{
    return !(left == right);
}

namespace {
struct SourceResourceIndex: stl_tools::unique_storage_index<std::pair<SourceKey, ResourceUUID>>
{
    static key_type get_key(const SourceResource& source_resource)
    {
        return std::make_pair(source_resource.m_source_ref.get_key(), get_uuid(source_resource.m_resource_ref));
    }
};

struct SourceActivityIndex: stl_tools::unique_storage_index<std::pair<SourceKey, ActivityIdentity>>
{
    static key_type get_key(const SourceResource& source_resource)
    {
        return std::make_pair(source_resource.m_source_ref.get_key(), get_identity(source_resource.m_activity));
    }
};

struct SourceIndex: stl_tools::single_storage_index<SourceKey>
{
    static key_type get_key(const SourceResource& source_resource)
    {
        return source_resource.m_source_ref.get_key();
    }
};

struct ResourceIndex: stl_tools::single_storage_index<ResourceUUID>
{
    static key_type get_key(const SourceResource& source_resource)
    {
        return get_uuid(source_resource.m_resource_ref);
    }
};

} //namespace {

class SourceResourceStorage: public stl_tools::storage<SourceResource, SourceResourceIndex, SourceActivityIndex, SourceIndex, ResourceIndex>
{
};

SourceResources::SourceResources(Database& db, Basis& basis):
    m_db(db),
    m_basis(basis),
    m_sources(nullptr),
    m_links(nullptr),
    m_roles(nullptr),
    m_resource_policies(nullptr),
    m_source_type_activities(basis.m_source_type_activities),
    m_storage(std::make_unique<SourceResourceStorage>()),
    m_running(false)
{
    static_assert(SOURCE_RESOURCES_PRIORITY > Basis::BASIS_PRIORITY, "Wrong component priority");

    m_source_type_activities.connect_add_activity([this](const SourceTypeActivity& source_type_activity, Transaction& t){
        if (m_running)
        {
            on_add_activity(source_type_activity, t);
        }
    }, SOURCE_RESOURCES_PRIORITY);

    m_source_type_activities.connect_remove_activity([this](SourceTypeRef source_type_ref, ActivityIdentity activity_identity, Transaction& t){
        if (m_running)
        {
            on_remove_activity(source_type_ref, activity_identity, t);
        }
    }, SOURCE_RESOURCES_PRIORITY);
}

SourceResources::~SourceResources()
{
}

void SourceResources::set_sources(Sources* sources)
{
    static_assert(SOURCE_RESOURCES_PRIORITY > Sources::SOURCES_PRIORITY, "Wrong component priority");

    _ASSERT(!m_sources);
    m_sources = sources;

    m_sources->connect_add_source([this](SourceRef source_ref, Transaction& t){
        _ASSERT(m_running);
        on_add_source(source_ref, t);
    }, SOURCE_RESOURCES_PRIORITY);

    m_sources->connect_rerole_source([this](SourceRef source_ref, RoleRef prev_role, RoleRef next_role, Transaction& t){
        _ASSERT(m_running);
        on_change_subject_role(source_ref, prev_role, next_role, t);
    }, SOURCE_RESOURCES_PRIORITY);

    m_sources->connect_remove_source([this](SourceKey source_key, const std::vector<SourceKey>& removed_sources, Transaction& t){
        _ASSERT(m_running);
        on_remove_sources(removed_sources, t);
    }, SOURCE_RESOURCES_PRIORITY);
}

void SourceResources::set_links(Links* links)
{
    static_assert(SOURCE_RESOURCES_PRIORITY > Links::LINKS_PRIORITY, "Wrong component priority");
    _ASSERT(!m_links);
    m_links = links;
}

void SourceResources::set_roles(Roles* roles)
{
    static_assert(SOURCE_RESOURCES_PRIORITY > Roles::ROLES_PRIORITY, "Wrong component priority");
    _ASSERT(!m_roles);
    m_roles = roles;

    m_roles->connect_replace_role([this](RoleRef role_ref, RoleRef prev_parent_ref, RoleRef next_parent_ref, Transaction& t){
        _ASSERT(m_running);
        on_change_subject_role(role_ref, prev_parent_ref, next_parent_ref, t);
    }, SOURCE_RESOURCES_PRIORITY);
}

void SourceResources::set_resource_policies(ResourcePolicies* resource_policies)
{
    static_assert(SOURCE_RESOURCES_PRIORITY > ResourcePolicies::RESOURCE_POLICIES_PRIORITY, "Wrong component priority");

    _ASSERT(!m_resource_policies);
    m_resource_policies = resource_policies;

    m_resource_policies->connect_update_resource_policy([this](SubjectRef subject_ref, ResourceRef resource_ref, Transaction& t){
        _ASSERT(m_running);
        on_update_policy(subject_ref, resource_ref, t);
    }, SOURCE_RESOURCES_PRIORITY);
}

void SourceResources::add_service_request(std::function<bool(SourceRef source_ref, ResourceRef resource_ref)> service_request)
{
    m_service_requests.push_back(service_request);
}

void SourceResources::check_service_request(SourceRef source_ref, ResourceRef resource_ref, Transaction& t)
{
    if (auto source_resource = find_source_resource(source_ref, resource_ref))
    {
        refresh_source_resource(*source_resource, t);
    }
}

void SourceResources::run(Executive* executive, Transaction& t)
{
    _ASSERT(m_sources && m_roles && m_resource_policies);
    for (auto source : m_sources->get_sources())
    {
        for (auto activity : m_source_type_activities.get_activities(source.get_source_type()))
        {
            add_activity_output(source, activity, t);
        }       
    }

    transact_assign(m_running, true, t);
}

SourceResources::SourceResourceRange SourceResources::get_source_resources(SourceRef source_ref) const
{
    return m_storage->find_range<SourceIndex>(source_ref.get_key());
}

SourceResource SourceResources::get_source_resource(SourceRef source_ref, ResourceRef resource_ref) const
{
    auto source_resource = m_storage->find({source_ref.get_key(), get_uuid(resource_ref)});
    if (source_resource == m_storage->end())
    {
        throw Exception(L"Invalid report type");
    }
    return *source_resource;
}

boost::optional<SourceResource> SourceResources::find_source_resource(SourceRef source_ref, ResourceRef resource_ref) const
{
    auto source_resource = m_storage->find({source_ref.get_key(), get_uuid(resource_ref)});
    if (source_resource == m_storage->end())
    {
        return boost::none;
    }
    return *source_resource;
}

std::vector<SourceResource> SourceResources::get_inputs(const SourceResource& source_resource) const
{
    std::vector<SourceResource> inputs;
    if (auto selection_activity = boost::get<SelectionActivity>(&source_resource.m_activity))
    {
        for (auto& input_ref : selection_activity->m_input_refs)
        {
            auto input = m_storage->find({source_resource.m_source_ref.get_key(), input_ref.get_uuid()});
            if (input != m_storage->end())
            {
                inputs.push_back(*input);
            }           
        }
    }

    if (auto convertion_activity = boost::get<ConvertionActivity>(&source_resource.m_activity))
    {
        auto input_ref = convertion_activity->m_input_ref;
        auto input = m_storage->find({source_resource.m_source_ref.get_key(), input_ref.get_uuid()});
        if (input != m_storage->end())
        {
            inputs.push_back(*input);
        }
    }

    if (auto grouping_activity = boost::get<GroupingActivity>(&source_resource.m_activity))
    {
        for (auto& child_source : m_sources->get_sources(source_resource.m_source_ref))
        {
            auto input = m_storage->find({child_source.get_ref().get_key(), grouping_activity->m_input_ref.get_uuid()});
            if (input != m_storage->end())
            {
                inputs.push_back(*input);
            }
        }

        for (auto& link : m_links->find_by_parent(source_resource.m_source_ref))
        {
            auto input = m_storage->find({link.m_child_ref.get_key(), grouping_activity->m_input_ref.get_uuid()});
            if (input != m_storage->end())
            {
                inputs.push_back(*input);
            }
        }
    }
    
    return inputs;
}

std::vector<SourceResource> SourceResources::get_depends(const SourceResource& source_resource) const
{
    auto source = *source_resource.m_source_ref;

    std::vector<SourceResource> depends;
    for (auto activity : m_source_type_activities.find_by_input(source.get_source_type(), source_resource.m_resource_ref))
    {
        if (auto selection_activity = boost::get<SelectionActivity>(&activity))
        {
            auto output_report = m_storage->find({source.get_ref().get_key(), selection_activity->m_output_ref.get_uuid()});
            if (output_report != m_storage->end())
            {
                depends.push_back(*output_report);
            }           
        }

        if (auto tracking_activity = boost::get<TrackingActivity>(&activity))
        {
            auto output_stream = m_storage->find({source.get_ref().get_key(), tracking_activity->m_output_ref.get_uuid()});
            if (output_stream != m_storage->end())
            {
                depends.push_back(*output_stream);
            }           
        }

        if (auto convertion_activity = boost::get<ConvertionActivity>(&activity))
        {
            auto output_stream = m_storage->find({source.get_ref().get_key(), convertion_activity->m_output_ref.get_uuid()});
            if (output_stream != m_storage->end())
            {
                depends.push_back(*output_stream);
            }
        }       
    }

    std::vector<SourceRef> parent_refs;
    if (source.has_parent())
    {
        parent_refs.push_back(source.get_parent());
    }

    for (auto& link : m_links->find_by_child(source_resource.m_source_ref))
    {
        parent_refs.push_back(link.m_parent_ref);
    }

    for (auto& parent_ref : parent_refs)
    {
        for (auto activity : m_source_type_activities.find_by_input((*parent_ref).get_source_type(), source_resource.m_resource_ref))
        {
            if (auto grouping_activity = boost::get<GroupingActivity>(&activity))
            {
                auto output_report = m_storage->find({parent_ref.get_key(), grouping_activity->m_output_ref.get_uuid()});
                if (output_report != m_storage->end())
                {
                    depends.push_back(*output_report);
                }
            }
        }
    }

    return depends;
}

void SourceResources::add_activity_output(SourceRef source_ref, const Activity& activity, Transaction& t)
{
    if (auto output_ref = get_output_ref(activity))
    {
        SourceResource source_resource(source_ref, *output_ref, activity, false, false, false, false, false, stl_tools::null_uuid());
        transact_insert(*m_storage, source_resource, t);
        refresh_source_resource(source_resource, t);
    }
}

SourceResource SourceResources::research_activity_output(SourceRef source_ref, const Activity& activity)
{
    auto output_ref = *get_output_ref(activity);

    bool available = m_resource_policies->is_enabled(source_ref, output_ref);
    if (available)
    {
        if (activity.type() == typeid(SelectionActivity))
        {
            for (auto& input_report : get_inputs({source_ref, output_ref, activity, false, false, false, false, false, stl_tools::null_uuid()}))
            {
                _ASSERT(input_report.m_source_ref == source_ref);
                if (input_report.m_available == false)
                {
                    available = false;
                    break;
                }
            }
        }
    }
    
    auto visible = available && m_resource_policies->is_visible(source_ref, output_ref);
    auto deferred = visible && m_resource_policies->is_deferred(source_ref, output_ref);
    auto persistent = visible && !deferred;
    auto significant = persistent;

    if (available && !significant)
    {
        significant = is_required_by_service(source_ref, output_ref);
    }

    if (available && !significant)
    {
        for (auto& depend_report : get_depends({source_ref, output_ref, activity, false, false, false, false, false, stl_tools::null_uuid()}))
        {
            if (depend_report.m_significant)
            {
                significant = true;             
                break;
            }          
        }
    }   

    auto setup_uuid = m_resource_policies->get_effective_setup_uuid(source_ref, output_ref);
    return {source_ref, output_ref, activity, available, visible, significant, deferred, persistent, setup_uuid};
}

void SourceResources::remove_activity_output(SourceRef source_ref, const ActivityIdentity& activity_identity, Transaction& t)
{
    auto position = m_storage->find<SourceActivityIndex>({source_ref.get_key(), activity_identity});
    if (position == m_storage->end())
    {
        return;
    }

    auto source_resource = *position;
    m_storage->erase(position);
    m_remove_source_resource_sig(source_ref, source_resource.m_resource_ref, t);

    //refresh inputs
    for (auto& input_resource : get_inputs(source_resource))
    {
        if (!input_resource.m_significant)
            continue; //optimisation
        refresh_source_resource(input_resource, t);
    }
}

void SourceResources::refresh_source_resource(const SourceResource& prev_source_resource, Transaction& t)
{
    auto& source_ref = prev_source_resource.m_source_ref;
    auto& resource_ref = prev_source_resource.m_resource_ref;
    auto& activity = prev_source_resource.m_activity;

    _ASSERT(m_storage->find({source_ref.get_key(), get_uuid(resource_ref)}) != m_storage->end());
    _ASSERT(*m_storage->find({source_ref.get_key(), get_uuid(resource_ref)}) == prev_source_resource);

    auto next_source_resource = research_activity_output(source_ref, activity);
    if (prev_source_resource == next_source_resource)
    {
        return;
    }

    transact_update(*m_storage, next_source_resource, t);
    m_update_source_resource_sig(next_source_resource, t);
    
    //refresh inputs
    if (prev_source_resource.m_significant != next_source_resource.m_significant)
    {
        for (auto& input_report : get_inputs(next_source_resource))
        {
            if (input_report.m_significant == next_source_resource.m_significant)
                continue; //optimisation
            
            refresh_source_resource(input_report, t);
        }
    }

    //refresh depends
    if (prev_source_resource.m_available != next_source_resource.m_available)
    {
        for (auto& depend_report : get_depends(next_source_resource))
        {
            if (depend_report.m_source_ref == source_ref)
            {
                if (depend_report.m_available != next_source_resource.m_available)
                {
                    refresh_source_resource(depend_report, t);
                }
            }
        }
    }   
}

bool SourceResources::is_required_by_service(SourceRef source_ref, ResourceRef resource_ref) const
{
    for (auto& service_request : m_service_requests)
    {
        if (service_request(source_ref, resource_ref))
        {
            return true;
        }
    }
    return false;
}

void SourceResources::on_add_source(SourceRef source_ref, Transaction& t)
{
    auto& source_type = (*source_ref).get_source_type();
    for (auto& activity : m_source_type_activities.get_activities(source_type))
    {
        add_activity_output(source_ref, activity, t);
    }
}

void SourceResources::on_remove_sources(const std::vector<SourceKey>& removed_sources, Transaction& t)
{
    for (auto removed_source : removed_sources)
    {
        transact_erase_range(*m_storage, m_storage->find_range<SourceIndex>(removed_source), t);        
    }
}

void SourceResources::on_change_subject_role(SubjectRef subject_ref, RoleRef prev_role, RoleRef next_role, Transaction& t)
{
    for (auto& report_type : m_basis.m_report_types.get_report_types())
    {
        on_update_policy(subject_ref, report_type, t);
    }

    for (auto& stream_type : m_basis.m_stream_types.get_stream_types())
    {
        on_update_policy(subject_ref, stream_type, t);
    }
}

void SourceResources::on_add_activity(const SourceTypeActivity& source_type_activity, Transaction& t)
{
    for (auto source : m_sources->get_sources())
    {
        if (source.get_source_type() != source_type_activity.m_source_type_ref)
        {
            continue;
        }

        add_activity_output(source, source_type_activity.m_activity, t);
    }
}

void SourceResources::on_remove_activity(SourceTypeRef source_type_ref, ActivityIdentity activity_identity, Transaction& t)
{
    for (auto source : m_sources->get_sources())
    {
        if (source.get_source_type() != source_type_ref)
        {
            continue;
        }

        remove_activity_output(source, activity_identity, t);
    }
}

void SourceResources::on_update_policy(SubjectRef subject_ref, ResourceRef resource_ref, Transaction& t)
{
    if (auto source_ref = boost::get<SourceRef>(&subject_ref))
    {
        if (auto source_resource = find_source_resource(*source_ref, resource_ref))
        {
            refresh_source_resource(*source_resource, t);
        }
    }
    else
    {
        _ASSERT(boost::get<RoleRef>(&subject_ref));
        for (auto child_ref : get_derived_refs(*m_sources, *m_roles, subject_ref))
        {
            on_update_policy(child_ref, resource_ref, t);
        }
    }
}

}} //namespace TR { namespace Core {