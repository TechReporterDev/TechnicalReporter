#pragma once
#include "ServicesDecl.h"
namespace TR { namespace Core {
class IntegrityCheckingDataset;
struct IntegrityCheckingInput;

enum class IntegrityCheckingPolicy { DEFAULT_POLICY = 0, ENABLE_CHECKING, DISABLE_CHECKING };
enum class IntegrityCheckingPatternChoise { DEFAULT_PATTERN = 0, CUSTOM_PATTERN };
enum class IntegrityCheckingResult { UNDEFINED = 0, MATCHED, MISMATCHED };

class IntegrityChecking
{
public:
    static const Priority INTEGRITY_CHECKING_PRIORITY = 1600;

    IntegrityChecking(Database& db, Basis& basis, Registry& registry);
    ~IntegrityChecking();

    void                            run(Executive* executive, Transaction& t);

    bool                            is_checking_on(SubjectRef subject_ref, ReportTypeRef report_type_ref) const;
    void                            set_checking_policy(SubjectRef subject_ref, ReportTypeRef report_type_ref, IntegrityCheckingPolicy policy, Transaction& t);
    IntegrityCheckingPolicy         get_checking_policy(SubjectRef subject_ref, ReportTypeRef report_type_ref) const;

    void                            set_pattern_choise(SubjectRef subject_ref, ReportTypeRef report_type_ref, IntegrityCheckingPatternChoise pattern_choise, Transaction& t);
    IntegrityCheckingPatternChoise  get_pattern_choise(SubjectRef subject_ref, ReportTypeRef report_type_ref) const;
    void                            set_pattern(SubjectRef subject_ref, ReportTypeRef report_type_ref, std::shared_ptr<Content> pattern, Transaction& t);
    void                            set_pattern(SubjectRef subject_ref, ReportTypeRef report_type_ref, std::shared_ptr<Content> pattern);
    bool                            has_pattern(SubjectRef subject_ref, ReportTypeRef report_type_ref) const;
    std::shared_ptr<Content>        get_pattern(SubjectRef subject_ref, ReportTypeRef report_type_ref) const;
    std::shared_ptr<Content>        get_effective_pattern(SubjectRef subject_ref, ReportTypeRef report_type_ref) const;

    //checking result
    IntegrityCheckingResult         get_checking_result(SourceRef source_ref, ReportTypeRef report_type_ref) const;

    template<class T>
    void connect_update_checking_result(T slot, Priority priority) { m_update_checking_result_sig.connect(slot, priority); }

private:
    friend class IntegrityCheckingJob;
    class IntegrityCheckingManager;

    IntegrityCheckingInput          get_checking_input(SourceRef source_ref, ReportTypeRef report_type_ref) const;
    UUID                            get_effective_pattern_uuid(SubjectRef subject_ref, ReportTypeRef report_type_ref) const;

    void                            set_pattern(SubjectRef subject_ref, ReportTypeRef report_type_ref, std::shared_ptr<Content> pattern, UUID pattern_uuid, Transaction& t);
    void                            reset_pattern(SubjectRef subject_ref, ReportTypeRef report_type_ref, Transaction& t);

    void                            set_checking_result(SourceRef source_ref, ReportTypeRef report_type_ref, IntegrityCheckingResult checking_result, const IntegrityCheckingInput& input, Transaction& t);
    void                            set_checking_result(SourceRef source_ref, ReportTypeRef report_type_ref, IntegrityCheckingResult checking_result, const IntegrityCheckingInput& input);

    void                            forward_checking_policy(SubjectRef subject_ref, ReportTypeRef report_type_ref, Transaction& t);
    void                            forward_pattern(SubjectRef subject_ref, ReportTypeRef report_type_ref, Transaction& t);

    void                            invalidate_checking_result(SourceRef source_ref, ReportTypeRef report_type_ref, Transaction& t);

    void                            on_change_subject_role(SubjectRef subject_ref, RoleRef prev_role, RoleRef next_role, Transaction& t);
    void                            on_remove_report_type(ReportTypeUUID report_type_uuid, Transaction& t);
    void                            on_remove_sources(const std::vector<SourceKey>& removed_sources, Transaction& t);
    void                            on_remove_role(RoleKey role_key, Transaction& t);

    Database& m_db;
    Basis& m_basis;
    Registry& m_registry;
    ReportTypes& m_report_types;
    Sources& m_sources;
    Roles& m_roles;
    Collector& m_collector;
    ResourcePolicies& m_resource_policies;
    SourceResources& m_source_resources;
    Executive* m_executive;
    bool m_running;

    std::unique_ptr<IntegrityCheckingDataset> m_integrity_checking_dataset;    
    std::unique_ptr<IntegrityCheckingManager> m_integrity_checking_manager;

    OrderedSignal<void(SourceRef source_ref, ReportTypeRef report_type_ref, IntegrityCheckingResult checking_result, Transaction& t)> m_update_checking_result_sig;
};

}} //namespace TR { namespace Core {