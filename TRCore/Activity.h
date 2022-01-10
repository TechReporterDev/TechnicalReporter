#pragma once
#include "BackgndFwd.h"
#include "BasisFwd.h"
#include "BasisRefs.h"
#include "stddecl.h"
#include "boost/operators.hpp"
namespace TR { namespace Core {

struct LoadingIdentity: boost::equivalent<LoadingIdentity>
{
    LoadingIdentity(ReportTypeUUID output_uuid):
        m_output_uuid(output_uuid)
    {
    }

    ReportTypeUUID m_output_uuid;
};

inline bool operator < (const LoadingIdentity& left, const LoadingIdentity& right)
{
    return left.m_output_uuid < right.m_output_uuid;
}

struct LoadingActivity: boost::equivalent<LoadingActivity>
{
    LoadingActivity(ReportTypeRef output_ref):
        m_output_ref(output_ref)
    {
    }

    LoadingIdentity get_identity() const
    {
        return LoadingIdentity(m_output_ref.get_uuid());
    }

    ReportTypeRef m_output_ref;
};

inline bool operator < (const LoadingActivity& left, const LoadingActivity& right)
{
    return left.get_identity() < right.get_identity();
}

struct SelectionIdentity: boost::equivalent<SelectionIdentity>
{
    SelectionIdentity(std::vector<ReportTypeUUID> input_uuids, ReportTypeUUID output_uuid):
        m_input_uuids(std::move(input_uuids)),
        m_output_uuid(output_uuid)
    {
    }

    std::vector<ReportTypeUUID> m_input_uuids;
    ReportTypeUUID m_output_uuid;
};

inline bool operator < (const SelectionIdentity& left, const SelectionIdentity& right)
{
    if (left.m_input_uuids < right.m_input_uuids)
    {
        return true;
    }

    if (right.m_input_uuids < left.m_input_uuids)
    {
        return false;
    }

    return left.m_output_uuid < right.m_output_uuid;
}

struct SelectionActivity: boost::equivalent<SelectionActivity>
{
    SelectionActivity(std::vector<ReportTypeRef> input_refs, std::vector<ReportTypeRef> binding_input_refs, ReportTypeRef output_ref, UUID uuid):
        m_input_refs(std::move(input_refs)),
        m_binding_input_refs(std::move(binding_input_refs)),
        m_output_ref(output_ref),
        m_uuid(uuid)
    {
    }

    SelectionActivity(std::vector<ReportTypeRef> input_refs, ReportTypeRef output_ref, UUID uuid) :
        SelectionActivity(input_refs, input_refs, output_ref, uuid)
    {
    }

    SelectionIdentity get_identity() const
    {
        return SelectionIdentity(get_uuids(m_input_refs), m_output_ref.get_uuid());
    }

    std::vector<ReportTypeRef> m_input_refs;
    std::vector<ReportTypeRef> m_binding_input_refs;
    ReportTypeRef m_output_ref;
    UUID m_uuid;
};

inline bool operator < (const SelectionActivity& left, const SelectionActivity& right)
{
    return left.m_uuid < right.m_uuid;
}

struct GroupingIdentity: boost::equivalent<GroupingIdentity>
{
    GroupingIdentity(ReportTypeUUID input_uuid, ReportTypeUUID output_uuid):
        m_input_uuid(input_uuid),
        m_output_uuid(output_uuid)
    {
    }

    ReportTypeUUID m_input_uuid;
    ReportTypeUUID m_output_uuid;
};

inline bool operator < (const GroupingIdentity& left, const GroupingIdentity& right)
{
    if (left.m_input_uuid < right.m_input_uuid)
    {
        return true;
    }

    if (right.m_input_uuid < left.m_input_uuid)
    {
        return false;
    }

    return left.m_output_uuid < right.m_output_uuid;
}

struct GroupingActivity: boost::equivalent<GroupingActivity>
{
    GroupingActivity(ReportTypeRef input_refs, ReportTypeRef output_ref, UUID uuid):
        m_input_ref(input_refs),
        m_output_ref(output_ref),
        m_uuid(uuid)
    {
    }

    GroupingIdentity get_identity() const
    {
        return GroupingIdentity(m_input_ref.get_uuid(), m_output_ref.get_uuid());
    }

    ReportTypeRef m_input_ref;
    ReportTypeRef m_output_ref;
    UUID m_uuid;
};

inline bool operator < (const GroupingActivity& left, const GroupingActivity& right)
{
    return left.get_identity() < right.get_identity();
}

struct ConvertionIdentity: boost::equivalent<ConvertionIdentity>
{
    ConvertionIdentity(StreamTypeUUID input_uuid, StreamTypeUUID output_uuid):
        m_input_uuid(input_uuid),
        m_output_uuid(output_uuid)
    {
    }

    StreamTypeUUID m_input_uuid;
    StreamTypeUUID m_output_uuid;
};

inline bool operator < (const ConvertionIdentity& left, const ConvertionIdentity& right)
{
    if (left.m_input_uuid < right.m_input_uuid)
    {
        return true;
    }

    if (right.m_input_uuid < left.m_input_uuid)
    {
        return false;
    }

    return left.m_output_uuid < right.m_output_uuid;
}

struct ConvertionActivity: boost::equivalent<ConvertionActivity>
{
    ConvertionActivity(StreamTypeRef input_ref, StreamTypeRef output_ref):
        m_input_ref(input_ref),
        m_output_ref(output_ref)
    {
    }

    ConvertionIdentity get_identity() const
    {
        return ConvertionIdentity(m_input_ref.get_uuid(), m_output_ref.get_uuid());
    }

    StreamTypeRef m_input_ref;
    StreamTypeRef m_output_ref;
};

inline bool operator < (const ConvertionActivity& left, const ConvertionActivity& right)
{
    return left.get_identity() < right.get_identity();
}

struct TrackingIdentity: boost::equivalent<TrackingIdentity>
{
    TrackingIdentity(ReportTypeUUID input_uuid, StreamTypeUUID output_uuid):
        m_input_uuid(input_uuid),
        m_output_uuid(output_uuid)
    {
    }

    ReportTypeUUID m_input_uuid;
    StreamTypeUUID m_output_uuid;
};

inline bool operator < (const TrackingIdentity& left, const TrackingIdentity& right)
{
    if (left.m_input_uuid < right.m_input_uuid)
    {
        return true;
    }

    if (right.m_input_uuid < left.m_input_uuid)
    {
        return false;
    }

    return left.m_output_uuid < right.m_output_uuid;
}

struct TrackingActivity: boost::equivalent<TrackingActivity>
{
    TrackingActivity(ReportTypeRef input_ref, StreamTypeRef output_ref):
        m_input_ref(input_ref),
        m_output_ref(output_ref)
    {
    }

    TrackingIdentity get_identity() const
    {
        return TrackingIdentity(m_input_ref.get_uuid(), m_output_ref.get_uuid());
    }

    ReportTypeRef m_input_ref;
    StreamTypeRef m_output_ref;
};

inline bool operator < (const TrackingActivity& left, const TrackingActivity& right)
{
    return left.get_identity() < right.get_identity();
}

struct ActionIdentity: boost::equivalent<ActionIdentity>
{
    ActionIdentity(ActionUUID action_uuid):
        m_action_uuid(action_uuid)
    {
    }

    ActionUUID m_action_uuid;
};

inline bool operator < (const ActionIdentity& left, const ActionIdentity& right)
{
    return left.m_action_uuid < right.m_action_uuid;
}

struct ActionActivity: boost::equivalent<ActionActivity>
{
    ActionActivity(ActionRef action_ref):
        m_action_ref(action_ref)
    {
    }

    ActionIdentity get_identity() const
    {
        return m_action_ref.get_uuid();
    }

    ActionRef m_action_ref;
};

inline bool operator < (const ActionActivity& left, const ActionActivity& right)
{
    return left.get_identity() < right.get_identity();
}

struct ActionDelegateIdentity: boost::equivalent<ActionDelegateIdentity>
{
    ActionDelegateIdentity(std::vector<ActionUUID> host_action_uuids, ActionUUID guest_action_uuid):
        m_host_action_uuids(std::move(host_action_uuids)),
        m_guest_action_uuid(guest_action_uuid)
    {
    }

    std::vector<ActionUUID> m_host_action_uuids;
    ActionUUID m_guest_action_uuid;
};

inline bool operator < (const ActionDelegateIdentity& left, const ActionDelegateIdentity& right)
{
    if (left.m_host_action_uuids < right.m_host_action_uuids)
    {
        return true;
    }

    if (left.m_host_action_uuids > right.m_host_action_uuids)
    {
        return false;
    }

    return left.m_guest_action_uuid < right.m_guest_action_uuid;
}

struct ActionDelegateActivity: boost::equivalent<ActionDelegateActivity>
{
    ActionDelegateActivity(std::vector<ReportTypeRef> input_refs, std::vector<ActionRef> host_action_refs, ActionRef guest_action_ref):
        m_input_refs(std::move(input_refs)),
        m_host_action_refs(std::move(host_action_refs)),
        m_guest_action_ref(guest_action_ref)        
    {
    }

    ActionDelegateIdentity get_identity() const
    {
        std::vector<ActionUUID> host_action_uuids;
        for (auto& host_action_ref : m_host_action_refs)
        {
            host_action_uuids.push_back(host_action_ref.get_uuid());
        }
        return ActionDelegateIdentity(std::move(host_action_uuids), m_guest_action_ref.get_uuid());
    }

    std::vector<ReportTypeRef> m_input_refs;
    std::vector<ActionRef> m_host_action_refs;
    ActionRef m_guest_action_ref;   
};

inline bool operator < (const ActionDelegateActivity& left, const ActionDelegateActivity& right)
{
    return left.get_identity() < right.get_identity();
}

struct ShortcutIdentity: boost::equivalent<ShortcutIdentity>
{
    ShortcutIdentity(UUID shortcut_uuid):
        m_shortcut_uuid(shortcut_uuid)
    {
    }

    UUID m_shortcut_uuid;
};

inline bool operator < (const ShortcutIdentity& left, const ShortcutIdentity& right)
{
    return left.m_shortcut_uuid < right.m_shortcut_uuid;
}

struct ShortcutActivity: boost::equivalent<ShortcutActivity>
{
    ShortcutActivity(ActionShortcutRef shortcut_ref, ActionRef host_action_ref, boost::optional<ReportTypeRef> output_ref):
        m_shortcut_ref(shortcut_ref),
        m_host_action_ref(host_action_ref),
        m_output_ref(output_ref)
    {
    }

    ShortcutIdentity get_identity() const
    {
        return ShortcutIdentity(m_shortcut_ref.get_uuid());
    }

    ActionShortcutRef m_shortcut_ref;
    ActionRef m_host_action_ref;
    boost::optional<ReportTypeRef> m_output_ref;
};

inline bool operator < (const ShortcutActivity& left, const ShortcutActivity& right)
{
    return left.get_identity() < right.get_identity();
}

struct InflowIdentity: boost::equivalent<InflowIdentity>
{
    InflowIdentity(StreamTypeUUID stream_type_uuid):
        m_stream_type_uuid(stream_type_uuid)
    {
    }

    StreamTypeUUID m_stream_type_uuid;
};

inline bool operator < (const InflowIdentity& left, const InflowIdentity& right)
{
    return left.m_stream_type_uuid < right.m_stream_type_uuid;
}

struct InflowActivity: boost::equivalent<InflowActivity>
{
    InflowActivity(StreamTypeRef stream_type_ref):
        m_stream_type_ref(stream_type_ref)
    {
    }

    InflowIdentity get_identity() const
    {
        return InflowIdentity(m_stream_type_ref.get_uuid());
    }

    StreamTypeRef m_stream_type_ref;
};

inline bool operator < (const InflowActivity& left, const InflowActivity& right)
{
    return left.get_identity() < right.get_identity();
}

using ActivityIdentity = boost::variant<
    LoadingIdentity,
    SelectionIdentity,
    GroupingIdentity,
    ConvertionIdentity,
    TrackingIdentity,
    ActionIdentity,
    ActionDelegateIdentity,
    ShortcutIdentity,
    InflowIdentity>;

using Activity = boost::variant<
    LoadingActivity, 
    SelectionActivity, 
    GroupingActivity,
    ConvertionActivity,
    TrackingActivity,
    ActionActivity, 
    ActionDelegateActivity, 
    ShortcutActivity,
    InflowActivity>;

ActivityIdentity                    get_identity(Activity activity);
std::vector<ResourceRef>            get_input_refs(Activity activity);
boost::optional<ResourceRef>        get_output_ref(Activity activity);
boost::optional<ActionRef>          get_action_ref(Activity activity);
std::vector<ActionRef>              get_host_action_refs(Activity activity);
UUID                                get_uuid(Activity activity);

}} //namespace TR { namespace Core {