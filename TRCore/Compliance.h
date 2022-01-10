#pragma once
#include "ServicesDecl.h"
namespace TR { namespace Core {
class ComplianceDataset;
class SubjectComplianceDataset;
struct SubjectComplianceIdentity;
struct SubjectComplianceData;

class Compliance
{
public:
    friend class Compliances;
    Compliance(std::wstring name, ReportTypeRef base_report_type_ref);
    
    std::wstring                        get_name() const;
    ReportTypeRef                       get_base_report_type_ref() const;
    ReportTypeRef                       get_output_ref() const;
    ComplianceRef                       get_ref() const;
    operator ComplianceRef() const;

private:
    Compliance(ComplianceRef ref, std::wstring name, ReportTypeRef base_report_type_ref, ReportTypeRef output_ref);

    boost::optional<ComplianceRef> m_ref;
    std::wstring m_name;
    ReportTypeRef m_base_report_type_ref;
    boost::optional<ReportTypeRef> m_output_ref;
};

enum class ComplianceCheckingPolicy { DEFAULT_POLICY = 0, ENABLE_CHECKING, DISABLE_CHECKING };
enum class ComplianceCheckingResult { UNDEFINED = 0, CHECKING_SUCCEEDED, CHECKING_FAILED };

class Compliances
{
public:
    static const Priority COMPLIANCE_PRIORITY = 1700;

    Compliances(Database& db, Basis& basis, Registry& registry);
    ~Compliances();

    void                                        set_joined_queries(JoinedQueries* joined_queries);
    void                                        restore(Transaction& t);
    void                                        run(Executive* executive, Transaction& t);

    // create/remove    
    void                                        add_compliance(Compliance& compliance, Transaction& t);
    std::vector<Compliance>                     get_compliances() const;
    Compliance                                  get_compliance(ComplianceKey compliance_key) const; 
    void                                        remove_compliance(ComplianceKey compliance_key, Transaction& t);
    boost::optional<Compliance>                 find_compliance(ComplianceKey compliance_key) const;
    boost::optional<Compliance>                 find_by_input(ReportTypeRef base_report_type_ref) const;
    boost::optional<Compliance>                 find_by_output(ReportTypeRef output_ref) const;

    // checking policy  
    bool                                        is_checking_on(ComplianceRef compliance_ref, SubjectRef subject_ref);
    void                                        set_checking_policy(ComplianceRef compliance_ref, SubjectRef subject_ref, ComplianceCheckingPolicy checking_policy, Transaction& t);
    ComplianceCheckingPolicy                    get_checking_policy(ComplianceRef compliance_ref, SubjectRef subject_ref) const;
    void                                        forward_checking_policy(ComplianceRef compliance_ref, SubjectRef subject_ref, Transaction& t);

    // setup
    bool                                        has_compliance_setup(ComplianceRef compliance_ref, SubjectRef subject_ref) const;
    std::shared_ptr<const XmlComplianceSetup>   get_compliance_setup(ComplianceRef compliance_ref, SubjectRef subject_ref) const;
    void                                        set_compliance_setup(ComplianceRef compliance_ref, SubjectRef subject_ref, std::unique_ptr<XmlComplianceSetup> compliance_setup, Transaction& t);
    std::shared_ptr<const XmlComplianceSetup>   get_base_setup(ComplianceRef compliance_ref, SubjectRef subject_ref) const;
    std::shared_ptr<const XmlComplianceDef>     get_compliance_def(ComplianceRef compliance_ref) const;
    
    // result   
    ComplianceCheckingResult                    get_checking_result(ComplianceRef compliance_ref, SourceRef source_ref) const;
    
    template<class T>
    void connect_update_checking_result(T slot, Priority priority) { m_update_checking_result_sig.connect(slot, priority); }

private:
    friend class ComplianceCheckingJob;
    class ComplianceCheckingManager;
    UUID                                        get_checking_input(ComplianceRef compliance_ref, SourceRef source_ref) const;
    void                                        invalidate_checking_result(ComplianceRef compliance_ref, SourceRef source_ref, Transaction& t);

    void                                        set_checking_result(ComplianceRef compliance_ref, SourceRef source_ref, ComplianceCheckingResult checking_result, UUID input_uuid);
    void                                        set_checking_result(ComplianceRef compliance_ref, SourceRef source_ref, ComplianceCheckingResult checking_result, UUID input_uuid, Transaction& t);

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

    std::unique_ptr<ComplianceDataset> m_compliance_dataset;
    std::unique_ptr<SubjectComplianceDataset> m_subject_compliance_dataset;
    std::unique_ptr<ComplianceCheckingManager> m_compliance_checking_manager;

    OrderedSignal<void(ComplianceRef compliance_ref, SourceRef source_ref, ComplianceCheckingResult checking_result, Transaction& t)> m_update_checking_result_sig;
};

}} //namespace TR { namespace Core {