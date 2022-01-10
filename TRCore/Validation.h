#pragma once
#include "ServicesDecl.h"
namespace TR {namespace Core {
class ValidationDataset;
class SubjectValidationDataset;
struct ValidationData;

struct ValidationInitializer
{
    ValidationInitializer(std::wstring name, ReportTypeRef input_ref, std::unique_ptr<XmlQueryDoc> default_validator = nullptr);
    ValidationInitializer(std::wstring name, ReportTypeRef input_ref, ReportTypeUUID output_uuid, std::unique_ptr<XmlQueryDoc> default_validator = nullptr);
    ValidationInitializer(ValidationInitializer&& validation) noexcept;

    std::wstring                        m_name;
    ReportTypeRef                       m_input_ref;
    boost::optional<ReportTypeUUID>     m_output_uuid;
    std::unique_ptr<XmlQueryDoc>        m_default_validator;
};

class Validation
{
public:
    friend class Validations;
    std::wstring                            get_name() const;
    ReportTypeRef                           get_input_ref() const;
    ReportTypeRef                           get_output_ref() const;
    ValidationRef                           get_ref() const;
    operator ValidationRef() const;

private:
    Validation(ValidationRef ref, std::wstring name, ReportTypeRef input_ref, ReportTypeRef output_ref);

    ValidationRef m_ref;
    std::wstring m_name;
    ReportTypeRef m_input_ref;
    ReportTypeRef m_output_ref;
};

enum class ValidationPolicy { DEFAULT_POLICY = 0, ENABLE_VALIDATION, DISABLE_VALIDATION };
enum class ValidationResult { UNDEFINED = 0, VALIDATION_SUCCEEDED, VALIDATION_FAILED };

class Validations
{
public:
    static const Priority VALIDATION_PRIORITY = 1800;

    Validations(Database& db, Basis& basis, Registry& registry);
    ~Validations();

    void                                        set_joined_queries(JoinedQueries* joined_queries);
    void                                        restore(Transaction& t);
    void                                        run(Executive* executive, Transaction& t);

    // create/remove    
    ValidationRef                               add_validation(ValidationInitializer validation_initializer, Transaction& t);
    std::vector<Validation>                     get_validations() const;
    Validation                                  get_validation(ValidationKey validation_key) const;
    void                                        remove_validation(ValidationKey validation_key, Transaction& t);
    boost::optional<Validation>                 find_validation(ValidationKey validation_key) const;
    std::vector<Validation>                     find_by_input(ReportTypeRef input_ref) const;
    boost::optional<Validation>                 find_by_output(ReportTypeRef output_ref) const;

    // settings 
    bool                                        is_validation_on(ValidationRef validation_ref, SubjectRef subject_ref);
    void                                        set_validation_policy(ValidationRef validation_ref, SubjectRef subject_ref, ValidationPolicy validation_policy, Transaction& t);
    ValidationPolicy                            get_validation_policy(ValidationRef validation_ref, SubjectRef subject_ref) const;
    void                                        forward_validation_policy(ValidationRef validation_ref, SubjectRef subject_ref, Transaction& t);

    bool                                        has_validator(ValidationRef validation_ref, SubjectRef subject_ref) const;
    std::shared_ptr<const XmlQueryDoc>          get_validator(ValidationRef validation_ref, SubjectRef subject_ref) const;
    void                                        set_validator(ValidationRef validation_ref, SubjectRef subject_ref, std::unique_ptr<XmlQueryDoc> validator, Transaction& t);
    std::shared_ptr<const XmlQueryDoc>          get_base_validator(ValidationRef validation_ref, SubjectRef subject_ref) const;
    std::shared_ptr<const XmlDefDoc>            get_validator_def(ValidationRef validation_ref) const;
    
    // result
    ValidationResult                            get_validation_result(ValidationRef validation_ref, SourceRef source_ref) const;

    template<class T>
    void connect_update_validation_result(T slot, Priority priority) { m_update_validation_result_sig.connect(slot, priority); }

private:
    friend class ValidationJob;
    class ValidationManager;

    UUID                                        get_validation_input(ValidationRef validation_ref, SourceRef source_ref) const;
    void                                        reset_validation_result(ValidationRef validation_ref, SourceRef source_ref, Transaction& t);
    Validation                                  unpack_validation_data(const ValidationData& validation_data) const;

    void                                        set_validation_result(ValidationRef validation_ref, SourceRef source_ref, ValidationResult validation_result, UUID input_uuid);
    void                                        set_validation_result(ValidationRef validation_ref, SourceRef source_ref, ValidationResult validation_result, UUID input_uuid, Transaction& t);

    void                                        on_change_subject_role(SubjectRef subject_ref, RoleRef prev_role, RoleRef next_role, Transaction& t);
    void                                        on_remove_sources(const std::vector<SourceKey>& removed_sources, Transaction& t);
    void                                        on_remove_role(RoleKey role_key, Transaction& t);

    Database& m_db;
    Basis& m_basis;
    Registry& m_registry;
    ReportTypes& m_report_types;
    Sources& m_sources;
    Roles& m_roles;
    Collector& m_collector;
    ResourcePolicies& m_resource_policies;
    SourceResources& m_source_resources;
    JoinedQueries* m_joined_queries;
    Executive* m_executive;
    bool m_running;
    
    std::unique_ptr<ValidationDataset> m_validation_dataset;
    std::unique_ptr<SubjectValidationDataset> m_subject_validation_dataset;
    std::unique_ptr<ValidationManager> m_validation_manager;

    OrderedSignal<void(ValidationRef validation_ref, SourceRef source_ref, ValidationResult validation_result, Transaction& t)> m_update_validation_result_sig;
};

}} //namespace TR { namespace Core {