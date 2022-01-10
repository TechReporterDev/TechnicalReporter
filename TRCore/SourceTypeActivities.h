#pragma once
#include "BackgndFwd.h"
#include "BasisFwd.h"
#include "ServicesFwd.h"
#include "BasisRefs.h"
#include "Activity.h"
#include "stddecl.h"
namespace TR {namespace Core {
class SourceTypeActivityStorage;

struct SourceTypeActivity
{
    SourceTypeActivity(SourceTypeRef source_type_ref, Activity activity):
        m_source_type_ref(source_type_ref),
        m_activity(activity)
    {
    }

    SourceTypeRef m_source_type_ref;
    Activity m_activity;    
};

bool operator < (const SourceTypeActivity& left, const SourceTypeActivity& right);
bool operator == (const SourceTypeActivity& left, const SourceTypeActivity& right);

class SourceTypeActivities
{
public:
    static const Priority SOURCE_TYPE_ACTIVITIES_PRIORITY = 800;
    using ActivityRange = boost::any_range<const Activity, boost::forward_traversal_tag>;

    SourceTypeActivities();
    ~SourceTypeActivities();

    void                                set_source_types(SourceTypes* source_types);
    void                                set_report_types(ReportTypes* report_types);
    void                                set_transformations(Transformations* transformations);
    void                                set_action_delegates(ActionDelegates* action_delegates);
    void                                set_shortcuts(ActionShortcuts* action_shortcuts);
    void                                run(Executive* executive, Transaction& t);

    ActivityRange                       get_activities(SourceTypeRef source_type_ref) const;
    ActivityRange                       find_by_input(SourceTypeRef source_type_ref, ResourceRef input_ref) const;
    boost::optional<Activity>           find_by_output(SourceTypeRef source_type_ref, ResourceRef output_ref) const;
    boost::optional<Activity>           find_by_action(SourceTypeRef source_type_ref, ActionRef action_ref) const;
    boost::optional<Activity>           find_by_shortcut(SourceTypeRef source_type_ref, ActionShortcutRef shortcut_ref) const;

    std::vector<ReportTypeRef>          get_reloads(SourceTypeRef source_type_ref, ActionRef action_ref) const;

    template<class T>
    void connect_add_activity(T slot, Priority priority) { m_add_activity_sig.connect(slot, priority); }

    template<class T>
    void connect_remove_activity(T slot, Priority priority) { m_remove_activity_sig.connect(slot, priority); }

private:
    void                                add_activities(const SourceType& source_type, Transaction& t);
    void                                add_activity(SourceTypeActivity source_type_activity, Transaction& t);
    void                                remove_activity(SourceTypeRef source_type_ref, const ActivityIdentity& identity, Transaction& t);
    bool                                validate_activity(SourceTypeRef source_type_ref, const Activity& activity);

    void                                on_add_source_type(SourceTypeRef source_type_ref, Transaction& t);
    void                                on_remove_source_type(SourceTypeUUID source_type_uuid, Transaction& t);
    void                                on_add_selection(SelectionDirection direction, Transaction& t);
    void                                on_remove_selection(SelectionDirection direction, Transaction& t);
    void                                on_add_grouping(GroupingDirection direction, Transaction& t);
    void                                on_remove_grouping(ReportTypeRef input_ref, ReportTypeRef output_ref, Transaction& t);
    void                                on_add_convertion(ConvertionDirection direction, Transaction& t);
    void                                on_remove_convertion(ConvertionDirection direction, Transaction& t);
    void                                on_add_tracking(TrackingDirection direction, Transaction& t);
    void                                on_remove_tracking(TrackingDirection direction, Transaction& t);

    void                                on_add_action_delegate(const ActionDelegate& action_delegate, Transaction& t);
    void                                on_remove_action_delegate(ActionDependency action_dependency, Transaction& t);
    void                                on_add_action_shortcut(const ActionShortcut& action_shortcut, Transaction& t);
    void                                on_remove_action_shortcut(UUID shortcut_uuid, Transaction& t);

    SourceTypes* m_source_types;
    ReportTypes* m_report_types;
    Transformations* m_transformations;
    ActionDelegates* m_action_delegates;
    ActionShortcuts* m_action_shortcuts;

    OrderedSignal<void(const SourceTypeActivity&, Transaction&)> m_add_activity_sig;
    OrderedSignal<void(SourceTypeRef, ActivityIdentity, Transaction&)> m_remove_activity_sig;

    std::unique_ptr<SourceTypeActivityStorage> m_storage;
    bool m_running;
};

}} //namespace TR { namespace Core {