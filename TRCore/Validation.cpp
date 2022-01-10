#include "stdafx.h"
#include "Validation.h"
#include "Basis.h"
#include "Registry.h"
#include "Executive.h"
#include "JoinedQueries.h"
#include "SourceResources.h"
#include "ServicesPacking.h"
#include "JobManager.h"
#include "Transaction.h"
#include "ValidationDataset.h"
namespace TR { namespace Core {

namespace {
bool is_valid(const XML::XmlRegularNode& validated_node)
{
    if (validated_node.get_bound_def().is_trivial())
    {
        return static_cast<const XML::XmlValidatedNode&>(validated_node).is_valid();
    }

    for (auto& child_node : xml_tools::get_child_nodes(validated_node))
    {
        if (!is_valid(static_cast<const XML::XmlRegularNode&>(child_node)))
        {
            return false;
        }
    }
    return true;
}

bool is_valid(const XmlRegularDoc& validated_doc)
{
    return is_valid(*validated_doc.get_root());
}
} //namespace {

class ValidationJob: public Job, public std::enable_shared_from_this<ValidationJob>
{
public:
    class Routine
    {
    public:
        using FunctorType = std::function<ValidationResult()>;
        using ResultType = ValidationResult;

        Routine(std::shared_ptr<ValidationJob> job):
            m_job(job),
            m_validations(job->m_validations),
            m_sources(job->m_validations.m_sources),
            m_report_types(job->m_validations.m_report_types),
            m_source_resources(job->m_validations.m_source_resources),
            m_input_uuid(stl_tools::null_uuid())
        {
        }

        std::function<ValidationResult()> prepare_functor()
        {
            auto job = m_job.lock();
            if (!job)
            {
                return nullptr;
            }

            auto source = m_sources.get_source(job->m_source_key);
            auto validation = m_validations.get_validation(job->m_validation_key);
            auto output_ref = validation.get_output_ref();

            auto source_resource = m_source_resources.find_source_resource(source, output_ref);
            if (!source_resource || !source_resource->m_persistent)
            {
                m_validations.set_validation_result(validation, source, ValidationResult::UNDEFINED, stl_tools::null_uuid());
                return nullptr;
            }

            if (!m_validations.is_validation_on(validation, source))
            {
                m_validations.set_validation_result(validation, source, ValidationResult::UNDEFINED, stl_tools::null_uuid());
                return nullptr;
            }

            auto current_report = m_validations.m_collector.find_current_report(source, output_ref);
            if (!current_report)
            {
                return nullptr;
            }

            m_input_uuid = current_report->get_uuid();
            if (m_input_uuid == m_validations.get_validation_input(validation, source))
            {
                return nullptr;
            }

            auto current_content = std::dynamic_pointer_cast<RegularContent>(current_report->get_content());
            return [current_content]{
                auto& validated_doc = current_content->as_regular_doc();
                return is_valid(validated_doc) ? ValidationResult::VALIDATION_SUCCEEDED : ValidationResult::VALIDATION_FAILED;
            };
        }

        void on_completed(ValidationResult validation_result) const
        {
            auto job = m_job.lock();
            if (!job)
            {
                return;
            }

            auto source = m_sources.get_source(job->m_source_key);
            auto validation = m_validations.get_validation(job->m_validation_key);
            m_validations.set_validation_result(validation, source, validation_result, m_input_uuid);
            m_validations.m_executive->run_processing(Routine(job));
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
        std::weak_ptr<ValidationJob> m_job;
        Validations& m_validations;
        Sources& m_sources;
        ReportTypes& m_report_types;
        SourceResources& m_source_resources;
        UUID m_input_uuid;
    };
    

    ValidationJob(Validations& validations, SourceRef source_ref, ValidationRef validation_ref):
        m_validations(validations), 
        m_source_key(source_ref.get_key()),
        m_validation_key(validation_ref.get_key())
    {
    }

    void do_run()
    {
        m_validations.m_executive->run_processing(Routine(shared_from_this()));
    }

private:
    Validations& m_validations;
    SourceKey m_source_key;
    ValidationKey m_validation_key;
};

class Validations::ValidationManager
{
public:
    ValidationManager(Validations& validations, Executive& executive):
        m_validations(validations),
        m_executive(executive)
    {
    }

    void start_validating(SourceRef source_ref, ValidationRef validation_ref)
    {
        auto strand = std::make_pair(source_ref.get_key(), validation_ref.get_key());
        m_job_manager.run_job(strand, std::make_shared<ValidationJob>(m_validations, source_ref, validation_ref));
    }

private:
    Validations& m_validations;
    Executive& m_executive;
    JobManager<std::pair<SourceKey, ValidationKey>> m_job_manager;
};

ValidationInitializer::ValidationInitializer(std::wstring name, ReportTypeRef input_ref, std::unique_ptr<XmlQueryDoc> default_validator):
    m_name(std::move(name)), 
    m_input_ref(input_ref), 
    m_default_validator(std::move(default_validator))
{
}

ValidationInitializer::ValidationInitializer(std::wstring name, ReportTypeRef input_ref, ReportTypeUUID output_uuid, std::unique_ptr<XmlQueryDoc> default_validator):
    m_name(std::move(name)), 
    m_input_ref(input_ref), 
    m_output_uuid(output_uuid),
    m_default_validator(std::move(default_validator))
{
}

ValidationInitializer::ValidationInitializer(ValidationInitializer&& validation_initializer) noexcept :
    m_name(std::move(validation_initializer.m_name)),
    m_input_ref(validation_initializer.m_input_ref),
    m_output_uuid(validation_initializer.m_output_uuid),
    m_default_validator(std::move(validation_initializer.m_default_validator))
{
}

std::wstring Validation::get_name() const
{
    return m_name;
}

ReportTypeRef Validation::get_input_ref() const
{
    return m_input_ref;
}

ReportTypeRef Validation::get_output_ref() const
{
    return m_output_ref;
}

ValidationRef Validation::get_ref() const
{
    return m_ref;
}

Validation::operator ValidationRef() const
{
    return get_ref();
}


Validation::Validation(ValidationRef ref, std::wstring name, ReportTypeRef input_ref, ReportTypeRef output_ref):
    m_ref(ref),
    m_name(std::move(name)),
    m_input_ref(input_ref),
    m_output_ref(output_ref)
{
}
    
Validations::Validations(Database& db, Basis& basis, Registry& registry):
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
    m_validation_dataset(std::make_unique<ValidationDataset>(m_db)),
    m_subject_validation_dataset(std::make_unique<SubjectValidationDataset>(m_db)),
    m_executive(nullptr),
    m_running(false)
{
    m_sources.connect_remove_source([this](SourceKey source_key, const std::vector<SourceKey>& removed_sources, Transaction& t){
        on_remove_sources(removed_sources, t);
    }, VALIDATION_PRIORITY);

    m_sources.connect_rerole_source([this](SourceRef source_ref, RoleRef prev_role, RoleRef next_role, Transaction& t){
        on_change_subject_role(source_ref, prev_role, next_role, t);
    }, VALIDATION_PRIORITY);

    m_roles.connect_remove_role([this](RoleKey role_key, Transaction& t){
        on_remove_role(role_key, t);
    }, VALIDATION_PRIORITY);

    m_roles.connect_replace_role([this](RoleRef role_ref, RoleRef prev_parent_ref, RoleRef next_parent_ref, Transaction& t){
        on_change_subject_role(role_ref, prev_parent_ref, next_parent_ref, t);
    }, VALIDATION_PRIORITY);

    m_collector.connect_update_current_report([this](CurrentReportRef current_report_ref, bool content_changed, Transaction& t){
        if (!content_changed)
            return;

        auto current_report = *current_report_ref;
        if (auto validation = find_by_output(current_report.get_report_type_ref()))
        {
            reset_validation_result(*validation, current_report.get_source_ref(), t);
        }
    }, VALIDATION_PRIORITY);   

    m_source_resources.connect_update_source_resource([this](const SourceResource& source_resource, Transaction& t){
        if (!m_running)
        {
            return;
        }
        
        if (auto report_type_ref = boost::get<ReportTypeRef>(&source_resource.m_resource_ref))
        {
            if (auto validation = find_by_output(*report_type_ref))
            {
                reset_validation_result(*validation, source_resource.m_source_ref, t);
            }
        }       
    }, VALIDATION_PRIORITY);

    m_source_resources.connect_remove_source_resource([this](SourceRef source_ref, ResourceRef resource_ref, Transaction& t){
        _ASSERT(m_running);

        if (auto report_type_ref = boost::get<ReportTypeRef>(&resource_ref))
        {
            if (auto validation = find_by_output(*report_type_ref))
            {
                reset_validation_result(*validation, source_ref, t);
            }
        }
    }, VALIDATION_PRIORITY);
}

Validations::~Validations()
{
}

void Validations::set_joined_queries(JoinedQueries* joined_queries)
{
    _ASSERT(!m_joined_queries);
    m_joined_queries = joined_queries;
}

void Validations::restore(Transaction& t)
{
    for (auto validation_data : m_db.query<ValidationData>())
    {
        auto input_ref = unpack(validation_data.m_input_uuid, &m_report_types);

        m_report_types.add_report_type(std::make_unique<ValidatedReportType>(
            validation_data.m_output_uuid,
            unpack(validation_data.m_name),
            [this, input_ref]{
                auto& input = dynamic_cast<const RegularReportType&>(*input_ref);
                return clone_doc(*input.get_def_doc());
            }), t
        );
    }
}

void Validations::run(Executive* executive, Transaction& t)
{
    transact_assign(m_validation_manager, std::make_unique<ValidationManager>(*this, *executive), t);
    transact_assign(m_executive, executive, t);
    transact_assign(m_running, true, t);

    t.connect_commit([this] {
        for (auto& source : m_sources.get_sources())
        {
            for (auto& validation : get_validations())
            {
                if (m_source_resources.find_source_resource(source, validation.get_output_ref()))
                {
                    m_validation_manager->start_validating(source, validation);
                }                
            }
        }
    });
}

ValidationRef Validations::add_validation(ValidationInitializer validation_initializer, Transaction& t)
{
    auto& input = dynamic_cast<const RegularReportType&>(*validation_initializer.m_input_ref);
    auto& output = m_report_types.add_report_type(std::make_unique<ValidatedReportType>(
        validation_initializer.m_output_uuid? *validation_initializer.m_output_uuid: stl_tools::gen_uuid(), 
        validation_initializer.m_name, clone_doc(*input.get_def_doc())),
        t);

    auto default_validator = validation_initializer.m_default_validator? 
         std::move(validation_initializer.m_default_validator):
         XML::create_validator(*input.get_def_doc());

    QueryJoining query_joining(input, output, std::move(default_validator));
    m_joined_queries->add_query_joining(query_joining, t);

    m_validation_dataset->persist({
        query_joining.get_key(),
        pack(validation_initializer.m_name),
        pack(input.get_ref()),
        pack(output.get_ref())
    }, t);

    return ValidationRef(this, ValidationKey(query_joining.get_key()));
}

std::vector<Validation> Validations::get_validations() const
{
    std::vector<Validation> validations;
    for (auto& validation_data : m_validation_dataset->query())
    {
        validations.push_back(unpack_validation_data(validation_data));
    }
    return validations;
}

Validation Validations::get_validation(ValidationKey validation_key) const
{
    ReadOnlyTransaction t(m_db);
    auto validation_data = m_validation_dataset->load(validation_key);
    return unpack_validation_data(validation_data);
}

void Validations::remove_validation(ValidationKey validation_key, Transaction& t)
{
    auto validation_data = m_validation_dataset->load(validation_key);
    m_joined_queries->remove_query_joining(QueryJoiningKey(validation_key), t);
    m_report_types.remove_report_type(validation_data.m_output_uuid, t);
    m_subject_validation_dataset->erase_validation_equal(validation_key, t);
    m_validation_dataset->erase(validation_key, t); 
}

boost::optional<Validation> Validations::find_validation(ValidationKey validation_key) const
{
    ReadOnlyTransaction t(m_db);    
    if (auto validation_data = m_validation_dataset->find(validation_key))
    {
        return unpack_validation_data(*validation_data);
    }
    return boost::none;
}

std::vector<Validation> Validations::find_by_input(ReportTypeRef input_ref) const
{
    ReadOnlyTransaction t(m_db);
    std::vector<Validation> validations;
    for (auto& validation_data : m_validation_dataset->find_by_input(input_ref.get_uuid()))
    {
        validations.push_back(unpack_validation_data(validation_data));
    }
    return validations;
}

boost::optional<Validation> Validations::find_by_output(ReportTypeRef output_ref) const
{
    ReadOnlyTransaction t(m_db);
    if (auto validation_data = m_validation_dataset->find_by_output(output_ref.get_uuid()))
    {
        return unpack_validation_data(*validation_data);
    }
    return boost::none;
}

bool Validations::is_validation_on(ValidationRef validation_ref, SubjectRef subject_ref)
{
    ReadOnlyTransaction t(m_db);
    auto validation_policy = get_validation_policy(validation_ref, subject_ref);

    if (validation_policy == ValidationPolicy::ENABLE_VALIDATION)
    {
        return true;
    }

    else if (validation_policy == ValidationPolicy::DISABLE_VALIDATION)
    {
        return false;
    }

    else if (auto base_ref = get_base_ref(subject_ref))
    {
        return is_validation_on(validation_ref, *base_ref);
    }

    return true;
}

void Validations::set_validation_policy(ValidationRef validation_ref, SubjectRef subject_ref, ValidationPolicy validation_policy, Transaction& t)
{
    auto subject_validation_data = m_subject_validation_dataset->get(pack(subject_ref), validation_ref.get_key());
    if (subject_validation_data.m_validation_policy == validation_policy)
    {
        return;
    }

    subject_validation_data.m_validation_policy = validation_policy;
    m_subject_validation_dataset->put(subject_validation_data, t);

    if (auto source_ref = boost::get<SourceRef>(&subject_ref))
    {
        m_source_resources.check_service_request(*source_ref, (*validation_ref).get_output_ref(), t);
        reset_validation_result(validation_ref, *source_ref, t);
        return;
    }

    _ASSERT(boost::get<RoleRef>(&subject_ref));
    for (auto derived_ref : get_derived_refs(m_registry, subject_ref))
    {
        forward_validation_policy(validation_ref, derived_ref, t);
    }
}

ValidationPolicy Validations::get_validation_policy(ValidationRef validation_ref, SubjectRef subject_ref) const
{
    return m_subject_validation_dataset->get(pack(subject_ref), validation_ref.get_key()).m_validation_policy;
}

void Validations::forward_validation_policy(ValidationRef validation_ref, SubjectRef subject_ref, Transaction& t)
{
    auto validation_policy = get_validation_policy(validation_ref, subject_ref);
    if (validation_policy != ValidationPolicy::DEFAULT_POLICY)
    {
        return;
    }

    if (auto source_ref = boost::get<SourceRef>(&subject_ref))
    {
        m_source_resources.check_service_request(*source_ref, (*validation_ref).get_output_ref(), t);
        reset_validation_result(validation_ref, *source_ref, t);
        return;
    }

    _ASSERT(boost::get<RoleRef>(&subject_ref));
    for (auto derived_ref : get_derived_refs(m_registry, subject_ref))
    {
        forward_validation_policy(validation_ref, derived_ref, t);
    }
}

bool Validations::has_validator(ValidationRef validation_ref, SubjectRef subject_ref) const
{
    auto query_joining = m_joined_queries->get_query_joining(QueryJoiningKey(validation_ref.get_key()));
    return m_joined_queries->has_subject_query(query_joining, subject_ref);
}

std::shared_ptr<const XmlQueryDoc> Validations::get_validator(ValidationRef validation_ref, SubjectRef subject_ref) const
{
    auto query_joining = m_joined_queries->get_query_joining(QueryJoiningKey(validation_ref.m_key));
    auto validator = m_joined_queries->get_subject_query(query_joining, subject_ref);
    if (!validator)
    {
        auto& output = dynamic_cast<const ValidatedReportType&>(*query_joining.get_output_ref());
        validator = XML::create_validator(*output.get_def_doc());
    }
    return validator;
}

void Validations::set_validator(ValidationRef validation_ref, SubjectRef subject_ref, std::unique_ptr<XmlQueryDoc> validator, Transaction& t)
{
    auto query_joining = m_joined_queries->get_query_joining(QueryJoiningKey(validation_ref.m_key));
    m_joined_queries->set_subject_query(query_joining, subject_ref, std::move(validator), t);
}

std::shared_ptr<const XmlQueryDoc> Validations::get_base_validator(ValidationRef validation_ref, SubjectRef subject_ref) const
{
    auto query_joining = m_joined_queries->get_query_joining(QueryJoiningKey(validation_ref.m_key));
    return m_joined_queries->get_base_query(query_joining, subject_ref);
}

std::shared_ptr<const XmlDefDoc> Validations::get_validator_def(ValidationRef validation_ref) const
{
    auto query_joining = m_joined_queries->get_query_joining(QueryJoiningKey(validation_ref.m_key));
    return query_joining.get_query_def();
}

ValidationResult Validations::get_validation_result(ValidationRef validation_ref, SourceRef source_ref) const
{
    return m_subject_validation_dataset->get(pack(source_ref), validation_ref.get_key()).m_validation_result;
}

UUID Validations::get_validation_input(ValidationRef validation_ref, SourceRef source_ref) const
{
    return m_subject_validation_dataset->get(pack(source_ref), validation_ref.get_key()).m_input_uuid;
}

void Validations::reset_validation_result(ValidationRef validation_ref, SourceRef source_ref, Transaction& t)
{
    t.connect_commit([source_ref, validation_ref, this]() {
        m_validation_manager->start_validating(*source_ref, validation_ref);
    });
}

Validation Validations::unpack_validation_data(const ValidationData& validation_data) const
{
    return Validation{
        ValidationRef(this, ValidationKey(validation_data.m_key)),
        unpack(validation_data.m_name),
        unpack(validation_data.m_input_uuid, &m_report_types),
        unpack(validation_data.m_output_uuid, &m_report_types)
    };
}

void Validations::set_validation_result(ValidationRef validation_ref, SourceRef source_ref, ValidationResult validation_result, UUID input_uuid)
{
    Transaction t(m_db);
    set_validation_result(validation_ref, source_ref, validation_result, input_uuid, t);
    t.commit();
}

void Validations::set_validation_result(ValidationRef validation_ref, SourceRef source_ref, ValidationResult validation_result, UUID input_uuid, Transaction& t)
{       
    auto subject_validation_data = m_subject_validation_dataset->get(pack(source_ref), validation_ref.get_key());
    if (subject_validation_data.m_validation_result == validation_result && subject_validation_data.m_input_uuid == input_uuid)
    {
        return;
    }

    subject_validation_data.m_validation_result = validation_result;
    subject_validation_data.m_input_uuid = input_uuid;
    m_subject_validation_dataset->put(subject_validation_data, t);
    m_update_validation_result_sig(validation_ref, source_ref, validation_result, t);
}

void Validations::on_change_subject_role(SubjectRef subject_ref, RoleRef prev_role, RoleRef next_role, Transaction& t)
{
    for (auto& validation : get_validations())
    {
        if (is_validation_on(validation, prev_role) != is_validation_on(validation, next_role))
        {
            forward_validation_policy(validation, subject_ref, t);
        }
    }
}

void Validations::on_remove_sources(const std::vector<SourceKey>& removed_sources, Transaction& t)
{
    m_subject_validation_dataset->erase_source_equal(removed_sources, t);
}

void Validations::on_remove_role(RoleKey role_key, Transaction& t)
{
    m_subject_validation_dataset->erase_subject_equal(role_key, t);
}

}} //namespace TR { namespace Core {