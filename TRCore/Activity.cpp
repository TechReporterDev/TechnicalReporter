#include "stdafx.h"
#include "Activity.h"
namespace TR { namespace Core {

namespace {
struct get_activity_identity: boost::static_visitor<ActivityIdentity>
{
    template<class SomeActivity>
    ActivityIdentity operator()(const SomeActivity& activity) const
    {
        return activity.get_identity();
    }
};

struct get_activity_input_refs: boost::static_visitor<std::vector<ResourceRef>>
{
    std::vector<ResourceRef> operator()(const SelectionActivity& selection_activity) const
    {
        return std::vector<ResourceRef>(selection_activity.m_input_refs.begin(), selection_activity.m_input_refs.end());
    }

    std::vector<ResourceRef> operator()(const ConvertionActivity& convertion_activity) const
    {
        return {convertion_activity.m_input_ref};
    }

    std::vector<ResourceRef> operator()(const TrackingActivity& tracking_activity) const
    {
        return {tracking_activity.m_input_ref};
    }

    std::vector<ResourceRef> operator()(const GroupingActivity& grouping_activity) const
    {
        return {grouping_activity.m_input_ref};
    }

    std::vector<ResourceRef> operator()(const ActionDelegateActivity& action_delegate_activity) const
    {
        return std::vector<ResourceRef>(action_delegate_activity.m_input_refs.begin(), action_delegate_activity.m_input_refs.end());
    }

    template<class SomeActivity>
    std::vector<ResourceRef> operator()(const SomeActivity& activity) const
    {
        return {};
    }
};

struct get_activity_output_ref: boost::static_visitor<boost::optional<ResourceRef>>
{
    boost::optional<ResourceRef> operator()(const LoadingActivity& loading_activity) const
    {
        return loading_activity.m_output_ref;
    }

    boost::optional<ResourceRef> operator()(const SelectionActivity& selection_activity) const
    {
        return selection_activity.m_output_ref;
    }

    boost::optional<ResourceRef> operator()(const ConvertionActivity& convertion_activity) const
    {
        return convertion_activity.m_output_ref;
    }

    boost::optional<ResourceRef> operator()(const TrackingActivity& tracking_activity) const
    {
        return tracking_activity.m_output_ref;
    }

    boost::optional<ResourceRef> operator()(const InflowActivity& inflow_activity) const
    {
        return inflow_activity.m_stream_type_ref;
    }

    boost::optional<ResourceRef> operator()(const GeneratingActivity& generating_activity) const
    {
        return generating_activity.m_output_ref;
    }

    boost::optional<ResourceRef> operator()(const GroupingActivity& grouping_activity) const
    {
        return grouping_activity.m_output_ref;
    }

    boost::optional<ResourceRef> operator()(const ShortcutActivity& shortcut_activity) const
    {
        if (shortcut_activity.m_output_ref)
        {
            return *shortcut_activity.m_output_ref;
        }
        return boost::none;
    }
    
    template<class SomeActivity>
    boost::optional<ResourceRef> operator()(const SomeActivity& activity) const
    {
        return boost::none;
    }
};

struct get_activity_action_ref: boost::static_visitor<boost::optional<ActionRef>>
{
    boost::optional<ActionRef> operator()(const ActionActivity& action_activity) const
    {
        return action_activity.m_action_ref;
    }

    boost::optional<ActionRef> operator()(const ActionDelegateActivity& action_delegate_activity) const
    {
        return action_delegate_activity.m_guest_action_ref;
    }

    template<class SomeActivity>
    boost::optional<ActionRef> operator()(const SomeActivity& activity) const
    {
        return boost::none;
    }
};

struct get_activity_host_action_refs: boost::static_visitor<std::vector<ActionRef>>
{
    std::vector<ActionRef> operator()(const ActionDelegateActivity& action_delegate_activity) const
    {
        return action_delegate_activity.m_host_action_refs;
    }

    std::vector<ActionRef> operator()(const ShortcutActivity& shortcut_activity) const
    {
        return { shortcut_activity.m_host_action_ref };
    }

    template<class SomeActivity>
    std::vector<ActionRef> operator()(const SomeActivity& activity) const
    {
        return {};
    }
};

struct get_activity_uuid : boost::static_visitor<UUID>
{
    UUID operator()(const SelectionActivity& selection_activity) const
    {
        return selection_activity.m_uuid;
    }

    UUID operator()(const GeneratingActivity& generating_activity) const
    {
        return generating_activity.m_uuid;
    }

    UUID operator()(const GroupingActivity& grouping_activity) const
    {
        return grouping_activity.m_uuid;
    }

    template<class SomeActivity>
    UUID operator()(const SomeActivity& activity) const
    {
        return stl_tools::null_uuid();
    }
};

} //namespace {

ActivityIdentity get_identity(Activity activity)
{
    return boost::apply_visitor(get_activity_identity(), activity);
}

std::vector<ResourceRef> get_input_refs(Activity activity)
{
    return boost::apply_visitor(get_activity_input_refs(), activity);
}

boost::optional<ResourceRef> get_output_ref(Activity activity)
{
    return boost::apply_visitor(get_activity_output_ref(), activity);
}

boost::optional<ActionRef> get_action_ref(Activity activity)
{
    return boost::apply_visitor(get_activity_action_ref(), activity);
}

std::vector<ActionRef> get_host_action_refs(Activity activity)
{
    return boost::apply_visitor(get_activity_host_action_refs(), activity);
}

UUID get_uuid(Activity activity)
{
    return boost::apply_visitor(get_activity_uuid(), activity);
}

}} //namespace TR { namespace Core {