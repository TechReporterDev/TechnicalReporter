#include "stdafx.h"
#include "SourceTypeActivities.h"
#include "SourceTypes.h"
#include "ReportTypes.h"
#include "Transformations.h"
#include "ActionDelegates.h"
#include "ActionShortcuts.h"
#include "Actions.h"
#include "Executive.h"
#include "Transaction.h"
namespace TR { namespace Core {

bool operator < (const SourceTypeActivity& left, const SourceTypeActivity& right)
{
    if (left.m_source_type_ref < right.m_source_type_ref)
        return true;

    if (right.m_source_type_ref < left.m_source_type_ref)
        return false;

    if (left.m_activity < right.m_activity)
        return true;

    if (right.m_activity < left.m_activity)
        return false;

    return false;
}

bool operator == (const SourceTypeActivity& left, const SourceTypeActivity& right)
{
    return !(left < right || right < left);
}

bool operator != (const SourceTypeActivity& left, const SourceTypeActivity& right)
{
    return !(left == right);
}

namespace {
struct SourceTypeActivityIndex: stl_tools::unique_storage_index<std::pair<UUID, ActivityIdentity>>
{
    static key_type get_key(const SourceTypeActivity& source_type_activity)
    {
        return std::make_pair(source_type_activity.m_source_type_ref.get_uuid(), get_identity(source_type_activity.m_activity));
    }
};

struct SourceTypeIndex: stl_tools::single_storage_index<UUID>
{
    static key_type get_key(const SourceTypeActivity& source_type_activity)
    {
        return source_type_activity.m_source_type_ref.get_uuid();
    }
};

struct ActivityIndex: stl_tools::single_storage_index<ActivityIdentity>
{
    static key_type get_key(const SourceTypeActivity& source_type_activity)
    {
        return get_identity(source_type_activity.m_activity);
    }
};

struct SourceTypeInputIndex: stl_tools::multi_storage_index<std::pair<UUID, ResourceUUID>>
{
    template<class F>
    static void enum_keys(const SourceTypeActivity& source_type_activity, F f)
    {
        for (auto input_ref : get_input_refs(source_type_activity.m_activity))
        {
            f(std::make_pair(source_type_activity.m_source_type_ref.get_uuid(), get_uuid(input_ref)));
        }
    }
};

struct SourceTypeOutputIndex: stl_tools::unique_storage_index<std::pair<UUID, ResourceUUID>, stl_tools::storage_index_optional::true_type>
{
    static boost::optional<std::pair<UUID, ResourceUUID>> get_key(const SourceTypeActivity& source_type_activity)
    {
        if (auto output_ref = get_output_ref(source_type_activity.m_activity))
        {
            return std::make_pair(source_type_activity.m_source_type_ref.get_uuid(), get_uuid(*output_ref));
        }
        return boost::none;
    }
};

struct SourceTypeActionIndex: stl_tools::unique_storage_index<std::pair<UUID, ActionUUID>, stl_tools::storage_index_optional::true_type>
{
    static boost::optional<std::pair<UUID, ActionUUID>> get_key(const SourceTypeActivity& source_type_activity)
    {
        if (auto action_ref = get_action_ref(source_type_activity.m_activity))
        {
            return std::make_pair(source_type_activity.m_source_type_ref.get_uuid(), action_ref->get_uuid());
        }
        return boost::none;
    }
};

struct SourceTypeHostActionIndex: stl_tools::multi_storage_index<std::pair<UUID, ActionUUID>>
{
    template<class F>
    static void enum_keys(const SourceTypeActivity& source_type_activity, F f)
    {
        for (auto host_action_ref : get_host_action_refs(source_type_activity.m_activity))
        {
            f(std::make_pair(source_type_activity.m_source_type_ref.get_uuid(), host_action_ref.get_uuid()));
        }
    }   
};

struct SourceTypeShortcutIndex: stl_tools::unique_storage_index<std::pair<UUID, UUID>, stl_tools::storage_index_optional::true_type>
{
    static boost::optional<std::pair<UUID, UUID>> get_key(const SourceTypeActivity& source_type_activity)
    {
        if (auto shortcut_activity = boost::get<const ShortcutActivity>(&source_type_activity.m_activity))
        {
            return std::make_pair(source_type_activity.m_source_type_ref.get_uuid(), shortcut_activity->m_shortcut_ref.get_uuid());
        }
        return boost::none;
    }
};
} //namespace {

class SourceTypeActivityStorage: public stl_tools::storage<SourceTypeActivity, SourceTypeActivityIndex,
    SourceTypeIndex, ActivityIndex, SourceTypeInputIndex, SourceTypeOutputIndex, SourceTypeActionIndex, SourceTypeHostActionIndex, SourceTypeShortcutIndex>
{
};

SourceTypeActivities::SourceTypeActivities():
    m_source_types(nullptr),
    m_report_types(nullptr),
    m_transformations(nullptr),
    m_action_delegates(nullptr),
    m_action_shortcuts(nullptr),
    m_storage(std::make_unique<SourceTypeActivityStorage>()),
    m_running(false)
{
}

SourceTypeActivities::~SourceTypeActivities()
{
}

void SourceTypeActivities::set_source_types(SourceTypes* source_types)
{
    static_assert(SOURCE_TYPE_ACTIVITIES_PRIORITY > SourceTypes::SOURCE_TYPES_PRIORITY, "Wrong component priority");

    _ASSERT(!m_source_types);
    m_source_types = source_types;

    m_source_types->connect_add_source_type([this](SourceTypeRef source_type_ref, Transaction& t){
        if (m_running)
        {
            on_add_source_type(source_type_ref, t);
        }
    }, SOURCE_TYPE_ACTIVITIES_PRIORITY);

    m_source_types->connect_remove_source_type([this](UUID source_type_uuid, Transaction& t){
        if (m_running)
        {
            on_remove_source_type(SourceTypeUUID(source_type_uuid), t);
        }
    }, SOURCE_TYPE_ACTIVITIES_PRIORITY);
}

void SourceTypeActivities::set_report_types(ReportTypes* report_types)
{
    static_assert(SOURCE_TYPE_ACTIVITIES_PRIORITY > ReportTypes::REPORT_TYPES_PRIORITY, "Wrong component priority");
    _ASSERT(!m_report_types);
    m_report_types = report_types;
}

void SourceTypeActivities::set_transformations(Transformations* transformations)
{
    _ASSERT(!m_transformations);
    m_transformations = transformations;

    m_transformations->m_selections.connect_add_selection([this](SelectionDirection direction, Transaction& t){
        if (m_running)
        {
            on_add_selection(direction, t);
        }
    }, SOURCE_TYPE_ACTIVITIES_PRIORITY);

    m_transformations->m_selections.connect_remove_selection([this](SelectionDirection direction, Transaction& t){
        if (m_running)
        {
            on_remove_selection(direction, t);
        }
    }, SOURCE_TYPE_ACTIVITIES_PRIORITY);

    m_transformations->m_groupings.connect_add_grouping([this](GroupingDirection direction, Transaction& t){
        if (m_running)
        {
            on_add_grouping(direction, t);
        }
    }, SOURCE_TYPE_ACTIVITIES_PRIORITY);

    m_transformations->m_groupings.connect_remove_grouping([this](GroupingDirection direction, Transaction& t){
        if (m_running)
        {
            on_remove_grouping(direction.m_input_ref, direction.m_output_ref, t);
        }
    }, SOURCE_TYPE_ACTIVITIES_PRIORITY);

    m_transformations->m_convertions.connect_add_convertion([this](ConvertionDirection direction, Transaction& t){
        if (m_running)
        {
            on_add_convertion(direction, t);
        }
    }, SOURCE_TYPE_ACTIVITIES_PRIORITY);

    m_transformations->m_convertions.connect_remove_convertion([this](ConvertionDirection direction, Transaction& t){
        if (m_running)
        {
            on_remove_convertion(direction, t);
        }
    }, SOURCE_TYPE_ACTIVITIES_PRIORITY);

    m_transformations->m_trackings.connect_add_tracking([this](TrackingDirection direction, Transaction& t){
        if (m_running)
        {
            on_add_tracking(direction, t);
        }
    }, SOURCE_TYPE_ACTIVITIES_PRIORITY);

    m_transformations->m_trackings.connect_remove_tracking([this](TrackingDirection direction, Transaction& t){
        if (m_running)
        {
            on_remove_tracking(direction, t);
        }
    }, SOURCE_TYPE_ACTIVITIES_PRIORITY);

    m_transformations->m_generators.connect_add_generator([this](UUID uuid, Transaction& t){
        if (m_running)
        {
            on_add_generator(uuid, t);
        }
    }, SOURCE_TYPE_ACTIVITIES_PRIORITY);

    m_transformations->m_generators.connect_remove_generator([this](UUID uuid, Transaction& t){
        if (m_running)
        {
            on_remove_generator(uuid, t);
        }
    }, SOURCE_TYPE_ACTIVITIES_PRIORITY);
}

void SourceTypeActivities::set_action_delegates(ActionDelegates* action_delegates)
{
    static_assert(SOURCE_TYPE_ACTIVITIES_PRIORITY > ActionDelegates::ACTION_DELEGATES_PRIORITY, "Wrong component priority");
    _ASSERT(!m_action_delegates);
    m_action_delegates = action_delegates;

    m_action_delegates->connect_add_delegate([this](const ActionDelegate& action_delegate, Transaction& t){
        on_add_action_delegate(action_delegate, t);
    }, SOURCE_TYPE_ACTIVITIES_PRIORITY);

    m_action_delegates->connect_remove_delegate([this](ActionDependency action_dependency, Transaction& t){
        on_remove_action_delegate(action_dependency, t);
    }, SOURCE_TYPE_ACTIVITIES_PRIORITY);
}

void SourceTypeActivities::set_shortcuts(ActionShortcuts* action_shortcuts)
{
    static_assert(SOURCE_TYPE_ACTIVITIES_PRIORITY > ActionShortcuts::ACTION_SHORTCUTS_PRIORITY, "Wrong component priority");
    _ASSERT(!m_action_shortcuts);
    m_action_shortcuts = action_shortcuts;

    m_action_shortcuts->connect_add_shortcut([this](const ActionShortcut& action_shortcut, Transaction& t){
        on_add_action_shortcut(action_shortcut, t);
    }, SOURCE_TYPE_ACTIVITIES_PRIORITY);

    m_action_shortcuts->connect_remove_shortcut([this](UUID shortcut_uuid, Transaction& t){
        on_remove_action_shortcut(shortcut_uuid, t);
    }, SOURCE_TYPE_ACTIVITIES_PRIORITY);
}

void SourceTypeActivities::run(Executive* executive, Transaction& t)
{       
    for (auto& source_type : m_source_types->get_source_types())
    {   
        add_activities(source_type, t);     
    }

    transact_assign(m_running, true, t);
}

SourceTypeActivities::ActivityRange SourceTypeActivities::get_activities(SourceTypeRef source_type_ref) const
{
    return m_storage->find_range<SourceTypeIndex>(source_type_ref.get_uuid()) | stl_tools::members(&SourceTypeActivity::m_activity);
}

SourceTypeActivities::ActivityRange SourceTypeActivities::find_by_input(SourceTypeRef source_type_ref, ResourceRef input_ref) const
{
    return m_storage->find_range<SourceTypeInputIndex>(std::make_pair(source_type_ref.get_uuid(), get_uuid(input_ref))) | stl_tools::members(&SourceTypeActivity::m_activity);
}

boost::optional<Activity> SourceTypeActivities::find_by_output(SourceTypeRef source_type_ref, ResourceRef output_ref) const
{
    auto position = m_storage->find<SourceTypeOutputIndex>({source_type_ref.get_uuid(), get_uuid(output_ref)});
    if (position == m_storage->end())
    {
        return boost::none;
    }
    return position->m_activity;
}

boost::optional<Activity> SourceTypeActivities::find_by_action(SourceTypeRef source_type_ref, ActionRef action_ref) const
{
    auto position = m_storage->find<SourceTypeActionIndex>({source_type_ref.get_uuid(), action_ref.get_uuid()});
    if (position == m_storage->end())
    {
        return boost::none;
    }
    return position->m_activity;    
}

boost::optional<Activity> SourceTypeActivities::find_by_shortcut(SourceTypeRef source_type_ref, ActionShortcutRef shortcut_ref) const
{
    auto position = m_storage->find<SourceTypeShortcutIndex>({source_type_ref.get_uuid(), shortcut_ref.get_uuid()});
    if (position == m_storage->end())
    {
        return boost::none;
    }
    return position->m_activity;
}

std::vector<ReportTypeRef> SourceTypeActivities::get_reloads(SourceTypeRef source_type_ref, ActionRef action_ref) const
{
    if (auto activity = find_by_action(source_type_ref, action_ref))
    {
        if (auto action_activity = boost::get<ActionActivity>(&*activity))
        {
            return (*source_type_ref).get_reloads(action_ref);          
        }

        if (auto action_delegate_activity = boost::get<ActionDelegateActivity>(&*activity))
        {
            std::vector<ReportTypeRef> reloads;
            for (auto host_action_ref : action_delegate_activity->m_host_action_refs)
            {
                boost::copy(get_reloads(source_type_ref, host_action_ref), std::back_inserter(reloads));
            }           
            return reloads;
        }
    }
    return {};
}

void SourceTypeActivities::add_activities(const SourceType& source_type, Transaction& t)
{
    for (auto download : source_type.get_downloads())
    {
        LoadingActivity loading_activity{download};
        add_activity({source_type, loading_activity}, t);
    }

    for (auto inflow_ref : source_type.get_streams())
    {
        InflowActivity inflow_activity{inflow_ref};
        add_activity({source_type, inflow_activity}, t);
    }

    for (auto generator : m_transformations->m_generators.find_by_source_type(source_type))
    {
        GeneratingActivity generating_activity(generator->get_report_type(), generator->get_uuid());
        add_activity({ source_type, generating_activity }, t);
    }

    for (auto grouping : m_transformations->m_groupings.get_groupings())
    {
        auto direction = grouping->get_direction();
        GroupingActivity grouping_activity(direction.m_input_ref, direction.m_output_ref, grouping->get_uuid());
        add_activity({source_type, grouping_activity}, t);
    }

    for (auto action_ref : source_type.get_actions())
    {
        ActionActivity action_activity(action_ref);
        add_activity({source_type, action_activity}, t);
    }
}

void SourceTypeActivities::add_activity(SourceTypeActivity source_type_activity, Transaction& t)
{
    if (auto selection_activity = boost::get<SelectionActivity>(&source_type_activity.m_activity))
    {
        auto current = m_storage->find({source_type_activity.m_source_type_ref.get_uuid(), get_identity(source_type_activity.m_activity)});
        if (current != m_storage->end() && get_uuid(current->m_activity) == selection_activity->m_uuid)
        {
            return;
        }
    }

    if (auto output_ref = get_output_ref(source_type_activity.m_activity))
    {
        auto current = m_storage->find<SourceTypeOutputIndex>({source_type_activity.m_source_type_ref.get_uuid(), get_uuid(*output_ref)});
        if (current != m_storage->end())
        {
            throw Exception(L"Activity conflict found");
        }
    }   

    transact_insert(*m_storage, source_type_activity, t);
    m_add_activity_sig(source_type_activity, t);

    if (auto output_ref = get_output_ref(source_type_activity.m_activity))
    {
        if (auto output_report_ref = boost::get<ReportTypeRef>(output_ref.get_ptr()))
        {
            for (auto& selection : m_transformations->m_selections.find_by_input(*output_report_ref))
            {
                auto selection_plan = selection->get_plan();
                SelectionActivity selection_activity(
                    get_inputs(selection_plan),
                    get_binding_inputs(selection_plan),
                    selection_plan.m_output_ref,
                    selection->get_uuid());

                if (validate_activity(source_type_activity.m_source_type_ref, selection_activity))
                {
                    add_activity({source_type_activity.m_source_type_ref, selection_activity}, t);
                }
            }

            for (auto& tracking : m_transformations->m_trackings.find_by_input(*output_report_ref))
            {
                auto direction = tracking->get_direction();
                TrackingActivity tracking_activity(direction.m_input_ref, direction.m_output_ref);
                if (validate_activity(source_type_activity.m_source_type_ref, tracking_activity))
                {
                    add_activity({source_type_activity.m_source_type_ref, tracking_activity}, t);
                }
            }

            for (auto& action_delegate : m_action_delegates->find_by_input(*output_report_ref))
            {
                ActionDelegateActivity action_delegate_activity(action_delegate.get_input_refs(), action_delegate.get_host_action_refs(), action_delegate.get_guest_action_ref());
                if (validate_activity(source_type_activity.m_source_type_ref, action_delegate_activity))
                {
                    add_activity({source_type_activity.m_source_type_ref, action_delegate_activity}, t);
                }
            }
        }
        else if (auto output_stream_ref = boost::get<StreamTypeRef>(output_ref.get_ptr()))
        {
            for (auto& convertion : m_transformations->m_convertions.find_by_input(*output_stream_ref))
            {
                auto direction = convertion->get_direction();
                ConvertionActivity convertion_activity(direction.m_input_ref, direction.m_output_ref);
                if (validate_activity(source_type_activity.m_source_type_ref, convertion_activity))
                {
                    add_activity({source_type_activity.m_source_type_ref, convertion_activity}, t);
                }
            }
        }
    }

    if (auto action_ref = get_action_ref(source_type_activity.m_activity))
    {
        for (auto& action_shortcut : m_action_shortcuts->find_shortcuts(*action_ref))
        {
            ShortcutActivity shortcut_activity(action_shortcut, action_shortcut.get_action_ref(), action_shortcut.get_output_ref());
            add_activity({source_type_activity.m_source_type_ref, shortcut_activity}, t);
        }

        for (auto& action_delegate : m_action_delegates->find_by_host_action(*action_ref))
        {
            ActionDelegateActivity action_delegate_activity(action_delegate.get_input_refs(), action_delegate.get_host_action_refs(), action_delegate.get_guest_action_ref());
            if (validate_activity(source_type_activity.m_source_type_ref, action_delegate_activity))
            {
                add_activity({source_type_activity.m_source_type_ref, action_delegate_activity}, t);
            }
        }
    }
}

void SourceTypeActivities::remove_activity(SourceTypeRef source_type_ref, const ActivityIdentity& identity, Transaction& t)
{
    auto found = m_storage->find({source_type_ref.get_uuid(), identity});
    if (found == m_storage->end())
    {
        _ASSERT(false);
        return;
    }
    auto activity = found->m_activity;
    transact_erase(*m_storage, found, t);
    m_remove_activity_sig(source_type_ref, identity, t);

    std::vector<ActivityIdentity> activities_to_remove;
    if (auto output_ref = get_output_ref(activity))
    {
        for (auto& child_activity : m_storage->find_range<SourceTypeInputIndex>(std::make_pair(source_type_ref.get_uuid(), get_uuid(*output_ref))))
        {
            if (!validate_activity(source_type_ref, child_activity.m_activity))
            {
                activities_to_remove.push_back(get_identity(child_activity.m_activity));
            }
        }       
    }

    if (auto action_ref = get_action_ref(activity))
    {
        for (auto& child_activity : m_storage->find_range<SourceTypeHostActionIndex>(std::make_pair(source_type_ref.get_uuid(), action_ref->get_uuid())))
        {
            if (!validate_activity(source_type_ref, child_activity.m_activity))
            {
                activities_to_remove.push_back(get_identity(child_activity.m_activity));
            }
        }
    }

    for (auto& activity_to_remove : activities_to_remove)
    {
        remove_activity(source_type_ref, activity_to_remove, t);
    }
}

bool SourceTypeActivities::validate_activity(SourceTypeRef source_type_ref, const Activity& activity)
{
    if (auto selection_activity = boost::get<SelectionActivity>(&activity))
    {
        if (selection_activity->m_binding_input_refs.empty())
        {
            for (auto& input_ref : selection_activity->m_input_refs)
            {
                if (m_storage->find<SourceTypeOutputIndex>({ source_type_ref.get_uuid(), input_ref.get_uuid() }) != m_storage->end())
                {
                    return true;
                }
            }
        }
        else
        {
            for (auto& input_ref : selection_activity->m_binding_input_refs)
            {
                if (m_storage->find<SourceTypeOutputIndex>({ source_type_ref.get_uuid(), input_ref.get_uuid() }) == m_storage->end())
                {
                    return false;
                }
            }
        }

        return !selection_activity->m_binding_input_refs.empty();
    }
    else if (auto convertion_activity = boost::get<ConvertionActivity>(&activity))
    {
        if (m_storage->find<SourceTypeOutputIndex>({source_type_ref.get_uuid(), convertion_activity->m_input_ref.get_uuid()}) == m_storage->end())
        {
            return false;
        }
        return true;
    }
    else if (auto tracking_activity = boost::get<TrackingActivity>(&activity))
    {
        if (m_storage->find<SourceTypeOutputIndex>({source_type_ref.get_uuid(), tracking_activity->m_input_ref.get_uuid()}) == m_storage->end())
        {
            return false;
        }
        return true;
    }
    else if (auto action_delegate_activity = boost::get<ActionDelegateActivity>(&activity))
    {
        for (auto input_ref : action_delegate_activity->m_input_refs)
        {
            if (m_storage->find<SourceTypeOutputIndex>({source_type_ref.get_uuid(), input_ref.get_uuid()}) == m_storage->end())
            {
                return false;
            }
        }

        for (auto host_action_ref : action_delegate_activity->m_host_action_refs)
        {
            if (m_storage->find<SourceTypeActionIndex>({source_type_ref.get_uuid(), host_action_ref.get_uuid()}) == m_storage->end())
            {
                return false;
            }
        }
        return true;
    }
    else if (auto shortcut_activity = boost::get<ShortcutActivity>(&activity))
    {
        if (m_storage->find<SourceTypeActionIndex>({source_type_ref.get_uuid(), shortcut_activity->m_host_action_ref.get_uuid()}) == m_storage->end())
        {
            return false;
        }
        return true;
    }

    return true;
}

void SourceTypeActivities::on_add_source_type(SourceTypeRef source_type_ref, Transaction& t)
{
    add_activities(*source_type_ref, t);    
}

void SourceTypeActivities::on_remove_source_type(SourceTypeUUID source_type_uuid, Transaction& t)
{
    transact_erase_range(*m_storage, m_storage->find_range<SourceTypeIndex>(source_type_uuid), t);
}

void SourceTypeActivities::on_add_selection(SelectionDirection direction, Transaction& t)
{
    auto selection = m_transformations->m_selections.find_selection(direction);
    auto selection_plan = selection->get_plan();

    SelectionActivity selection_activity(
        get_inputs(selection_plan),
        get_binding_inputs(selection_plan),
        selection_plan.m_output_ref,
        selection->get_uuid());

    for (auto& source_type : m_source_types->get_source_types())
    {
        if (validate_activity(source_type, selection_activity))
        {
            add_activity({source_type, selection_activity}, t);
        }
    }
}

void SourceTypeActivities::on_remove_selection(SelectionDirection direction, Transaction& t)
{
    SelectionIdentity selection_identity(get_uuids(direction.m_input_refs), direction.m_output_ref.get_uuid());
    for (auto& found : stl_tools::copy_vector(m_storage->find_range<ActivityIndex>(selection_identity)))
    {
        remove_activity(found.m_source_type_ref, selection_identity, t);
    }
}

void SourceTypeActivities::on_add_generator(UUID generator_uuid, Transaction& t)
{
    auto generator = m_transformations->m_generators.get_generator(generator_uuid);
    GeneratingActivity generating_activity(generator->get_report_type(), generator->get_uuid());
    add_activity({ generator->get_source_type(), generating_activity }, t);
}

void SourceTypeActivities::on_remove_generator(UUID generator_uuid, Transaction& t)
{
    GeneratingIdentity generating_identity(generator_uuid);
    for (auto& found : stl_tools::copy_vector(m_storage->find_range<ActivityIndex>(generating_identity)))
    {
        remove_activity(found.m_source_type_ref, generating_identity, t);
    }
}

void SourceTypeActivities::on_add_grouping(GroupingDirection direction, Transaction& t)
{
    auto grouping = m_transformations->m_groupings.find_grouping(direction);
    GroupingActivity grouping_activity(direction.m_input_ref, direction.m_output_ref, grouping->get_uuid());
    for (auto& source_type : m_source_types->get_source_types())
    {
        add_activity({source_type, grouping_activity}, t);
    }
}

void SourceTypeActivities::on_remove_grouping(ReportTypeRef input_ref, ReportTypeRef output_ref, Transaction& t)
{
    GroupingIdentity grouping_identity(input_ref.get_uuid(), output_ref.get_uuid());
    for (auto& found : stl_tools::copy_vector(m_storage->find_range<ActivityIndex>(grouping_identity)))
    {
        remove_activity(found.m_source_type_ref, grouping_identity, t);
    }
}

void SourceTypeActivities::on_add_convertion(ConvertionDirection direction, Transaction& t)
{
    ConvertionActivity convertion_activity(direction.m_input_ref, direction.m_output_ref);
    for (auto& source_type : m_source_types->get_source_types())
    {
        if (validate_activity(source_type, convertion_activity))
        {
            add_activity({source_type, convertion_activity}, t);
        }
    }
}

void SourceTypeActivities::on_remove_convertion(ConvertionDirection direction, Transaction& t)
{
    ConvertionIdentity convertion_identity(direction.m_input_ref.get_uuid(), direction.m_output_ref.get_uuid());
    for (auto& found : stl_tools::copy_vector(m_storage->find_range<ActivityIndex>(convertion_identity)))
    {
        remove_activity(found.m_source_type_ref, convertion_identity, t);
    }
}

void SourceTypeActivities::on_add_tracking(TrackingDirection direction, Transaction& t)
{
    TrackingActivity tracking_activity(direction.m_input_ref, direction.m_output_ref);
    for (auto& source_type : m_source_types->get_source_types())
    {
        if (validate_activity(source_type, tracking_activity))
        {
            add_activity({source_type, tracking_activity}, t);
        }
    }
}

void SourceTypeActivities::on_remove_tracking(TrackingDirection direction, Transaction& t)
{
    TrackingIdentity tracking_identity(direction.m_input_ref.get_uuid(), direction.m_output_ref.get_uuid());
    for (auto& found : stl_tools::copy_vector(m_storage->find_range<ActivityIndex>(tracking_identity)))
    {
        remove_activity(found.m_source_type_ref, tracking_identity, t);
    }
}

void SourceTypeActivities::on_add_action_delegate(const ActionDelegate& action_delegate, Transaction& t)
{
    for (auto& source_type : m_source_types->get_source_types())
    {
        ActionDelegateActivity action_delegate_activity(action_delegate.get_input_refs(), action_delegate.get_host_action_refs(), action_delegate.get_guest_action_ref());
        if (validate_activity(source_type, action_delegate_activity))
        {
            add_activity({source_type, action_delegate_activity}, t);
        }
    }
}

void SourceTypeActivities::on_remove_action_delegate(ActionDependency action_dependency, Transaction& t)
{
    auto host_action_uuids = stl_tools::copy_vector(action_dependency.m_host_action_refs | stl_tools::transformed(&ActionRef::get_uuid));
    ActionDelegateIdentity action_delegate_identity(std::move(host_action_uuids), action_dependency.m_guest_action_ref.get_uuid());

    for (auto found : stl_tools::copy_vector(m_storage->find_range<ActivityIndex>(action_delegate_identity)))
    {
        remove_activity(found.m_source_type_ref, action_delegate_identity, t);
    }
}

void SourceTypeActivities::on_add_action_shortcut(const ActionShortcut& action_shortcut, Transaction& t)
{
    for (auto& source_type : m_source_types->get_source_types())
    {
        ShortcutActivity shortcut_activity(action_shortcut, action_shortcut.get_action_ref(), action_shortcut.get_output_ref());
        if (validate_activity(source_type, shortcut_activity))
        {
            add_activity({source_type, shortcut_activity}, t);
        }
    }
}

void SourceTypeActivities::on_remove_action_shortcut(UUID shortcut_uuid, Transaction& t)
{
    ShortcutIdentity shortcut_identity(shortcut_uuid);
    for (auto& found : stl_tools::copy_vector(m_storage->find_range<ActivityIndex>(shortcut_identity)))
    {
        remove_activity(found.m_source_type_ref, shortcut_identity, t);
    }
}

}} //namespace TR { namespace Core {