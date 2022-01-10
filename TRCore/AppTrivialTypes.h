#pragma once
#include <boost\variant.hpp>
#include "BasisTrivialTypes.h"
namespace TR { namespace Core {

struct PeriodicReloadIdentity
{
    PeriodicReloadIdentity()
    {
    }

    PeriodicReloadIdentity(ReportTypeUUID report_type_uuid):
        m_report_type_uuid(report_type_uuid)
    {
    }

    static PeriodicReloadIdentity create_invalid()
    {
        return PeriodicReloadIdentity(ReportTypeUUID::create_invalid());
    }

    ReportTypeUUID m_report_type_uuid;
};

inline bool operator == (const PeriodicReloadIdentity& left, const PeriodicReloadIdentity& right)
{
    return left.m_report_type_uuid == right.m_report_type_uuid;
}

inline bool operator != (const PeriodicReloadIdentity& left, const PeriodicReloadIdentity& right)
{
    return !(left == right);
}

inline bool operator < (const PeriodicReloadIdentity& left, const PeriodicReloadIdentity& right)
{
    return left.m_report_type_uuid < right.m_report_type_uuid;
}

struct PeriodicLaunchIdentity
{
    PeriodicLaunchIdentity():
        m_action_shortcut_uuid(stl_tools::null_uuid())
    {
    }

    PeriodicLaunchIdentity(UUID action_shortcut_uuid):
        m_action_shortcut_uuid(action_shortcut_uuid)
    {
    }

    static PeriodicLaunchIdentity create_invalid()
    {
        return PeriodicLaunchIdentity(INVALID_UUID);
    }

    UUID m_action_shortcut_uuid;
};

inline bool operator == (const PeriodicLaunchIdentity& left, const PeriodicLaunchIdentity& right)
{
    return left.m_action_shortcut_uuid == right.m_action_shortcut_uuid;
}

inline bool operator != (const PeriodicLaunchIdentity& left, const PeriodicLaunchIdentity& right)
{
    return !(left == right);
}

inline bool operator < (const PeriodicLaunchIdentity& left, const PeriodicLaunchIdentity& right)
{
    return left.m_action_shortcut_uuid < right.m_action_shortcut_uuid;
}

using PeriodicOperationIdentity = boost::variant<PeriodicReloadIdentity, PeriodicLaunchIdentity>;

}} //namespace TR { namespace Core {