#include "stdafx.h"
#include "AppRefs.h"
#include "Basis.h"
#include "Scheduler.h"
namespace TR { namespace Core {

Key ScheduleRef::get_key() const
{
    return m_schedule_key;
}

Schedule ScheduleRef::operator*() const
{
    return m_scheduler->get_schedule(m_schedule_key);
}

ScheduleRef::ScheduleRef(const Scheduler* scheduler, Key schedule_key):
    m_scheduler(scheduler), m_schedule_key(schedule_key)
{
}

bool operator == (const ScheduleRef& left, const ScheduleRef& right)
{
    return left.get_key() == right.get_key();
}

PeriodicOperation get_operation(Basis& basis, PeriodicOperationIdentity operation_identity)
{
    if (auto reload_operation = boost::get<const PeriodicReloadIdentity>(&operation_identity))
    {
        auto& report_type_ref = basis.m_report_types.get_report_type(reload_operation->m_report_type_uuid);
        return PeriodicReload(report_type_ref);
    }
    auto& launch_operation = boost::get<const PeriodicLaunchIdentity>(operation_identity);
    auto& shortcut_ref = basis.m_action_shortcuts.get_shortcut(launch_operation.m_action_shortcut_uuid);
    return PeriodicLaunch(shortcut_ref);
}

PeriodicOperationIdentity get_operation_identity(PeriodicOperation operation)
{
    if (auto periodic_reload = boost::get<const PeriodicReload>(&operation))
    {
        return PeriodicReloadIdentity(periodic_reload->m_report_type_ref.get_uuid());
    }
    
    auto& periodic_launch = boost::get<const PeriodicLaunch>(operation);
    return PeriodicLaunchIdentity(periodic_launch.m_shortcut_ref.get_uuid());
}

}} //namespace TR { namespace Core {