#include "stdafx.h"
#include "Compliance.h"
#include "Basis.h"
#include "Registry.h"
#include "Executive.h"
#include "JoinedQueries.h"
#include "JobManager.h"
#include "Transaction.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
#include "ServicesPacking.h"
#include "ComplianceDataset.h"
namespace TR { namespace Core {

class ComplianceCheckingJob: public Job, public std::enable_shared_from_this<ComplianceCheckingJob>
{
public:
    class Routine
    {
    public:
        using FunctorType = std::function<ComplianceCheckingResult()>;
        using ResultType = ComplianceCheckingResult;

        Routine(std::shared_ptr<ComplianceCheckingJob> job):
            m_job(job),
            m_compliances(job->m_compliances),
            m_sources(job->m_compliances.m_sources),
            m_report_types(job->m_compliances.m_report_types),
            m_source_resources(job->m_compliances.m_source_resources),            
            m_input_uuid(stl_tools::null_uuid())
        {
        }

        std::function<ComplianceCheckingResult()> prepare_functor()
        {
            auto job = m_job.lock();
            if (!job)
            {
                return nullptr;
            }

            auto source = m_sources.get_source(job->m_source_key);
            auto compliance = m_compliances.get_compliance(job->m_compliance_key);
            auto output_ref = compliance.get_output_ref();

            auto source_resource = m_source_resources.find_source_resource(source, output_ref);
            if (!source_resource || !source_resource->m_persistent)
            {
                m_compliances.set_checking_result(compliance, source, ComplianceCheckingResult::UNDEFINED, stl_tools::null_uuid());
                return nullptr;
            }

            if (!m_compliances.is_checking_on(compliance, source))
            {
                m_compliances.set_checking_result(compliance, source, ComplianceCheckingResult::UNDEFINED, stl_tools::null_uuid());
                return nullptr;
            }

            auto current_report = m_compliances.m_collector.find_current_report(source, compliance.get_output_ref());
            if (!current_report)
            {
                return nullptr;
            }

            m_input_uuid = current_report->get_uuid();
            if (m_input_uuid == m_compliances.get_checking_input(compliance, source))
            {
                return nullptr;
            }

            auto current_content = std::dynamic_pointer_cast<ComplianceContent>(current_report->get_content());
            return [current_content]{
                auto& compliance_doc = current_content->get_compliance_doc();
                for (auto& rule_node : compliance_doc.get_rules())
                {
                    if (rule_node.get_check_result() == XML::XmlRuleNode::CheckResult::FAILED)
                    {
                        return ComplianceCheckingResult::CHECKING_FAILED;
                    }
                }
                return ComplianceCheckingResult::CHECKING_SUCCEEDED;
            };
        }

        void on_completed(ComplianceCheckingResult checking_result) const
        {
            auto job = m_job.lock();
            if (!job)
            {
                return;
            }

            auto source = m_sources.get_source(job->m_source_key);
            auto compliance = m_compliances.get_compliance(job->m_compliance_key);
            m_compliances.set_checking_result(compliance, source, checking_result, m_input_uuid);
            m_compliances.m_executive->run_processing(Routine(job));
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
        std::weak_ptr<ComplianceCheckingJob> m_job;
        Compliances& m_compliances;
        Sources& m_sources;
        ReportTypes& m_report_types;
        SourceResources& m_source_resources;
        UUID m_input_uuid;             
    };

    ComplianceCheckingJob(Compliances& compliances, SourceRef source_ref, ComplianceRef compliance_ref):
        m_compliances(compliances),
        m_source_key(source_ref.get_key()),
        m_compliance_key(compliance_ref.get_key())
    {
    }

    void do_run()
    {
        m_compliances.m_executive->run_processing(Routine(shared_from_this()));
    }

private:
    Compliances& m_compliances; 
    SourceKey m_source_key;
    ComplianceKey m_compliance_key;
};

class Compliances::ComplianceCheckingManager
{
public:
    ComplianceCheckingManager(Compliances& compliances, Executive& executive):
        m_compliances(compliances),
        m_executive(executive)
    {
    }

    void start_compliance_checking(SourceRef source_ref, ComplianceRef compliance_ref)
    {
        auto strand = std::make_pair(source_ref.get_key(), compliance_ref.get_key());
        m_job_manager.run_job(strand, std::make_shared<ComplianceCheckingJob>(m_compliances, source_ref, compliance_ref));
    }

private:
    Compliances& m_compliances;
    Executive& m_executive;
    JobManager<std::pair<SourceKey, ComplianceKey>> m_job_manager;
};

Compliance::Compliance(std::wstring name, ReportTypeRef base_report_type_ref):
    m_name(std::move(name)),
    m_base_report_type_ref(base_report_type_ref)
{
}

std::wstring Compliance::get_name() const
{
    return m_name;
}

ReportTypeRef Compliance::get_base_report_type_ref() const
{
    return m_base_report_type_ref;
}

ReportTypeRef Compliance::get_output_ref() const
{
    _ASSERT(m_output_ref);
    if (!m_output_ref)
    {
        throw Exception(L"Comliance is not persist.");
    }
    return *m_output_ref;
}

ComplianceRef Compliance::get_ref() const
{
    _ASSERT(m_ref);
    if (!m_ref)
    {
        throw Exception(L"Comliance is not persist.");
    }
    return *m_ref;
}

Compliance::operator ComplianceRef() const
{
    return get_ref();
}

Compliance::Compliance(ComplianceRef ref, std::wstring name, ReportTypeRef base_report_type_ref, ReportTypeRef output_ref):
    m_ref(ref),
    m_name(std::move(name)),
    m_base_report_type_ref(base_report_type_ref),
    m_output_ref(output_ref)
{
}

Compliances::Compliances(Database& db, Basis& basis, Registry& registry):
    m_db(db),
    m_basis(basis),
    m_registry(registry),
    m_report_types(basis.m_report_types),
    m_sources(registry.m_sources),
    m_roles(registry.m_roles),
    m_collector(registry.m_collector),
    m_resource_policies(registry.m_resource_policies),
    m_source_resources(registry.m_source_resources),
    m_joined_queries(nullptr),
    m_compliance_dataset(std::make_unique<ComplianceDataset>(m_db)),
    m_subject_compliance_dataset(std::make_unique<SubjectComplianceDataset>(m_db)),
    m_executive(nullptr),
    m_running(false)
{
    m_sources.connect_remove_source([this](SourceKey source_key, const std::vector<SourceKey>& removed_sources, Transaction& t){
        on_remove_sources(removed_sources, t);
    }, COMPLIANCE_PRIORITY);
    
    m_sources.connect_rerole_source([this](SourceRef source_ref, RoleRef prev_role, RoleRef next_role, Transaction& t){
        on_change_subject_role(source_ref, prev_role, next_role, t);
    }, COMPLIANCE_PRIORITY);

    m_roles.connect_remove_role([this](RoleKey role_key, Transaction& t){
        on_remove_role(role_key, t);
    }, COMPLIANCE_PRIORITY);    

    m_roles.connect_replace_role([this](RoleRef role_ref, RoleRef prev_parent_ref, RoleRef next_parent_ref, Transaction& t){
        on_change_subject_role(role_ref, prev_parent_ref, next_parent_ref, t);
    }, COMPLIANCE_PRIORITY);

    m_collector.connect_update_current_report([this](CurrentReportRef current_report_ref, bool content_changed, Transaction& t){
        if (!content_changed)
        {
            return;
        }

        auto current_report = *current_report_ref;
        if (auto compliance = find_by_output(current_report.get_report_type_ref()))
        {
            invalidate_checking_result(*compliance, current_report.get_source_ref(), t);
        }

    }, COMPLIANCE_PRIORITY);

    m_source_resources.connect_update_source_resource([this](const SourceResource& source_resource, Transaction& t){
        if (!m_running)
        {
            return;
        }
        
        if (auto report_type_ref = boost::get<ReportTypeRef>(&source_resource.m_resource_ref))
        {
            if (auto compliance = find_by_output(*report_type_ref))
            {
                invalidate_checking_result(*compliance, source_resource.m_source_ref, t);
            }
        }
    }, COMPLIANCE_PRIORITY);

    m_source_resources.connect_remove_source_resource([this](SourceRef source_ref, ResourceRef resource_ref, Transaction& t){
        _ASSERT(m_running);

        if (auto report_type_ref = boost::get<ReportTypeRef>(&resource_ref))
        {
            if (auto compliance = find_by_output(*report_type_ref))
            {
                invalidate_checking_result(*compliance, source_ref, t);
            }
        }
    }, COMPLIANCE_PRIORITY);
}

Compliances::~Compliances()
{
}

void Compliances::set_joined_queries(JoinedQueries* joined_queries)
{
    _ASSERT(!m_joined_queries);
    m_joined_queries = joined_queries;
}

void Compliances::restore(Transaction& t)
{
    for (auto compliance_data : m_db.query<ComplianceData>())
    {
        auto input_ref = unpack(compliance_data.m_input_uuid, &m_report_types);

        m_report_types.add_report_type(std::make_unique<ComplianceReportType>(
            compliance_data.m_output_uuid,
            unpack(compliance_data.m_name),
            [this, input_ref]{
                auto& input = dynamic_cast<const CheckListReportType&>(*input_ref);
                return clone_doc(*input.get_compliance_def());
            }), t
        );
    }
}

void Compliances::run(Executive* executive, Transaction& t)
{
    transact_assign(m_compliance_checking_manager, std::make_unique<ComplianceCheckingManager>(*this, *executive), t);
    transact_assign(m_executive, executive, t); 
    transact_assign(m_running, true, t);

    t.connect_commit([this] {
        for (auto& source : m_sources.get_sources())
        {
            for (auto& compliance : get_compliances())
            {
                if (m_source_resources.find_source_resource(source, compliance.get_output_ref()))
                {
                    m_compliance_checking_manager->start_compliance_checking(source, compliance);
                }                
            }
        }
    });
}

void Compliances::add_compliance(Compliance& compliance, Transaction& t)
{
    _ASSERT(!compliance.m_ref);
    auto input = dynamic_cast<const CheckListReportType&>(*compliance.get_base_report_type_ref());
    
    auto& output = m_basis.m_report_types.add_report_type(
        std::make_unique<ComplianceReportType>(stl_tools::gen_uuid(), compliance.get_name(), clone_doc(*input.get_compliance_def())),
        t);
    
    QueryJoining query_joining(input, output, XML::create_compliance_setup(*input.get_compliance_def()));       
    m_joined_queries->add_query_joining(query_joining, t);
    
    ComplianceData compliance_data(
        query_joining.get_key(),
        pack(compliance.m_name),        
        pack(input.get_ref()),
        pack(output.get_ref()));
    
    compliance.m_ref = ComplianceRef(this, m_compliance_dataset->persist(compliance_data, t));
    compliance.m_output_ref = output;
}

std::vector<Compliance> Compliances::get_compliances() const
{
    ReadOnlyTransaction t(m_db);
    std::vector<Compliance> compliances;
    for (auto& compliance_data : m_compliance_dataset->query())
    {
        compliances.push_back({
            ComplianceRef(this, ComplianceKey(compliance_data.m_key)),
            unpack(compliance_data.m_name),
            unpack(compliance_data.m_input_uuid, &m_report_types),
            unpack(compliance_data.m_output_uuid, &m_report_types),
        });
    }
    return compliances;
}

Compliance Compliances::get_compliance(ComplianceKey compliance_key) const
{
    ReadOnlyTransaction t(m_db);
    auto compliance_data = m_compliance_dataset->load(compliance_key);

    return Compliance(
        ComplianceRef(this, ComplianceKey(compliance_data.m_key)),
        unpack(compliance_data.m_name),
        unpack(compliance_data.m_input_uuid, &m_report_types),
        unpack(compliance_data.m_output_uuid, &m_report_types)
    );
}

void Compliances::remove_compliance(ComplianceKey compliance_key, Transaction& t)
{
    auto compliance_data = m_compliance_dataset->load(compliance_key);
    m_joined_queries->remove_query_joining(QueryJoiningKey(compliance_data.m_key), t);
    m_report_types.remove_report_type(compliance_data.m_output_uuid);
    m_subject_compliance_dataset->erase_compliance_equal(compliance_key, t);
    m_compliance_dataset->erase(compliance_key, t);
}

boost::optional<Compliance> Compliances::find_compliance(ComplianceKey compliance_key) const
{
    ReadOnlyTransaction t(m_db);
    if (auto compliance_data = m_compliance_dataset->find(compliance_key))
    {
        return Compliance(
            ComplianceRef(this, ComplianceKey(compliance_data->m_key)),
            unpack(compliance_data->m_name),
            unpack(compliance_data->m_input_uuid, &m_report_types),
            unpack(compliance_data->m_output_uuid, &m_report_types)
        );
    }
    return boost::none;
}

boost::optional<Compliance> Compliances::find_by_input(ReportTypeRef base_report_type_ref) const
{
    ReadOnlyTransaction t(m_db);
    if (auto compliance_data = m_compliance_dataset->find_by_input(base_report_type_ref.get_uuid()))
    {
        return Compliance(
            ComplianceRef(this, ComplianceKey(compliance_data->m_key)),
            unpack(compliance_data->m_name),
            unpack(compliance_data->m_input_uuid, &m_report_types),
            unpack(compliance_data->m_output_uuid, &m_report_types)
        );
    }

    return boost::none;
}

boost::optional<Compliance> Compliances::find_by_output(ReportTypeRef output_ref) const
{
    ReadOnlyTransaction t(m_db);
    if (auto compliance_data = m_compliance_dataset->find_by_output(output_ref.get_uuid()))
    {
        return Compliance(
            ComplianceRef(this, ComplianceKey(compliance_data->m_key)),
            unpack(compliance_data->m_name),
            unpack(compliance_data->m_input_uuid, &m_report_types),
            unpack(compliance_data->m_output_uuid, &m_report_types)
        );
    }

    return boost::none;
}

bool Compliances::is_checking_on(ComplianceRef compliance_ref, SubjectRef subject_ref)
{
    ReadOnlyTransaction t(m_db);
    auto checking_policy = get_checking_policy(compliance_ref, subject_ref);

    if (checking_policy == ComplianceCheckingPolicy::ENABLE_CHECKING)
    {
        return true;
    }

    else if (checking_policy == ComplianceCheckingPolicy::DISABLE_CHECKING)
    {
        return false;
    }

    else if (auto base_ref = get_base_ref(subject_ref))
    {
        return is_checking_on(compliance_ref, *base_ref);
    }

    return true;
}

void Compliances::set_checking_policy(ComplianceRef compliance_ref, SubjectRef subject_ref, ComplianceCheckingPolicy checking_policy, Transaction& t)
{
    SubjectComplianceData subject_compliance_data = m_subject_compliance_dataset->get(pack(subject_ref), compliance_ref.get_key());
    if (subject_compliance_data.m_checking_policy == checking_policy)
    {
        return;
    }

    subject_compliance_data.m_checking_policy = checking_policy;
    m_subject_compliance_dataset->put(subject_compliance_data, t);

    if (auto source_ref = boost::get<SourceRef>(&subject_ref))
    {
        m_source_resources.check_service_request(*source_ref, (*compliance_ref).get_output_ref(), t);
        invalidate_checking_result(compliance_ref, *source_ref, t);
        return;
    }

    _ASSERT(boost::get<RoleRef>(&subject_ref));
    for (auto derived_ref : get_derived_refs(m_registry, subject_ref))
    {
        forward_checking_policy(compliance_ref, derived_ref, t);
    }
}

ComplianceCheckingPolicy Compliances::get_checking_policy(ComplianceRef compliance_ref, SubjectRef subject_ref) const
{
    return m_subject_compliance_dataset->get(pack(subject_ref), compliance_ref.get_key()).m_checking_policy;
}

void Compliances::forward_checking_policy(ComplianceRef compliance_ref, SubjectRef subject_ref, Transaction& t)
{
    auto checking_policy = get_checking_policy(compliance_ref, subject_ref);
    if (checking_policy != ComplianceCheckingPolicy::DEFAULT_POLICY)
    {
        return;
    }

    if (auto source_ref = boost::get<SourceRef>(&subject_ref))
    {       
        m_source_resources.check_service_request(*source_ref, (*compliance_ref).get_output_ref(), t);
        invalidate_checking_result(compliance_ref, *source_ref, t);
        return;
    }

    _ASSERT(boost::get<RoleRef>(&subject_ref));
    for (auto derived_ref : get_derived_refs(m_registry, subject_ref))
    {
        forward_checking_policy(compliance_ref, derived_ref, t);
    }
}

bool Compliances::has_compliance_setup(ComplianceRef compliance_ref, SubjectRef subject_ref) const
{
    auto query_joining = m_joined_queries->get_query_joining(QueryJoiningKey(compliance_ref.get_key()));
    return m_joined_queries->has_subject_query(query_joining, subject_ref);
}

std::shared_ptr<const XmlComplianceSetup> Compliances::get_compliance_setup(ComplianceRef compliance_ref, SubjectRef subject_ref) const
{
    auto query_joining = m_joined_queries->get_query_joining(QueryJoiningKey(compliance_ref.get_key()));
    auto compliance_setup = std::static_pointer_cast<const XmlComplianceSetup>(m_joined_queries->get_subject_query(query_joining, subject_ref));
    if (!compliance_setup)
    {
        auto& output = dynamic_cast<const ComplianceReportType&>(*query_joining.get_output_ref());
        compliance_setup = XML::create_compliance_setup(*output.get_compliance_def());
    }
    return compliance_setup;
}

void Compliances::set_compliance_setup(ComplianceRef compliance_ref, SubjectRef subject_ref, std::unique_ptr<XmlComplianceSetup> compliance_setup, Transaction& t)
{
    auto query_joining = m_joined_queries->get_query_joining(QueryJoiningKey(compliance_ref.get_key()));
    m_joined_queries->set_subject_query(query_joining, subject_ref, std::move(compliance_setup), t);
}

std::shared_ptr<const XmlComplianceSetup> Compliances::get_base_setup(ComplianceRef compliance_ref, SubjectRef subject_ref) const
{
    auto query_joining = m_joined_queries->get_query_joining(QueryJoiningKey(compliance_ref.get_key()));
    return std::static_pointer_cast<const XmlComplianceSetup>(m_joined_queries->get_base_query(query_joining, subject_ref));
}

std::shared_ptr<const XmlComplianceDef> Compliances::get_compliance_def(ComplianceRef compliance_ref) const
{
    auto query_joining = m_joined_queries->get_query_joining(QueryJoiningKey(compliance_ref.get_key()));
    return std::static_pointer_cast<const XmlComplianceDef>(query_joining.get_query_def());
}

ComplianceCheckingResult Compliances::get_checking_result(ComplianceRef compliance_ref, SourceRef source_ref) const
{
    ReadOnlyTransaction t(m_db);
    auto subject_compliance_data = m_subject_compliance_dataset->get(pack(source_ref), compliance_ref.get_key());
    return subject_compliance_data.m_checking_result;
}

UUID Compliances::get_checking_input(ComplianceRef compliance_ref, SourceRef source_ref) const
{
    ReadOnlyTransaction t(m_db);
    auto subject_compliance_data = m_subject_compliance_dataset->get(pack(source_ref), compliance_ref.get_key());
    return subject_compliance_data.m_input_uuid;
}

void Compliances::invalidate_checking_result(ComplianceRef compliance_ref, SourceRef source_ref, Transaction& t)
{
    t.connect_commit([source_ref, compliance_ref, this]() {
        m_compliance_checking_manager->start_compliance_checking(*source_ref, compliance_ref);
    });
}

void Compliances::set_checking_result(ComplianceRef compliance_ref, SourceRef source_ref, ComplianceCheckingResult checking_result, UUID input_uuid)
{
    Transaction t(m_db);
    set_checking_result(compliance_ref, source_ref, checking_result, input_uuid, t);
    t.commit();
}

void Compliances::set_checking_result(ComplianceRef compliance_ref, SourceRef source_ref, ComplianceCheckingResult checking_result, UUID input_uuid, Transaction& t)
{
    auto subject_compliance_data = m_subject_compliance_dataset->get(pack(source_ref), compliance_ref.get_key());
    if (subject_compliance_data.m_checking_result == checking_result && subject_compliance_data.m_input_uuid == input_uuid)
    {
        return;
    }

    subject_compliance_data.m_checking_result = checking_result;
    subject_compliance_data.m_input_uuid = input_uuid;
    m_subject_compliance_dataset->put(subject_compliance_data, t);
    m_update_checking_result_sig(compliance_ref, source_ref, checking_result, t);
}

void Compliances::on_change_subject_role(SubjectRef subject_ref, RoleRef prev_role, RoleRef next_role, Transaction& t)
{
    for (auto& compliance : get_compliances())
    {
        if (is_checking_on(compliance, prev_role) != is_checking_on(compliance, next_role))
        {
            forward_checking_policy(compliance, subject_ref, t);
        }
    }
}

void Compliances::on_remove_sources(const std::vector<SourceKey>& removed_sources, Transaction& t)
{
    m_subject_compliance_dataset->erase_source_equal(removed_sources, t);
}

void Compliances::on_remove_role(RoleKey role_key, Transaction& t)
{
    m_subject_compliance_dataset->erase_subject_equal(role_key, t);
}

}} //namespace TR { namespace Core {