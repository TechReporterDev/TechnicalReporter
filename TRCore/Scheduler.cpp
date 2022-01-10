#include "stdafx.h"
#include "Scheduler.h"
#include "CoreDomain.h"
#include "JobManager.h"
#include "Transaction.h"
#include "Tasks.h"
#include "ServicesPacking.h"
#include "ScheduleDataset.h"
#include "TimeTools.h"
#include <limits>
namespace TR { namespace Core {
namespace {

time_t get_day_time(struct tm& tm)
{
    return tm.tm_hour * ONE_HOUR + tm.tm_min * ONE_MIN + tm.tm_sec;
}

void add_month(struct tm& tm)
{
    if (tm.tm_mon == 11)
    {
        tm.tm_mon = 0;
        ++tm.tm_year;
    }
    else
    {
        ++tm.tm_mon;
    }
}

time_t next_scheduled_time(const Schedule& schedule)
{
    const auto current = time(nullptr);
    tm now = { 0 };
    gmtime_s(&now, &current);

    switch (schedule.get_period())
    {
    case Schedule::Period::Never:
        return std::numeric_limits<time_t>::max();

    case Schedule::Period::Hourly:
    {
        return _mkgmtime(&now) - now.tm_min * ONE_MIN - now.tm_sec + ONE_HOUR;
    }

    case Schedule::Period::Daily:
    {
        auto sheduled_time = _mkgmtime(&now) - get_day_time(now) + schedule.get_day_time();
        if (get_day_time(now) > schedule.get_day_time())
        {
            sheduled_time += ONE_DAY;
        }
        return sheduled_time;
    }

    case Schedule::Period::Weekly:
    {
        auto sheduled_time = _mkgmtime(&now) - get_day_time(now) + schedule.get_day_time();
        if (now.tm_wday < schedule.get_week_day())
        {
            sheduled_time += (schedule.get_week_day() - int(now.tm_wday)) * ONE_DAY;
        }
        else if (now.tm_wday > schedule.get_week_day())
        {
            sheduled_time += ONE_WEEK - (int(now.tm_wday) - schedule.get_week_day()) * ONE_DAY;
        }
        else if (now.tm_wday == schedule.get_week_day() && get_day_time(now) > schedule.get_day_time())
        {
            sheduled_time += ONE_WEEK;
        }
        return sheduled_time;
    }

    case Schedule::Period::Mounthly:
    {
        auto tm = now;
        if (now.tm_mday > schedule.get_day() || 
            ((now.tm_mday == schedule.get_day() || boost::gregorian::gregorian_calendar::end_of_month_day(tm.tm_year, tm.tm_mon)) && get_day_time(now) > schedule.get_day_time()))
        {
            add_month(tm);
        }
        tm.tm_mday = std::min<int>(boost::gregorian::gregorian_calendar::end_of_month_day(tm.tm_year, tm.tm_mon), schedule.get_day());
        return _mkgmtime(&now) - get_day_time(now) + schedule.get_day_time();
    }

    case Schedule::Period::Often:
    {
        return current;
    }

    default:
        _ASSERT(false);
    }
    return 0;   
}
} //namespace {

Schedule::Schedule(std::wstring name, Period period, int day, int week_day, time_t day_time):
    Schedule(boost::none, std::move(name), period, day, week_day, day_time, -1)
{
}

std::wstring Schedule::get_name() const
{
    return m_name;
}
void Schedule::set_name(std::wstring name)
{
    m_name = std::move(name);
}

Schedule::Period Schedule::get_period() const
{
    return m_period;
}

void Schedule::set_period(Period period)
{
    m_period = period;
}

int Schedule::get_day() const
{
    return m_day;
}

void Schedule::set_day(int day)
{
    m_day = day;
}

int Schedule::get_week_day() const
{
    return m_week_day;
}

void Schedule::set_week_day(int week_day)
{
    m_week_day = week_day;
}

time_t Schedule::get_day_time() const
{
    return m_day_time;
}

void Schedule::set_day_time(time_t day_time)
{
    m_day_time = day_time;
}

time_t Schedule::get_scheduled_time() const
{
    return m_scheduled_time;
}

ScheduleRef Schedule::get_ref() const
{
    _ASSERT(m_ref);
    if (!m_ref)
    {
        throw Exception(L"Schedule is not persist");
    }
    return *m_ref;
}

Schedule::operator ScheduleRef() const
{
    return get_ref();
}

Schedule::Schedule(boost::optional<ScheduleRef> ref, std::wstring name, Period period, int day, int week_day, time_t day_time, time_t scheduled_time):
    m_ref(ref),
    m_name(std::move(name)),
    m_period(period),
    m_day(day),
    m_week_day(week_day),
    m_day_time(day_time),
    m_scheduled_time(scheduled_time)
{
}

using ScheduleJobManager = JobManager<std::pair<SourceKey, PeriodicOperationIdentity>>;

class ScheduledTask: public Task
{
public:
    class ControlTask : public Job
    {
    public:
        ControlTask(ScheduledTask* sheduled_task):
            m_sheduled_task(sheduled_task)
        {
        }

        virtual void do_run() override
        {
            m_sheduled_task->run_operation();
        }

        using Job::emit_completed;
        using Job::emit_failed;

    private:
        ScheduledTask* m_sheduled_task;
    };

    ScheduledTask(Scheduler& scheduler, ScheduleJobManager& control_manager, SourceRef source_ref, PeriodicOperation operation, time_t schedule_time):
        Task(*scheduler.m_executive),
        m_scheduler(scheduler),
        m_core_domain(scheduler.m_core_domain),
        m_source_resources(scheduler.m_core_domain.m_registry.m_source_resources),
        m_control_manager(control_manager),
        m_control_task(std::make_shared<ControlTask>(this)),
        m_source_key(source_ref.get_key()),
        m_operation_identity(get_operation_identity(operation)),
        m_schedule_time(schedule_time)
    {        
    }

protected:
    // Job override
    virtual void do_run() override
    {
        m_executive.async([this] {
            m_control_manager.run_job(
                std::make_pair(m_source_key, m_operation_identity),
                m_control_task);
        });
    }

    void run_operation()
    {
        auto source = m_core_domain.m_registry.m_sources.get_source(m_source_key);
        auto operation = get_operation(m_core_domain.m_basis, m_operation_identity);

        auto result = m_scheduler.get_task_result(source, operation);        
        if (result.m_launch_time > m_schedule_time)
        {
            if (result.m_result)
            {
                on_completed();
            }
            else
            {
                on_failed(Exception(result.m_message));
            }
            return;
        }

        m_subtasks = std::make_shared<Batch>();
        if (auto reload_operation = boost::get<const PeriodicReload>(&operation))
        {           
            auto source_resource = m_source_resources.get_source_resource(source, reload_operation->m_report_type_ref);
            if (auto loading_activity = boost::get<LoadingActivity>(&source_resource.m_activity))
            {
                m_subtasks->add_job(std::make_shared<ReloadReportTask>(m_core_domain, m_source_key, loading_activity->m_output_ref.get_uuid(), m_executive));
            }
            else if(auto shortcut_activity = boost::get<ShortcutActivity>(&source_resource.m_activity))
            {
                m_subtasks->add_job(std::make_shared<ScheduledTask>(m_scheduler, m_control_manager, source, PeriodicLaunch(shortcut_activity->m_shortcut_ref), m_schedule_time));
            }
            else
            {
                for (auto input : m_source_resources.get_inputs(source_resource))
                {
                    m_subtasks->add_job(std::make_shared<ScheduledTask>(
                        m_scheduler,
                        m_control_manager,
                        input.m_source_ref, 
                        PeriodicReload(boost::get<ReportTypeRef>(input.m_resource_ref)), 
                        m_schedule_time)
                    );
                }
            }
        }
        else
        {
            auto& launch_operation = boost::get<const PeriodicLaunch>(operation);
            m_subtasks->add_job(std::make_shared<LaunchShortcutTask>(
                m_core_domain, 
                m_source_key, 
                launch_operation.m_shortcut_ref.get_uuid(), 
                m_executive));
        }

        m_subtasks->run(
            [this]{
                m_subtasks.reset();
                save_task_result({time(nullptr), true, L""});   
            }, 
                
            [this](const std::exception& err){
                m_subtasks.reset();
                save_task_result({time(nullptr), false, stl_tools::utf8_to_ucs(err.what())});
            }
        );
    }

private:
    void save_task_result(const ScheduledTaskResult& scheduled_task_result)
    {
        try
        {
            Transaction t(m_scheduler.m_db);
            auto source = m_core_domain.m_registry.m_sources.get_source(m_source_key);          
            auto operation = get_operation(m_core_domain.m_basis, m_operation_identity);
            
            if (m_scheduler.get_effective_task_schedule(source, operation).get_key() == NEVER_SCHEDULE_KEY)
            {
                throw std::logic_error("Operation is unavailable for NEVER_SCHEDULE");
            }

            m_scheduler.set_task_result(source, operation, scheduled_task_result, t);
            t.commit();

            on_completed();
        }
        catch (std::exception& err)
        {
            // log error
            on_failed(err);
        }
    }

    void on_completed()
    {
        m_control_task->emit_completed();
        emit_completed();
    }

    void on_failed(std::exception& err)
    {
        m_control_task->emit_failed(err);
        emit_failed(err);
    }

    Scheduler& m_scheduler;
    CoreDomain& m_core_domain;
    SourceResources& m_source_resources;

    ScheduleJobManager& m_control_manager;
    std::shared_ptr<ControlTask> m_control_task;

    SourceKey m_source_key;
    PeriodicOperationIdentity m_operation_identity;
    time_t m_schedule_time;
};

class Scheduler::TaskManager
{
public:
    TaskManager(Scheduler& scheduler, Executive& executive):
        m_scheduler(scheduler),
        m_executive(executive)
    {
    }       

    void start_scheduled_task(SourceRef source_ref, PeriodicOperation operation, time_t schedule_time)
    {
        m_job_manager.run_job(
            std::make_pair(source_ref.get_key(), get_operation_identity(operation)), 
            std::make_shared<ScheduledTask>(m_scheduler, m_control_manager, source_ref, operation, schedule_time));
    }

private:
    Scheduler& m_scheduler;
    Executive& m_executive;

    JobManager<std::pair<SourceKey, PeriodicOperationIdentity>> m_job_manager;
    JobManager<std::pair<SourceKey, PeriodicOperationIdentity>> m_control_manager;
};

void Scheduler::install(Database& db)
{
    ScheduleData never_schedule(0, "Never", int(Schedule::Period::Never), 0, 0, 0, std::numeric_limits<time_t>::max());
    Key never_schedule_key = db.persist(never_schedule);
    _ASSERT(never_schedule_key == NEVER_SCHEDULE_KEY);
}

Scheduler::Scheduler(Database& db, CoreDomain& core_domain):
    m_db(db),
    m_core_domain(core_domain),
    m_report_types(core_domain.m_basis.m_report_types),
    m_shortcuts(core_domain.m_basis.m_action_shortcuts),
    m_source_type_activities(core_domain.m_basis.m_source_type_activities),
    m_sources(core_domain.m_registry.m_sources),
    m_roles(core_domain.m_registry.m_roles),    
    m_source_resources(core_domain.m_registry.m_source_resources),
    m_executive(nullptr),
    m_scheduled_task_dataset(std::make_unique<ScheduledTaskDataset>(m_db))
{
    m_report_types.connect_remove_report_type([this](ReportTypeUUID report_type_uuid, Transaction& t){
        on_remove_report_type(report_type_uuid, t);
    }, SCHEDULER_PRIORITY);

    m_shortcuts.connect_remove_shortcut([this](UUID shortcut_uuid, Transaction& t){
        on_remove_shortcut(shortcut_uuid, t);
    }, SCHEDULER_PRIORITY);

    m_sources.connect_remove_source([this](SourceKey source_key, const std::vector<SourceKey>& removed_sources, Transaction& t){
        on_remove_source(removed_sources, t);
    }, SCHEDULER_PRIORITY);

    m_roles.connect_remove_role([this](RoleKey role_key, Transaction& t){
        on_remove_role(role_key, t);
    }, SCHEDULER_PRIORITY); 
}

Scheduler::~Scheduler()
{
}

void Scheduler::run(Executive* executive, Transaction& t)
{
    _ASSERT(!m_executive);
    transact_assign(m_executive, executive, t);
    transact_assign(m_task_manager, std::make_unique<TaskManager>(*this, *executive), t);

    t.connect_commit([this]()
    {
        shedule_checking_loop();
    });
}

void Scheduler::add_schedule(Schedule& schedule, Transaction& t)
{
    schedule.m_scheduled_time = next_scheduled_time(schedule);
    auto key = odb::persist(m_db, pack_schedule(schedule));
    schedule.m_ref = ScheduleRef(this, key);
}

void Scheduler::update_schedule(Schedule& schedule, Transaction& t)
{
    if (schedule.get_ref().get_key() == NEVER_SCHEDULE_KEY)
    {
        throw Exception(L"Can`t update 'never' schedule");
    }

    schedule.m_scheduled_time = next_scheduled_time(schedule);
    m_db.update(pack_schedule(schedule));
}

void Scheduler::remove_schedule(Key shedule_key, Transaction& t)
{
    if (shedule_key == NEVER_SCHEDULE_KEY)
    {
        throw Exception(L"Can`t remove 'never' schedule");
    }

    if (!m_scheduled_task_dataset->find_schedule_equal(shedule_key).empty())
    {
        throw Exception(L"Can`t remove active schedule");
    }

    m_db.erase<ScheduleData>(shedule_key);  
}

std::vector<Schedule> Scheduler::get_schedules() const
{
    ReadOnlyTransaction t(m_db);
    std::vector<Schedule> schedules;
    for (auto& schedule_data : m_db.query<ScheduleData>())
    {
        schedules.push_back(unpack_schedule(schedule_data));
    }
    return schedules;
}

Schedule Scheduler::get_schedule(Key schedule_key) const
{
    ReadOnlyTransaction t(m_db);
    ScheduleData schedule_data;
    m_db.load<ScheduleData>(schedule_key, schedule_data);
    return unpack_schedule(schedule_data);
}

void Scheduler::set_task_schedule(SubjectRef subject_ref, PeriodicOperation operation, boost::optional<ScheduleRef> schedule_ref, Transaction& t)
{
    auto scheduled_task_data = m_scheduled_task_dataset->get(pack(subject_ref), get_operation_identity(operation));
    scheduled_task_data.m_schedule_key = schedule_ref ? boost::make_optional(schedule_ref->get_key()): boost::none;
    m_scheduled_task_dataset->put(scheduled_task_data, t);

    if (auto source_ref = boost::get<SourceRef>(&subject_ref))
    {
        if (get_effective_task_schedule(*source_ref, operation).get_key() == NEVER_SCHEDULE_KEY)
        {
            reset_task_result(*source_ref, operation, t);
        }
    }
    else
    {
        for (auto derived_ref : get_derived_refs(m_core_domain.m_registry, subject_ref))
        {
            forward_task_schedule(derived_ref, operation, t);
        }
    }
}

boost::optional<ScheduleRef> Scheduler::get_task_schedule(SubjectRef subject_ref, PeriodicOperation operation) const
{
    ReadOnlyTransaction t(m_db);
    auto scheduled_task_data = m_scheduled_task_dataset->get(pack(subject_ref), get_operation_identity(operation));
    return scheduled_task_data.m_schedule_key ? boost::make_optional(ScheduleRef(this, *scheduled_task_data.m_schedule_key)): boost::none;
}

ScheduleRef Scheduler::get_effective_task_schedule(SubjectRef subject_ref, PeriodicOperation operation) const
{
    ReadOnlyTransaction t(m_db);
    if (auto schedule_ref = get_task_schedule(subject_ref, operation))
    {
        return *schedule_ref;
    }

    if (auto base_ref = get_base_ref(subject_ref))
    {
        return get_effective_task_schedule(*base_ref, operation);
    }

    return ScheduleRef(this, NEVER_SCHEDULE_KEY);
}

ScheduledTaskResult Scheduler::get_task_result(SourceRef source_ref, PeriodicOperation operation) const
{
    ReadOnlyTransaction t(m_db);
    auto scheduled_task_data = m_scheduled_task_dataset->get(pack(source_ref), get_operation_identity(operation));
    return ScheduledTaskResult(
        scheduled_task_data.m_launch_time, 
        scheduled_task_data.m_result, 
        unpack(scheduled_task_data.m_message)
    );
}

void Scheduler::check_scheduled_time()
{
    try
    {
        auto now = time(nullptr);
        for (auto& schedule : get_schedules())
        {
            auto schedule_time = schedule.m_scheduled_time;
            if (schedule_time > now)
            {
                continue;
            }

            Transaction t(m_db);
            schedule.m_scheduled_time = next_scheduled_time(schedule);
            update_schedule(schedule, t);
            t.commit();

            start_scheduled_tasks(schedule, schedule_time);
        }
    }
    catch (std::exception&)
    {
        // Write log error
    }
}

void Scheduler::shedule_checking_loop()
{
    no_except([&]{
        check_scheduled_time();
        m_executive->async(std::bind(&Scheduler::shedule_checking_loop, this), time(nullptr) + ONE_MIN);
    });
}

void Scheduler::start_scheduled_tasks(ScheduleRef schedule, time_t schedule_time)
{
    for (auto& source : m_sources.get_sources())
    {
        for (auto& source_resource : m_source_resources.get_source_resources(source))
        {
            if (!source_resource.m_significant)
            {
                continue;
            }

            if (auto report_type_ref = boost::get<ReportTypeRef>(&source_resource.m_resource_ref))
            {
                auto operation = PeriodicReload(*report_type_ref);
                if (get_effective_task_schedule(source, operation) == schedule)
                {
                    m_task_manager->start_scheduled_task(source, operation, schedule_time);
                }
            }           
        }

        auto source_type = source.get_source_type();
        for(auto& activity : m_source_type_activities.get_activities(source_type))
        {
            if (auto shortcut_activity = boost::get<ShortcutActivity>(&activity))
            {
                auto operation = PeriodicLaunch(shortcut_activity->m_shortcut_ref);
                if (get_effective_task_schedule(source, operation) == schedule)
                {
                    m_task_manager->start_scheduled_task(source, operation, schedule_time);
                }
            }
        }
    }
}

void Scheduler::set_task_result(SourceRef source_ref, PeriodicOperation operation, const ScheduledTaskResult& result, Transaction& t)
{
    if (get_task_result(source_ref, operation) == result)
    {
        return;
    }

    auto scheduled_task_data = m_scheduled_task_dataset->get(pack(source_ref), get_operation_identity(operation));
    scheduled_task_data.m_launch_time = result.m_launch_time;
    scheduled_task_data.m_result = result.m_result;
    scheduled_task_data.m_message = pack(result.m_message);
    m_scheduled_task_dataset->put(scheduled_task_data, t);

    m_update_task_result_sig(source_ref, operation, t);
}

void Scheduler::reset_task_result(SourceRef source_ref, PeriodicOperation operation, Transaction& t)
{
    set_task_result(source_ref, operation, ScheduledTaskResult(), t);
}

void Scheduler::forward_task_schedule(SubjectRef subject_ref, PeriodicOperation operation, Transaction& t)
{
    if (get_task_schedule(subject_ref, operation))
    {
        return;
    }

    if (auto source_ref = boost::get<SourceRef>(&subject_ref))
    {
        if (get_effective_task_schedule(*source_ref, operation).get_key() == NEVER_SCHEDULE_KEY)
        {
            reset_task_result(*source_ref, operation, t);
        }
    }
    else
    {
        for (auto derived_ref : get_derived_refs(m_core_domain.m_registry, subject_ref))
        {
            forward_task_schedule(derived_ref, operation, t);
        }
    }
}

ScheduleData Scheduler::pack_schedule(const Schedule& schedule) const
{
    return ScheduleData {
        schedule.m_ref ? schedule.m_ref->get_key() : 0,
        pack(schedule.m_name),
        int(schedule.m_period),
        schedule.m_day,
        schedule.m_week_day,
        schedule.m_day_time,
        schedule.m_scheduled_time
    };
}

Schedule Scheduler::unpack_schedule(const ScheduleData& schedule_data) const
{
    return Schedule {
        ScheduleRef(this, schedule_data.m_key),
        unpack(schedule_data.m_name),
        Schedule::Period(schedule_data.m_period),
        schedule_data.m_day,
        schedule_data.m_week_day,
        schedule_data.m_day_time,
        schedule_data.m_scheduled_time
    };
}

void Scheduler::on_remove_report_type(ReportTypeUUID report_type_uuid, Transaction& t)
{
    m_scheduled_task_dataset->erase_operation_equal(PeriodicReloadIdentity(report_type_uuid), t);
}

void Scheduler::on_remove_shortcut(UUID shortcut_uuid, Transaction& t)
{
    m_scheduled_task_dataset->erase_operation_equal(PeriodicLaunchIdentity(shortcut_uuid), t);
}

void Scheduler::on_remove_source(const std::vector<SourceKey>& source_keys, Transaction& t)
{
    m_scheduled_task_dataset->erase_source_equal(source_keys, t);
}

void Scheduler::on_remove_role(RoleKey role_key, Transaction& t)
{
    m_scheduled_task_dataset->erase_subject_equal(role_key, t);
}

}} //namespace TR { namespace Core {