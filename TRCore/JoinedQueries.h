#pragma once
#include "ServicesDecl.h"
namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4521)
#pragma warning(disable:4522)
struct QueryJoiningData;
class QueryJoiningDataset;
class SubjectQueryDataset;
class SubjectJoinedQueryDataset;
struct SubjectQuery;
struct JoinedQuery;

class QueryJoining
{
public:
    friend class JoinedQueries;
    QueryJoining(ReportTypeRef input_ref, ReportTypeRef output_ref, std::unique_ptr<XmlQueryDoc> default_query = nullptr);
    ~QueryJoining();

    QueryJoiningKey                         get_key() const;
    std::wstring                            get_name() const;
    ReportTypeRef                           get_input_ref() const;
    ReportTypeRef                           get_output_ref() const;

    std::shared_ptr<const XmlQueryDoc>      get_default_query() const;
    std::shared_ptr<const XmlDefDoc>        get_query_def() const;
    
private:    
    QueryJoining(QueryJoiningKey key, ReportTypeRef input_ref, ReportTypeRef output_ref, stl_tools::lazy_unique_ptr<XmlQueryDoc> default_query);
    
    QueryJoiningKey m_key;
    ReportTypeRef m_input_ref;
    ReportTypeRef m_output_ref;

    stl_tools::lazy_shared_ptr<const XmlQueryDoc> m_default_query;
};

class JoinedQueries
{
public:
    static const Priority JOINED_QUERIES_PRIORITY = 1400;

    JoinedQueries(Database& db, Basis& basis, Registry& registry);
    ~JoinedQueries();

    void                                    restore(Transaction& t);
    void                                    run(Executive* executive, Transaction& t);

    void                                    add_query_joining(QueryJoining& query_joining, Transaction& t);
    QueryJoining                            get_query_joining(QueryJoiningKey query_joining_key) const;
    std::vector<QueryJoining>               get_query_joinings() const;
    void                                    remove_query_joining(QueryJoiningKey query_joining_key, Transaction& t);

    std::vector<QueryJoining>               find_by_input(ReportTypeRef input_ref) const;
    boost::optional<QueryJoining>           find_by_output(ReportTypeRef output_ref) const;
    
    bool                                    has_subject_query(const QueryJoining& query_joining, SubjectRef subject_ref) const;
    std::shared_ptr<const XmlQueryDoc>      get_subject_query(const QueryJoining& query_joining, SubjectRef subject_ref) const;
    void                                    set_subject_query(const QueryJoining& query_joining, SubjectRef subject_ref, std::unique_ptr<XmlQueryDoc> query_doc, Transaction& t);
    
    std::shared_ptr<const XmlQueryDoc>      get_base_query(const QueryJoining& query_joining, SubjectRef subject_ref) const;
    std::shared_ptr<const XmlQueryDoc>      get_joined_query(const QueryJoining& query_joining, SubjectRef subject_ref) const;
    
private:
    class QueryJoiningManager;

    void                                    set_subject_query(const SubjectQuery& subject_query, Transaction& t);
    void                                    reset_subject_query(const QueryJoining& query_joining, SubjectRef subject_ref, Transaction& t);
    boost::optional<SubjectQuery>           find_subject_query(const QueryJoining& query_joining, SubjectRef subject_ref) const;
    
    boost::optional<JoinedQuery>            find_joined_query(const QueryJoining& query_joining, SubjectRef subject_ref) const;
    JoinedQuery                             find_base_query(const QueryJoining& query_joining, SubjectRef subject_ref) const;

    void                                    set_joined_query(const JoinedQuery& subject_query, Transaction& t);
    void                                    reset_joined_query(const QueryJoining& query_joining, SubjectRef subject_ref, Transaction& t);
   
    void                                    invalidate_joined_query(const QueryJoining& query_joining, SubjectRef subject_ref, Transaction& t);

    QueryJoining                            unpack_query_joining(const QueryJoiningData& query_joining_data) const;
    QueryJoiningData                        pack_query_joining(const QueryJoining& query_joining) const;

    void                                    on_change_subject_role(SubjectRef subject_ref, RoleRef prev_role_ref, RoleRef next_role_ref, Transaction& t);
    void                                    on_remove_sources(const std::vector<SourceKey>& removed_sources, Transaction& t);
    void                                    on_remove_role(RoleKey role_key, Transaction& t);

    Database& m_db;
    Basis& m_basis;
    ReportTypes& m_report_types;
    Registry& m_registry;
    Sources& m_sources; 
    Roles& m_roles;
    ResourcePolicies& m_resource_policies;
    Executive* m_executive;

    std::unique_ptr<QueryJoiningDataset> m_query_joining_dataset;
    std::unique_ptr<SubjectQueryDataset> m_subject_query_dataset;
    std::unique_ptr<SubjectJoinedQueryDataset> m_subject_joined_query_dataset;
    std::unique_ptr<QueryJoiningManager> m_query_joining_manager;
};

#pragma warning(pop)
}} //namespace TR { namespace Core {