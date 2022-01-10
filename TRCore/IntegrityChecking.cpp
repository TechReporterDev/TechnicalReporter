#include "stdafx.h"
#include "IntegrityChecking.h"
#include "Basis.h"
#include "Registry.h"
#include "Executive.h"
#include "SourceResources.h"
#include "JobManager.h"
#include "Transaction.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
#include "ServicesPacking.h"
#include "IntegrityCheckingDataset.h"
namespace TR {namespace Core {

struct IntegrityCheckingInput
{
    IntegrityCheckingInput(UUID pattern_uuid, UUID target_report_uuid) :
        m_pattern_uuid(pattern_uuid),
        m_target_report_uuid(target_report_uuid)
    {
    }

    UUID m_pattern_uuid;
    UUID m_target_report_uuid;
};

bool operator == (const IntegrityCheckingInput& left, const IntegrityCheckingInput& right)
{
    return left.m_pattern_uuid == right.m_pattern_uuid &&
        left.m_target_report_uuid == right.m_target_report_uuid;
}

class IntegrityCheckingJob: public Job, public std::enable_shared_from_this<IntegrityCheckingJob>
{
public:
    class Routine
    {
    public:
        using FunctorType = std::function<IntegrityCheckingResult()>;
        using ResultType = IntegrityCheckingResult;

        Routine(std::shared_ptr<IntegrityCheckingJob> job):
            m_job(job),
            m_integrity_checking(job->m_integrity_checking),
            m_sources(job->m_integrity_checking.m_sources),
            m_report_types(job->m_integrity_checking.m_report_types),
            m_source_resources(job->m_integrity_checking.m_source_resources),
            m_checking_input(stl_tools::null_uuid(), stl_tools::null_uuid())
        {
        }

        std::function<IntegrityCheckingResult()> prepare_functor()
        {
            auto job = m_job.lock();
            if (!job)
            {
                return nullptr;
            }

            auto source = m_sources.get_source(job->m_source_key);
            auto& report_type = m_report_types.get_report_type(job->m_report_type_uuid);
            auto source_resource = m_source_resources.find_source_resource(source, report_type);

            if (source_resource == boost::none ||  source_resource->m_persistent == false)                
            {
                m_integrity_checking.set_checking_result(source, report_type, IntegrityCheckingResult::UNDEFINED, { stl_tools::null_uuid(), stl_tools::null_uuid() });
                return nullptr;
            }           

            if (!m_integrity_checking.is_checking_on(source, report_type))
            {
                m_integrity_checking.set_checking_result(source, report_type, IntegrityCheckingResult::UNDEFINED, { stl_tools::null_uuid(), stl_tools::null_uuid() });
                return nullptr;
            }
                      
            auto current_report = m_integrity_checking.m_collector.find_current_report(source, report_type);
            if (!current_report)
            {
                return nullptr;
            }

            m_checking_input = {
                m_integrity_checking.get_effective_pattern_uuid(source, report_type),
                current_report->get_uuid() };

            if (m_checking_input.m_pattern_uuid != stl_tools::null_uuid())
            {
                if (m_checking_input == m_integrity_checking.get_checking_input(source, report_type))
                {
                    return nullptr;
                }
            }

            auto current_content = current_report->get_content();
            if (m_checking_input.m_pattern_uuid == stl_tools::null_uuid())
            {
                Transaction t(m_integrity_checking.m_db);
                m_checking_input.m_pattern_uuid = stl_tools::gen_uuid();
                m_integrity_checking.set_pattern_choise(source, report_type, IntegrityCheckingPatternChoise::CUSTOM_PATTERN, t);                
                m_integrity_checking.set_pattern(source, report_type, current_content, m_checking_input.m_pattern_uuid, t);
                m_integrity_checking.set_checking_result(source, report_type, IntegrityCheckingResult::MATCHED, m_checking_input, t);
                t.commit();
                return nullptr;
            }

            auto effective_pattern = m_integrity_checking.get_effective_pattern(source, report_type);
            _ASSERT(effective_pattern);            

            return [current_content, effective_pattern]{
                return current_content->is_equal(*effective_pattern) ? IntegrityCheckingResult::MATCHED : IntegrityCheckingResult::MISMATCHED;
            };
        }

        void on_completed(IntegrityCheckingResult checking_result) const
        {
            auto job = m_job.lock();
            if (!job)
            {
                return;
            }

            auto source = m_sources.get_source(job->m_source_key);
            auto& report_type = m_report_types.get_report_type(job->m_report_type_uuid);
            m_integrity_checking.set_checking_result(source, report_type, checking_result, m_checking_input);

            m_integrity_checking.m_executive->run_processing(Routine(job));
        }

        void on_failed(const std::exception& err) const
        {
            auto job = m_job.lock();
            if (!job)
            {
                return;
            }           
            job->emit_failed(err);
        }

        void on_cancel() const
        {
            auto job = m_job.lock();
            if (!job)
            {
                return;
            }
            job->emit_completed();          
        }       

    private:
        std::weak_ptr<IntegrityCheckingJob> m_job;
        IntegrityChecking& m_integrity_checking;
        Sources& m_sources;
        ReportTypes& m_report_types;
        SourceResources& m_source_resources;
        IntegrityCheckingInput m_checking_input;
    };

    IntegrityCheckingJob(IntegrityChecking& integrity_checking, SourceRef source_ref, ReportTypeRef report_type_ref):
        m_integrity_checking(integrity_checking),       
        m_source_key(source_ref.get_key()),
        m_report_type_uuid(report_type_ref.get_uuid())
    {
    }

    void do_run()
    {
        m_integrity_checking.m_executive->run_processing(Routine(shared_from_this()));
    }

private:
    IntegrityChecking& m_integrity_checking;
    SourceKey m_source_key; 
    ReportTypeUUID m_report_type_uuid;
};

class IntegrityChecking::IntegrityCheckingManager
{
public:
    IntegrityCheckingManager(IntegrityChecking& integrity_checking, Executive& executive):
        m_integrity_checking(integrity_checking),
        m_executive(executive)
    {
        m_job_manager.connect_completed([](std::pair<SourceKey, ReportTypeUUID>, std::shared_ptr<Job>){
            // unset_checking_result_dirty
        });
    }

    void start_integrity_checking(SourceRef source_ref, ReportTypeRef report_type_ref)
    {
        auto strand = std::make_pair(source_ref.get_key(), report_type_ref.get_uuid());
        if (m_job_manager.is_empty(strand))
        {
            m_job_manager.run_job(strand, std::make_shared<IntegrityCheckingJob>(m_integrity_checking, source_ref, report_type_ref));
        }
    }

private:
    IntegrityChecking& m_integrity_checking;
    Executive& m_executive;
    JobManager<std::pair<SourceKey, ReportTypeUUID>> m_job_manager;
};

IntegrityChecking::IntegrityChecking(Database& db, Basis& basis, Registry& registry):
    m_db(db),
    m_basis(basis),
    m_registry(registry),
    m_report_types(basis.m_report_types),
    m_sources(registry.m_sources),
    m_roles(registry.m_roles),
    m_collector(registry.m_collector),
    m_resource_policies(registry.m_resource_policies),
    m_source_resources(registry.m_source_resources),
    m_integrity_checking_dataset(std::make_unique<IntegrityCheckingDataset>(m_db)),
    m_executive(nullptr),
    m_running(false)
{
    m_report_types.connect_remove_report_type([this](ReportTypeUUID report_type_uuid, Transaction& t){
        on_remove_report_type(report_type_uuid, t);
    }, INTEGRITY_CHECKING_PRIORITY);

    m_sources.connect_remove_source([this](SourceKey source_key, const std::vector<SourceKey>& removed_sources, Transaction& t){
        on_remove_sources(removed_sources, t);
    }, INTEGRITY_CHECKING_PRIORITY);

    m_sources.connect_rerole_source([this](SourceRef source_ref, RoleRef prev_role, RoleRef next_role, Transaction& t){
        on_change_subject_role(source_ref, prev_role, next_role, t);
    }, INTEGRITY_CHECKING_PRIORITY);

    m_roles.connect_remove_role([this](RoleKey role_key, Transaction& t){
        on_remove_role(role_key, t);
    }, INTEGRITY_CHECKING_PRIORITY);

    m_roles.connect_replace_role([this](RoleRef role_ref, RoleRef prev_parent_ref, RoleRef next_parent_ref, Transaction& t){
        on_change_subject_role(role_ref, prev_parent_ref, next_parent_ref, t);
    }, INTEGRITY_CHECKING_PRIORITY);

    m_collector.connect_update_current_report([this](CurrentReportRef current_report_ref, bool content_changed, Transaction& t){
        _ASSERT(m_running);

        if (!content_changed)
        {
            return;
        }

        auto current_report = *current_report_ref;
        invalidate_checking_result(current_report.get_source_ref(), current_report.get_report_type_ref(), t);
    }, INTEGRITY_CHECKING_PRIORITY);

    m_source_resources.connect_update_source_resource([this](const SourceResource& source_resource, Transaction& t){
        if (!m_running)
        {
            return;
        }

        if (auto report_type_ref = boost::get<ReportTypeRef>(&source_resource.m_resource_ref))
        {
            invalidate_checking_result(source_resource.m_source_ref, *report_type_ref, t);
        }
    }, INTEGRITY_CHECKING_PRIORITY);

    m_source_resources.connect_remove_source_resource([this](SourceRef source_ref, ResourceRef resource_ref, Transaction& t){
        _ASSERT(m_running);

        if (auto report_type_ref = boost::get<ReportTypeRef>(&resource_ref))
        {
            invalidate_checking_result(source_ref, *report_type_ref, t);
        }
    }, INTEGRITY_CHECKING_PRIORITY);    
}

IntegrityChecking::~IntegrityChecking()
{
}

void IntegrityChecking::run(Executive* executive, Transaction& t)
{
    transact_assign(m_integrity_checking_manager, std::make_unique<IntegrityCheckingManager>(*this, *executive), t);
    transact_assign(m_executive, executive, t); 
    transact_assign(m_running, true, t);

    t.connect_commit([this] {
        for (auto& source : m_sources.get_sources())
        {
            for (auto& resource : m_source_resources.get_source_resources(source))
            {
                if (auto report_type_ref = boost::get<ReportTypeRef>(&resource.m_resource_ref))
                {
                    m_integrity_checking_manager->start_integrity_checking(source, *report_type_ref);
                }
            }
        }
    });
}

bool IntegrityChecking::is_checking_on(SubjectRef subject_ref, ReportTypeRef report_type_ref) const
{
    ReadOnlyTransaction t(m_db);
    auto checking_policy = m_integrity_checking_dataset->get({pack(subject_ref), pack(report_type_ref)}).m_checking_policy;

    if (checking_policy == IntegrityCheckingPolicy::ENABLE_CHECKING)
    {
        return true;
    }

    else if (checking_policy == IntegrityCheckingPolicy::DISABLE_CHECKING)
    {
        return false;
    }

    else if (auto base_ref = get_base_ref(subject_ref))
    {
        return is_checking_on(*base_ref, report_type_ref);      
    }

    return false;
}

void IntegrityChecking::set_checking_policy(SubjectRef subject_ref, ReportTypeRef report_type_ref, IntegrityCheckingPolicy checking_policy, Transaction& t)
{
    auto integrity_checking_data = m_integrity_checking_dataset->get({pack(subject_ref), pack(report_type_ref)});
    if (integrity_checking_data.m_checking_policy == checking_policy)
    {
        return;
    }   

    integrity_checking_data.m_checking_policy = checking_policy;
    m_integrity_checking_dataset->put(integrity_checking_data, t);
    
    if (auto source_ref = boost::get<SourceRef>(&subject_ref))
    {
        invalidate_checking_result(*source_ref, report_type_ref, t);
    }
    else
    {
        _ASSERT(boost::get<RoleRef>(&subject_ref));
        for (auto derived_ref : get_derived_refs(m_registry, subject_ref))
        {
            forward_checking_policy(derived_ref, report_type_ref, t);
        }
    }
}

IntegrityCheckingPolicy IntegrityChecking::get_checking_policy(SubjectRef subject_ref, ReportTypeRef report_type_ref) const
{
    return m_integrity_checking_dataset->get({pack(subject_ref), pack(report_type_ref)}).m_checking_policy;
}

void IntegrityChecking::set_pattern_choise(SubjectRef subject_ref, ReportTypeRef report_type_ref, IntegrityCheckingPatternChoise pattern_choise, Transaction& t)
{
    auto integrity_checking_data = m_integrity_checking_dataset->get({ pack(subject_ref), pack(report_type_ref) });
    if (integrity_checking_data.m_pattern_choise == pattern_choise)
    {
        return;
    }

    integrity_checking_data.m_pattern_choise = pattern_choise;
    m_integrity_checking_dataset->put(integrity_checking_data, t);

    if (auto source_ref = boost::get<SourceRef>(&subject_ref))
    {
        invalidate_checking_result(*source_ref, report_type_ref, t);
    }
    else
    {
        _ASSERT(boost::get<RoleRef>(&subject_ref));
        for (auto derived_ref : get_derived_refs(m_registry, subject_ref))
        {
            forward_pattern(derived_ref, report_type_ref, t);
        }
    }
}

IntegrityCheckingPatternChoise IntegrityChecking::get_pattern_choise(SubjectRef subject_ref, ReportTypeRef report_type_ref) const
{
    return m_integrity_checking_dataset->get({ pack(subject_ref), pack(report_type_ref) }).m_pattern_choise;
}

void IntegrityChecking::set_pattern(SubjectRef subject_ref, ReportTypeRef report_type_ref, std::shared_ptr<Content> pattern)
{
    Transaction t(m_db);
    set_pattern(subject_ref, report_type_ref, pattern, t);
    t.commit();
}

void IntegrityChecking::set_pattern(SubjectRef subject_ref, ReportTypeRef report_type_ref, std::shared_ptr<Content> pattern, Transaction& t)
{  
    if (pattern)
    {
        set_pattern(subject_ref, report_type_ref, pattern, stl_tools::gen_uuid(), t);
    }
    else
    {
        reset_pattern(subject_ref, report_type_ref, t);
    }    
}

bool IntegrityChecking::has_pattern(SubjectRef subject_ref, ReportTypeRef report_type_ref) const
{
    auto integrity_checking_data = m_integrity_checking_dataset->get({ pack(subject_ref), pack(report_type_ref) });
    return integrity_checking_data.m_custom_pattern_uuid != stl_tools::null_uuid();
}

std::shared_ptr<Content> IntegrityChecking::get_pattern(SubjectRef subject_ref, ReportTypeRef report_type_ref) const
{
    ReadOnlyTransaction t(m_db);
    auto integrity_checking_data = m_integrity_checking_dataset->get({ pack(subject_ref), pack(report_type_ref) });
    if (integrity_checking_data.m_custom_pattern_uuid == stl_tools::null_uuid())
    {
        return nullptr;
    }

    IntegrityCheckingPatternData integrity_checking_pattern_data;
    m_db.load<IntegrityCheckingPatternData>({ pack(subject_ref), pack(report_type_ref) }, integrity_checking_pattern_data);
    return (*report_type_ref).make_content(integrity_checking_pattern_data.m_pattern);
}

std::shared_ptr<Content> IntegrityChecking::get_effective_pattern(SubjectRef subject_ref, ReportTypeRef report_type_ref) const
{
    ReadOnlyTransaction t(m_db);
    if (get_pattern_choise(subject_ref, report_type_ref) == IntegrityCheckingPatternChoise::CUSTOM_PATTERN)
    {
        return get_pattern(subject_ref, report_type_ref);
    }

    if (auto base_ref = get_base_ref(subject_ref))
    {
        return get_effective_pattern(*base_ref, report_type_ref);
    }

    return nullptr;    
}

IntegrityCheckingResult IntegrityChecking::get_checking_result(SourceRef source_ref, ReportTypeRef report_type_ref) const
{
    return m_integrity_checking_dataset->get({pack(source_ref), pack(report_type_ref)}).m_result;
}

IntegrityCheckingInput IntegrityChecking::get_checking_input(SourceRef source_ref, ReportTypeRef report_type_ref) const
{
    auto integrity_checking_data = m_integrity_checking_dataset->get({ pack(source_ref), pack(report_type_ref) });
    return { integrity_checking_data.m_input.m_pattern_uuid, integrity_checking_data.m_input.m_target_report_uuid };
}

UUID IntegrityChecking::get_effective_pattern_uuid(SubjectRef subject_ref, ReportTypeRef report_type_ref) const
{
    ReadOnlyTransaction t(m_db);
    auto integrity_checking_data = m_integrity_checking_dataset->get({ pack(subject_ref), pack(report_type_ref) });

    if (integrity_checking_data.m_pattern_choise == IntegrityCheckingPatternChoise::CUSTOM_PATTERN)
    {
        return integrity_checking_data.m_custom_pattern_uuid;
    }
   
    else if (auto base_ref = get_base_ref(subject_ref))
    {
        return get_effective_pattern_uuid(*base_ref, report_type_ref);
    }

    return stl_tools::null_uuid();
}

void IntegrityChecking::set_pattern(SubjectRef subject_ref, ReportTypeRef report_type_ref, std::shared_ptr<Content> pattern, UUID pattern_uuid, Transaction& t)
{
    _ASSERT(pattern);

    auto integrity_checking_data = m_integrity_checking_dataset->get({ pack(subject_ref), pack(report_type_ref) });
    
    IntegrityCheckingPatternData pattern_data{
        {pack(subject_ref), pack(report_type_ref)},
        pattern->as_blob()
    };

    if (integrity_checking_data.m_custom_pattern_uuid == stl_tools::null_uuid())
    {
        m_db.persist(pattern_data);
    }
    else
    {
        m_db.update(pattern_data);
    }

    integrity_checking_data.m_custom_pattern_uuid = pattern_uuid;
    m_integrity_checking_dataset->put(integrity_checking_data, t);   

    if (auto source_ref = boost::get<SourceRef>(&subject_ref))
    {
        invalidate_checking_result(*source_ref, report_type_ref, t);
    }
    else
    {
        _ASSERT(boost::get<RoleRef>(&subject_ref));
        for (auto derived_ref : get_derived_refs(m_registry, subject_ref))
        {
            forward_pattern(derived_ref, report_type_ref, t);
        }
    }
}

void IntegrityChecking::reset_pattern(SubjectRef subject_ref, ReportTypeRef report_type_ref, Transaction& t)
{
    auto integrity_checking_data = m_integrity_checking_dataset->get({ pack(subject_ref), pack(report_type_ref) });

    if (integrity_checking_data.m_custom_pattern_uuid != stl_tools::null_uuid())
    {
        m_db.erase<IntegrityCheckingPatternData>({ pack(subject_ref), pack(report_type_ref) });
    }

    integrity_checking_data.m_custom_pattern_uuid = stl_tools::null_uuid();
    m_integrity_checking_dataset->put(integrity_checking_data, t);

    if (auto source_ref = boost::get<SourceRef>(&subject_ref))
    {
        invalidate_checking_result(*source_ref, report_type_ref, t);
    }
    else
    {
        _ASSERT(boost::get<RoleRef>(&subject_ref));
        for (auto derived_ref : get_derived_refs(m_registry, subject_ref))
        {
            forward_pattern(derived_ref, report_type_ref, t);
        }
    }
}

void IntegrityChecking::set_checking_result(SourceRef source_ref, ReportTypeRef report_type_ref, IntegrityCheckingResult checking_result, const IntegrityCheckingInput& input, Transaction& t)
{
    auto integrity_checking_data = m_integrity_checking_dataset->get({pack(source_ref), pack(report_type_ref)});
    if (integrity_checking_data.m_result == checking_result && 
        integrity_checking_data.m_input.m_pattern_uuid == input.m_pattern_uuid &&
        integrity_checking_data.m_input.m_target_report_uuid == input.m_target_report_uuid)
    {
        return;
    }

    integrity_checking_data.m_result = checking_result;
    integrity_checking_data.m_input = { input.m_pattern_uuid, input.m_target_report_uuid };
    m_integrity_checking_dataset->put(integrity_checking_data, t);
    m_update_checking_result_sig(source_ref, report_type_ref, checking_result, t);
}

void IntegrityChecking::set_checking_result(SourceRef source_ref, ReportTypeRef report_type_ref, IntegrityCheckingResult checking_result, const IntegrityCheckingInput& input)
{
    Transaction t(m_db);    
    set_checking_result(source_ref, report_type_ref, checking_result, input, t);
    t.commit();
}

void IntegrityChecking::forward_checking_policy(SubjectRef subject_ref, ReportTypeRef report_type_ref, Transaction& t)
{
    auto checking_policy = get_checking_policy(subject_ref, report_type_ref);
    if (checking_policy != IntegrityCheckingPolicy::DEFAULT_POLICY)
    {
        return;
    }

    if (auto source_ref = boost::get<SourceRef>(&subject_ref))
    {
        m_source_resources.check_service_request(*source_ref, report_type_ref, t);
        invalidate_checking_result(*source_ref, report_type_ref, t);
    }
    else
    {
        _ASSERT(boost::get<RoleRef>(&subject_ref));
        for (auto derived_ref : get_derived_refs(m_registry, subject_ref))
        {
            forward_checking_policy(derived_ref, report_type_ref, t);
        }
    }   
}

void IntegrityChecking::forward_pattern(SubjectRef subject_ref, ReportTypeRef report_type_ref, Transaction& t)
{
    auto pattern_choise = get_pattern_choise(subject_ref, report_type_ref);
    if (pattern_choise != IntegrityCheckingPatternChoise::DEFAULT_PATTERN)
    {
        return;
    }
    
    if (auto source_ref = boost::get<SourceRef>(&subject_ref))
    {
        invalidate_checking_result(*source_ref, report_type_ref, t);
    }
    else
    {
        _ASSERT(boost::get<RoleRef>(&subject_ref));
        for (auto derived_ref : get_derived_refs(m_registry, subject_ref))
        {
            forward_pattern(derived_ref, report_type_ref, t);
        }
    }   
}

void IntegrityChecking::invalidate_checking_result(SourceRef source_ref, ReportTypeRef report_type_ref, Transaction& t)
{
    t.connect_commit([source_ref, report_type_ref, this]() {
        m_integrity_checking_manager->start_integrity_checking(*source_ref, report_type_ref);
    });   
}

void IntegrityChecking::on_change_subject_role(SubjectRef subject_ref, RoleRef prev_role, RoleRef next_role, Transaction& t)
{
    for (auto& report_type : m_report_types.get_report_types())
    {       
        forward_checking_policy(subject_ref, report_type, t);        
        forward_pattern(subject_ref, report_type, t);        
    }
}

void IntegrityChecking::on_remove_report_type(ReportTypeUUID report_type_uuid, Transaction& t)
{
    m_integrity_checking_dataset->erase_report_type_equal(report_type_uuid, t);
    m_db.erase_query<IntegrityCheckingPatternData>(
        odb::query<IntegrityCheckingPatternData>::identity.report_type_uuid.value == report_type_uuid);    
}

void IntegrityChecking::on_remove_sources(const std::vector<SourceKey>& removed_sources, Transaction& t)
{
    m_integrity_checking_dataset->erase_source_equal(removed_sources, t);
    m_db.erase_query<IntegrityCheckingPatternData>(
        odb::query<IntegrityCheckingPatternData>::identity.subject.value1.value.in_range(removed_sources.begin(), removed_sources.end()));
}

void IntegrityChecking::on_remove_role(RoleKey role_key, Transaction& t)
{
    m_integrity_checking_dataset->erase_subject_equal(role_key, t);
    m_db.erase_query<IntegrityCheckingPatternData>(
        odb::query<IntegrityCheckingPatternData>::identity.subject.value1.value == 0 &&
        odb::query<IntegrityCheckingPatternData>::identity.subject.value2.value == role_key);
}

}} //namespace TR { namespace Core {