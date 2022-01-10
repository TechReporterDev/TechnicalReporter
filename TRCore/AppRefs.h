#pragma once
#include "AppFwd.h"
#include "AppTrivialTypes.h"
#include "BasisRefs.h"
#include "stddecl.h"
namespace TR { namespace Core {

class ScheduleRef
{
public:
    friend class Scheduler;

    Key             get_key() const;
    Schedule        operator*() const;

private:
    ScheduleRef(const Scheduler* scheduler, Key schedule_key);
    const Scheduler* m_scheduler;
    Key m_schedule_key;
};

bool operator == (const ScheduleRef& left, const ScheduleRef& right);

struct PeriodicReload
{
    PeriodicReload(ReportTypeRef report_type_ref):
        m_report_type_ref(report_type_ref)
    {
    }

    ReportTypeRef m_report_type_ref;
};

struct PeriodicLaunch
{
    PeriodicLaunch(ActionShortcutRef shortcut_ref):
        m_shortcut_ref(shortcut_ref)
    {
    }

    ActionShortcutRef m_shortcut_ref;
};

using PeriodicOperation = boost::variant<PeriodicReload, PeriodicLaunch>;
PeriodicOperation get_operation(Basis& basis, PeriodicOperationIdentity operation_identity);
PeriodicOperationIdentity get_operation_identity(PeriodicOperation operation);

}} // namespace TR { namespace Core {