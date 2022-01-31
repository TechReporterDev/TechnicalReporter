#include "stdafx.h"
#include "ExportTasks.h"
#include "CoreDomain.h"
#include "Scheduler.h"
#include "SummaryPdf.h"

namespace TR { namespace Core {

enum class SourceStatus { OK = 0, WARNING, FAILED };

void increment(SourceCounters& source_counters, SourceStatus status)
{
    ++source_counters.m_count;
    switch (status)
    {
    case SourceStatus::OK:
        ++source_counters.m_ok;
        break;

    case SourceStatus::WARNING:
        ++source_counters.m_warnings;
        break;

    case SourceStatus::FAILED:
        ++source_counters.m_errors;
        break;

    default:
        _ASSERT(false);
        break;
    }
}

class ExportOverview : public Task, public std::enable_shared_from_this<ExportOverview>
{
public:
    class Routine
    {
    public:
        Routine(std::shared_ptr<ExportOverview> task) :
            m_weak_ptr(task)
        {
        }

        std::function<void()> prepare_functor()
        {
            auto task = m_weak_ptr.lock();
            if (!task)
            {
                return nullptr;
            }

            auto& sources = task->m_core_domain.m_registry.m_sources;
            auto& source_resources = task->m_core_domain.m_registry.m_source_resources;
            auto& collector = task->m_core_domain.m_registry.m_collector;
            auto& integrity_checking = task->m_core_domain.m_services.m_integrity_checking;
            auto& compliances = task->m_core_domain.m_services.m_compliances;
            auto& validations = task->m_core_domain.m_services.m_validations;
            auto& scheduler = task->m_scheduler;

            SourceCounters source_counters;
            ReportCounters report_counters;
            std::vector<Alert> alerts;
            for (auto& source : sources.get_sources())
            {
                SourceStatus source_status = SourceStatus::OK;
                for (auto& source_resource : source_resources.get_source_resources(source))
                {
                    if (!source_resource.m_visible)
                    {
                        continue;
                    }

                    auto report_type_ref = boost::get<ReportTypeRef>(&source_resource.m_resource_ref);
                    if (!report_type_ref)
                    {
                        continue;
                    }
                    auto& report_type = **report_type_ref;

                    if (source_resource.m_deferred)
                    {
                        continue;
                    }

                    if (! collector.find_current_report(source, *report_type_ref))
                    {
                        continue;
                    }
                    
                    ++report_counters.m_count;

                    if (integrity_checking.is_checking_on(source, report_type) && 
                        integrity_checking.get_checking_result(source, report_type) == IntegrityCheckingResult::MISMATCHED)
                    {
                        ++report_counters.m_intrgrity_checking_errors;
                        source_status = std::max(source_status, SourceStatus::WARNING);
                        alerts.push_back({ source.get_name(), report_type.get_name(), Alert::Reason::INTEGRITY_CHECKING_FAILED });
                    }

                    if (auto validation = validations.find_by_output(*report_type_ref))
                    {
                        if (validations.get_validation_result(*validation, source) == ValidationResult::VALIDATION_FAILED)
                        {
                            ++report_counters.m_validation_errors;
                            source_status = std::max(source_status, SourceStatus::WARNING);
                            alerts.push_back({ source.get_name(), report_type.get_name(), Alert::Reason::VALIDATION_FAILED });                          
                        }
                    }

                    if (auto compliance = compliances.find_by_output(*report_type_ref))
                    {
                        if (compliances.get_checking_result(*compliance, source) == ComplianceCheckingResult::CHECKING_FAILED)
                        {
                            ++report_counters.m_compliance_errors;
                            source_status = std::max(source_status, SourceStatus::WARNING);
                            alerts.push_back({ source.get_name(), report_type.get_name(), Alert::Reason::COMPLIANCE_FAILED });
                        }
                    }

                    auto scheduler_reload_result = scheduler.get_task_result(source, PeriodicReload(*report_type_ref));
                    if (!scheduler_reload_result.m_result)
                    {
                        source_status = std::max(source_status, SourceStatus::FAILED);
                        alerts.push_back({ source.get_name(), report_type.get_name(), Alert::Reason::LOADING_FAILED });
                    }
                }
                increment(source_counters, source_status);
            }

            return[summaryPdf = task->m_summaryPdf, source_counters, report_counters, alerts = std::move(alerts)]() {
                summaryPdf->print_overview_title();
                summaryPdf->print_source_counters(source_counters);
                summaryPdf->print_report_counters(report_counters);
                summaryPdf->print_alerts(alerts);
            };
        }

        void on_completed() const
        {
            if (auto task = m_weak_ptr.lock())
            {
                task->emit_completed();
            }
        }

        void on_failed(const std::exception& err) const
        {
            if (auto task = m_weak_ptr.lock())
            {
                task->emit_failed(err);
            }
        }

        void on_cancel() const
        {
            if (auto task = m_weak_ptr.lock())
            {
                task->emit_completed();
            }
        }

    private:
        std::weak_ptr<ExportOverview> m_weak_ptr;
    };

    ExportOverview(CoreDomain& core_domain, Scheduler& scheduler, std::shared_ptr<SummaryPdf> summaryPdf, Executive& executive):
        Task(executive),
        m_core_domain(core_domain),
        m_scheduler(scheduler),
        m_summaryPdf(summaryPdf)
    {
    }

protected:
    // Job override
    virtual void do_run() override
    {
        m_executive.run_processing(Routine(shared_from_this()));
    }

protected:
    Key m_user_key;
    std::shared_ptr<SummaryPdf> m_summaryPdf;
    CoreDomain& m_core_domain;
    Scheduler& m_scheduler;
};

class PrintSourceHeader : public Task, public std::enable_shared_from_this<PrintSourceHeader>
{
public: 
    class Routine
    {
    public:
        Routine(std::shared_ptr<PrintSourceHeader> task) :
            m_weak_ptr(task)
        {
        }

        std::function<void()> prepare_functor()
        {
            auto task = m_weak_ptr.lock();
            if (!task)
            {
                return nullptr;
            }

            auto& sources = task->m_core_domain.m_registry.m_sources;
            auto source = sources.get_source(task->m_source_key);

            auto source_name = source.get_name();
            auto source_type = (*source.get_source_type()).get_name();
            auto source_config = source.get_config();

            return[source_name, source_type, source_config, summaryPdf = task->m_summaryPdf]() {
                summaryPdf->print_source_title(source_name, source_type);
                summaryPdf->print_source_config(source_config);              
            };
        }

        void on_completed() const
        {
            if (auto task = m_weak_ptr.lock())
            {
                task->emit_completed();
            }
        }

        void on_failed(const std::exception& err) const
        {
            if (auto task = m_weak_ptr.lock())
            {
                task->emit_failed(err);
            }
        }

        void on_cancel() const
        {
            if (auto task = m_weak_ptr.lock())
            {
                task->emit_completed();
            }
        }

    private:
        std::weak_ptr<PrintSourceHeader> m_weak_ptr;
    };

    PrintSourceHeader(SourceKey source_key, CoreDomain& core_domain, Scheduler& scheduler, std::shared_ptr<SummaryPdf> summaryPdf, Executive& executive) :
        Task(executive),
        m_source_key(source_key),
        m_core_domain(core_domain),
        m_scheduler(scheduler),
        m_summaryPdf(summaryPdf)
    {
    }

protected:
    // Job override
    virtual void do_run() override
    {
        m_executive.run_processing(Routine(shared_from_this()));
    }

private:
    SourceKey m_source_key;
    std::shared_ptr<SummaryPdf> m_summaryPdf;
    CoreDomain& m_core_domain;
    Scheduler& m_scheduler;
};

template<class Routine>
class ProcessingTask : public Task, public std::enable_shared_from_this<ProcessingTask<Routine>>
{
public:
    class RoutineWrapper
    {
    public:
        RoutineWrapper(std::shared_ptr<ProcessingTask> task) :
            m_weak_ptr(task)
        {
        }

        std::function<void()> prepare_functor()
        {
            auto task = m_weak_ptr.lock();
            if (!task)
            {
                return nullptr;
            }
            
            return task->m_routine.prepare_functor();
        }

        void on_completed() const
        {
            if (auto task = m_weak_ptr.lock())
            {
                task->m_routine.on_completed();
                task->emit_completed();
            }            
        }

        void on_failed(const std::exception& err) const
        {
            if (auto task = m_weak_ptr.lock())
            {
                task->m_routine.on_failed(err);
                task->emit_failed(err);
            }
        }

        void on_cancel() const
        {
            if (auto task = m_weak_ptr.lock())
            {
                task->m_routine.on_cancel();
                task->emit_completed();
            }
        }

    private:
        std::weak_ptr<ProcessingTask> m_weak_ptr;
    };

    template<class... Args>
    ProcessingTask(Executive& executive, Args... args) :
        Task(executive),      
        m_routine(std::move(args)...)
    {
    }

protected:
    // Job override
    virtual void do_run() override
    {
        m_executive.run_processing(RoutineWrapper(shared_from_this()));
    }

private:
    Routine m_routine;
};

template<class Functor>
struct FunctorRoutine
{
    template<class... Args>
    FunctorRoutine(Args... args) :
        m_functor(std::move(args)...)
    {
    }

    std::function<void()> prepare_functor()
    {
        return [functor = m_functor] {
            functor();
        };
    }

    void on_completed() const
    {
    }

    void on_failed(const std::exception& err) const
    {
    }

    void on_cancel() const
    {
    }

    Functor m_functor;
};


struct PrintSourceIssuesTitleFunctor
{
    PrintSourceIssuesTitleFunctor(std::shared_ptr<SummaryPdf> summaryPdf):
        m_summaryPdf(summaryPdf)
    {
    }

    void operator()() const
    {
        m_summaryPdf->print_source_issues_title();       
    }    

    std::shared_ptr<SummaryPdf> m_summaryPdf;
};

using PrintSourceIssuesTitle = ProcessingTask<FunctorRoutine<PrintSourceIssuesTitleFunctor>>;

struct PrintSourceHasNoIssuesFunctor
{
    PrintSourceHasNoIssuesFunctor(std::shared_ptr<SummaryPdf> summaryPdf) :
        m_summaryPdf(summaryPdf)
    {
    }

    void operator()() const
    {
        m_summaryPdf->print_source_has_no_issues();
    }

    std::shared_ptr<SummaryPdf> m_summaryPdf;
};

using PrintSourceHasNoIssues = ProcessingTask<FunctorRoutine<PrintSourceHasNoIssuesFunctor>>;

class PrintIntegrityCheckingRoutine
{
public:
    PrintIntegrityCheckingRoutine(SourceKey source_key, ReportTypeUUID report_type_uuid, CoreDomain& core_domain, std::shared_ptr<SummaryPdf> summaryPdf) :
        m_source_key(source_key),
        m_report_type_uuid(report_type_uuid),
        m_core_domain(core_domain),
        m_summaryPdf(summaryPdf)
    {
    }

    std::function<void()> prepare_functor()
    {
        auto& sources = m_core_domain.m_registry.m_sources;
        auto& report_types = m_core_domain.m_basis.m_report_types;
        auto& collector = m_core_domain.m_registry.m_collector;
        auto& integrity_checking = m_core_domain.m_services.m_integrity_checking;

        if (!sources.has_source(m_source_key))
        {
            return nullptr;
        }

        if (!report_types.has_report_type(m_report_type_uuid))
        {
            return nullptr;
        }

        auto& report_type = report_types.get_report_type(m_report_type_uuid);
        auto source = sources.get_source(m_source_key);

        if (integrity_checking.get_checking_result(source, report_type) != IntegrityCheckingResult::MISMATCHED)
        {
            return nullptr;
        }

        auto current_content = collector.get_current_report(source, report_type).get_content();
        auto pattern_content = integrity_checking.get_effective_pattern(source, report_type);

        return[report_name = report_type.get_name(), current_content, pattern_content, summaryPdf = m_summaryPdf]() {
            auto diff = current_content->make_diff(*pattern_content);
            summaryPdf->print_integrity_checking_alert(report_name, std::move(diff));
        };
    }

    void on_completed() const
    {
    }

    void on_failed(const std::exception& err) const
    {
    }

    void on_cancel() const
    {
    }

private:
    SourceKey m_source_key;
    ReportTypeUUID m_report_type_uuid;
    std::shared_ptr<SummaryPdf> m_summaryPdf;
    CoreDomain& m_core_domain;
};

using PrintIntegrityCheckingAlert = ProcessingTask<PrintIntegrityCheckingRoutine>;

class PrintValidationErrorsRoutine
{
public:
    PrintValidationErrorsRoutine(SourceKey source_key, ReportTypeUUID report_type_uuid, CoreDomain& core_domain, std::shared_ptr<SummaryPdf> summaryPdf) :
        m_source_key(source_key),
        m_report_type_uuid(report_type_uuid),
        m_core_domain(core_domain),
        m_summaryPdf(summaryPdf)
    {
    }

    std::function<void()> prepare_functor()
    {
        auto& sources = m_core_domain.m_registry.m_sources;
        auto& report_types = m_core_domain.m_basis.m_report_types;
        auto& collector = m_core_domain.m_registry.m_collector;
        auto& validations = m_core_domain.m_services.m_validations;

        if (!sources.has_source(m_source_key))
        {
            return nullptr;
        }

        if (!report_types.has_report_type(m_report_type_uuid))
        {
            return nullptr;
        }

        auto& report_type = report_types.get_report_type(m_report_type_uuid);
        auto source = sources.get_source(m_source_key);
        auto validation = validations.find_by_output(report_type);
        if (!validation)
        {
            return nullptr;
        }

        if (validations.get_validation_result(*validation, source) != ValidationResult::VALIDATION_FAILED)
        {
            return nullptr;
        }

        auto current_content = collector.get_current_report(source, report_type).get_content();
        return[report_name = report_type.get_name(), current_content, summaryPdf = m_summaryPdf]() {
            summaryPdf->print_validation_alert(report_name, std::dynamic_pointer_cast<RegularContent>(current_content)->as_regular_doc());
        };
    }

    void on_completed() const
    {
    }

    void on_failed(const std::exception& err) const
    {
    }

    void on_cancel() const
    {
    }

private:
    SourceKey m_source_key;
    ReportTypeUUID m_report_type_uuid;
    std::shared_ptr<SummaryPdf> m_summaryPdf;
    CoreDomain& m_core_domain;
};

using PrintValidationErrorsAlert = ProcessingTask<PrintValidationErrorsRoutine>;

class PrintSourceIssues : public Task, public std::enable_shared_from_this<PrintSourceIssues>
{
public:
    PrintSourceIssues(SourceKey source_key, CoreDomain& core_domain, Scheduler& scheduler, std::shared_ptr<SummaryPdf> summaryPdf, Executive& executive) :
        Task(executive),
        m_source_key(source_key),
        m_core_domain(core_domain),
        m_scheduler(scheduler),
        m_summaryPdf(summaryPdf)
    {
    }

protected:
    // Job override
    virtual void do_run() override
    {
        std::vector<std::shared_ptr<Task>> subtasks{
            std::make_shared<PrintSourceIssuesTitle>(m_executive, m_summaryPdf)
        };

        if (add_issues(subtasks) == 0)
        {
            subtasks.clear();
        }

        run_sequence(subtasks, [this] {
            emit_completed();
        });
    }

    unsigned add_issues(std::vector<std::shared_ptr<Task>>& subtasks)
    {
        auto& sources = m_core_domain.m_registry.m_sources;
        auto& source_resources = m_core_domain.m_registry.m_source_resources;
        auto& collector = m_core_domain.m_registry.m_collector;
        auto& integrity_checking = m_core_domain.m_services.m_integrity_checking;
        auto& validations = m_core_domain.m_services.m_validations;

        if (!sources.has_source(m_source_key))
        {
            // log source removed 
            return 0;
        }

        unsigned issues_count = 0;
        auto source = sources.get_source(m_source_key);
        for (auto& source_resource : source_resources.get_source_resources(source))
        {
            if (!source_resource.m_visible)
            {
                continue;
            }

            auto report_type_ref = boost::get<ReportTypeRef>(&source_resource.m_resource_ref);
            if (!report_type_ref)
            {
                continue;
            }
            auto& report_type = **report_type_ref;

            if (source_resource.m_deferred)
            {
                continue;
            }

            auto current_report = collector.find_current_report(source, *report_type_ref);
            if (!current_report)
            {
                continue;
            }

            if (integrity_checking.get_checking_result(source, report_type) == IntegrityCheckingResult::MISMATCHED)
            {
                subtasks.push_back(std::make_shared<PrintIntegrityCheckingAlert>(
                    m_executive,
                    m_source_key,
                    report_type.get_uuid(),
                    std::ref(m_core_domain),
                    m_summaryPdf));

                ++issues_count;
            }

            if (auto validation = validations.find_by_output(*report_type_ref))
            {
                if (validations.get_validation_result(*validation, source) == ValidationResult::VALIDATION_FAILED)
                {
                    subtasks.push_back(std::make_shared<PrintValidationErrorsAlert>(
                        m_executive,
                        m_source_key,
                        report_type.get_uuid(),
                        std::ref(m_core_domain),
                        m_summaryPdf));

                    ++issues_count;
                }
            }

            /*if (auto compliance = compliances.find_by_output(*report_type_ref))
            {
                if (compliances.get_checking_result(*compliance, source) == ComplianceCheckingResult::CHECKING_FAILED)
                {
                    ++report_counters.m_compliance_errors;
                    source_status = std::max(source_status, SourceStatus::WARNING);
                    alerts.push_back({ source.get_name(), report_type.get_name(), Alert::Reason::COMPLIANCE_FAILED });
                }
            }

            auto scheduler_reload_result = scheduler.get_task_result(source, PeriodicReload(*report_type_ref));
            if (!scheduler_reload_result.m_result)
            {
                source_status = std::max(source_status, SourceStatus::FAILED);
                alerts.push_back({ source.get_name(), report_type.get_name(), Alert::Reason::LOADING_FAILED });
            }*/
        }

        return issues_count;
    }

private:
    SourceKey m_source_key;
    std::shared_ptr<SummaryPdf> m_summaryPdf;
    CoreDomain& m_core_domain;
    Scheduler& m_scheduler;
};

class PrintSourceDetails : public Task, public std::enable_shared_from_this<PrintSourceDetails>
{
public:
    PrintSourceDetails(SourceKey source_key, CoreDomain& core_domain, Scheduler& scheduler, std::shared_ptr<SummaryPdf> summaryPdf, Executive& executive) :
        Task(executive),
        m_source_key(source_key),
        m_core_domain(core_domain),
        m_scheduler(scheduler),
        m_summaryPdf(summaryPdf)
    {
    }

protected:
    // Job override
    virtual void do_run() override
    {
        std::vector<std::shared_ptr<Task>> subtasks{
            std::make_shared<PrintSourceHeader>(
                m_source_key,
                m_core_domain,
                m_scheduler,
                m_summaryPdf,
                m_executive),
            std::make_shared<PrintSourceIssues>(
                m_source_key,
                m_core_domain,
                m_scheduler,
                m_summaryPdf,
                m_executive)
        };

        run_sequence(subtasks, [this] {
            emit_completed();
        });
    }

private:
    SourceKey m_source_key;
    std::shared_ptr<SummaryPdf> m_summaryPdf;
    CoreDomain& m_core_domain;
    Scheduler& m_scheduler;
};

class PrintDetails : public Task, public std::enable_shared_from_this<PrintDetails>
{
public: 
    PrintDetails(CoreDomain& core_domain, Scheduler& scheduler, std::shared_ptr<SummaryPdf> summaryPdf, Executive& executive) :
        Task(executive),
        m_core_domain(core_domain),
        m_scheduler(scheduler),
        m_summaryPdf(summaryPdf)
    {
    }

protected:
    // Job override
    virtual void do_run() override
    {
        std::vector<std::shared_ptr<Task>> subtasks;
        auto& sources = m_core_domain.m_registry.m_sources;
        for (auto& source : sources.get_sources())
        {
            subtasks.push_back(std::make_shared<PrintSourceDetails>(
                source.get_ref().get_key(),
                m_core_domain,
                m_scheduler,
                m_summaryPdf,
                m_executive));
        }

        run_sequence(subtasks, [this] {
            emit_completed();
        });
    }

private:
    std::shared_ptr<SummaryPdf> m_summaryPdf;
    CoreDomain& m_core_domain;
    Scheduler& m_scheduler;
};

ExportSummary::ExportSummary(CoreDomain& core_domain, Scheduler& scheduler, Key user_key, Executive& executive):
    Task(executive),
    m_core_domain(core_domain),
    m_scheduler(scheduler),
    m_user_key(user_key),
    m_sources(core_domain.m_registry.m_sources),
    m_report_types(core_domain.m_basis.m_report_types),
    m_source_resources(core_domain.m_registry.m_source_resources)
{
}

void ExportSummary::do_run()
{
    m_ostream = std::make_unique<stl_tools::boost_ios::stream<stl_tools::blob_sink>>(m_output);
    m_summaryPdf = std::make_shared<SummaryPdf>(m_ostream.get());
    //m_summaryPdf = std::make_shared<SummaryPdf>(L"d:\\Temp\\1.pdf");

    run_sequence(
        {
            std::make_shared<ExportOverview>(m_core_domain, m_scheduler, m_summaryPdf, m_executive),
            std::make_shared<PrintDetails>(m_core_domain, m_scheduler, m_summaryPdf, m_executive)
        },
        [this] {
            m_summaryPdf->close();
            m_ostream.reset();
            emit_completed();
        });
}

Blob ExportSummary::get_output()
{
    _ASSERT(get_state() == State::STATE_COMPLETED);
    return std::move(m_output);
}

}} //namespace TR { namespace Core {