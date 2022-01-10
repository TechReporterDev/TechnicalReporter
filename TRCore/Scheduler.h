#pragma once
#include "AppDecl.h"
namespace TR {  namespace Core {
class ScheduledTaskDataset;
struct ScheduleData;
static const int NEVER_SCHEDULE_KEY = 1;

class Schedule
{
public:
    friend class Scheduler;
    enum class Period { Never = 0, Hourly, Daily, Weekly, Mounthly, Often };

    Schedule(std::wstring name, Period period, int day, int week_day, time_t day_time);

    std::wstring    get_name() const;
    void            set_name(std::wstring name);
    Period          get_period() const;
    void            set_period(Period period);  
    int             get_day() const;
    void            set_day(int day);
    int             get_week_day() const;
    void            set_week_day(int week_day);
    time_t          get_day_time() const;
    void            set_day_time(time_t day_time);
    time_t          get_scheduled_time() const;

    ScheduleRef     get_ref() const;
    operator ScheduleRef() const;

private:
    Schedule(boost::optional<ScheduleRef> ref, std::wstring name, Period period, int day, int week_day, time_t day_time, time_t scheduled_time);
    
    boost::optional<ScheduleRef> m_ref;
    std::wstring m_name;
    Period m_period;
    int m_day;
    int m_week_day;
    time_t m_day_time;
    time_t m_scheduled_time;
};

struct ScheduledTaskResult
{
    ScheduledTaskResult():
        m_launch_time(0),
        m_result(true)
    {
    }

    ScheduledTaskResult(time_t launch_time, bool result, std::wstring message):
        m_launch_time(launch_time),
        m_result(result),
        m_message(std::move(message))
    {
    }

    time_t          m_launch_time;
    bool            m_result;
    std::wstring    m_message;
};

inline bool operator == (const ScheduledTaskResult& left, const ScheduledTaskResult& right)
{
    return left.m_launch_time == right.m_launch_time &&
        left.m_result == right.m_result &&
        left.m_message == right.m_message;
}

class Scheduler
{
public:
    static const Priority SCHEDULER_PRIORITY = 1900;

    static void                     install(Database& db);

    Scheduler(Database& db, CoreDomain& core_domain);
    ~Scheduler();
    void                            run(Executive* executive, Transaction& t);

    void                            add_schedule(Schedule& schedule, Transaction& t);
    void                            update_schedule(Schedule& schedule, Transaction& t);
    void                            remove_schedule(Key shedule_key, Transaction& t);
    std::vector<Schedule>           get_schedules() const;
    Schedule                        get_schedule(Key schedule_key) const;

    void                            set_task_schedule(SubjectRef subject_ref, PeriodicOperation operation, boost::optional<ScheduleRef> schedule_ref, Transaction& t);
    boost::optional<ScheduleRef>    get_task_schedule(SubjectRef subject_ref, PeriodicOperation operation) const;
    ScheduleRef                     get_effective_task_schedule(SubjectRef subject_ref, PeriodicOperation operation) const;
    ScheduledTaskResult             get_task_result(SourceRef source_ref, PeriodicOperation operation) const;
        
    template<class T>
    void connect_update_task_result(T slot, Priority priority) { m_update_task_result_sig.connect(slot, priority); }

private:
    friend class ScheduledTask;
    class TaskManager;

    void                            check_scheduled_time();
    void                            shedule_checking_loop();
    void                            start_scheduled_tasks(ScheduleRef schedule, time_t schedule_time);

    void                            set_task_result(SourceRef source_ref, PeriodicOperation operation, const ScheduledTaskResult& task_result, Transaction& t);
    void                            reset_task_result(SourceRef source_ref, PeriodicOperation operation, Transaction& t);
    void                            forward_task_schedule(SubjectRef subject_ref, PeriodicOperation operation, Transaction& t);
        
    ScheduleData                    pack_schedule(const Schedule& schedule) const;
    Schedule                        unpack_schedule(const ScheduleData& schedule_data) const;

    void                            on_remove_report_type(ReportTypeUUID report_type_uuid, Transaction& t);
    void                            on_remove_shortcut(UUID shortcut_uuid, Transaction& t);
    void                            on_remove_source(const std::vector<SourceKey>& source_keys, Transaction& t);
    void                            on_remove_role(RoleKey role_key, Transaction& t);
    
    OrderedSignal<void(SourceRef source_ref, PeriodicOperation operation, Transaction& t)> m_update_task_result_sig;
    
    CoreDomain& m_core_domain;
    Database& m_db; 
    ReportTypes& m_report_types;
    ActionShortcuts& m_shortcuts;
    SourceTypeActivities& m_source_type_activities;
    Sources& m_sources;
    Roles& m_roles; 
    SourceResources& m_source_resources;
    Executive* m_executive;

    std::unique_ptr<ScheduledTaskDataset> m_scheduled_task_dataset;
    std::unique_ptr<TaskManager> m_task_manager;
};

}} //namespace TR { namespace Core {