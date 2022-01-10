#include "stdafx.h"
#include "JoinedQueries.h"
#include "Basis.h"
#include "Registry.h"
#include "Executive.h"
#include "JobManager.h"
#include "Transaction.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
#include "ServicesPacking.h"
#include "CustomReportDataset.h"
namespace TR { namespace Core {

static UUID QUERY_JOINING_SELECTION_UUID = stl_tools::gen_uuid("31E12F81-302B-49A3-AC88-B9BA9BD99B44");

struct SubjectQuery
{
    SubjectQuery(SubjectRef subject, QueryJoiningKey query_joining_key, UUID uuid, std::shared_ptr<const XmlQueryDoc> query_doc):
        m_subject(subject),
        m_query_joining_key(query_joining_key),
        m_uuid(std::move(uuid)),
        m_query_doc(query_doc)
    {
    }

    SubjectRef m_subject;
    QueryJoiningKey m_query_joining_key;
    UUID m_uuid;
    std::shared_ptr<const XmlQueryDoc> m_query_doc;    
};

struct JoinedQuery
{
    JoinedQuery(SubjectRef subject, QueryJoiningKey query_joining_key, UUID uuid, std::shared_ptr<const XmlQueryDoc> query_doc, UUID base_query_uuid, UUID subject_query_uuid):
        m_subject(subject),
        m_query_joining_key(query_joining_key),
        m_uuid(std::move(uuid)),
        m_query_doc(query_doc),
        m_base_query_uuid(base_query_uuid),
        m_subject_query_uuid(subject_query_uuid)
    {
    }

    SubjectRef m_subject;
    QueryJoiningKey m_query_joining_key;
    UUID m_uuid;
    std::shared_ptr<const XmlQueryDoc> m_query_doc;

    UUID m_base_query_uuid;
    UUID m_subject_query_uuid;
};

class JoinedQueries::QueryJoiningManager
{
public:
    class QueryJoiningJob: public Job, public std::enable_shared_from_this<QueryJoiningJob>
    {
    public:
        friend class QueryJoiningManager;
        class Routine
        {
        public:
            Routine(std::shared_ptr<QueryJoiningJob> job):
                m_job(job),
                m_joined_queries(job->m_job_manager.m_joined_queries),
                m_registry(job->m_job_manager.m_joined_queries.m_registry),
                m_subject_query_uuid(stl_tools::null_uuid()),
                m_base_query_uuid(stl_tools::null_uuid())
            {
            }

            std::function<std::unique_ptr<XmlQueryDoc>()> prepare_functor()
            {
                auto job = m_job.lock();
                if (!job)
                {
                    return nullptr;
                }

                auto subject_ref = get_subject_ref(m_registry, job->m_subject_key);
                auto query_joining = m_joined_queries.get_query_joining(job->m_query_joining_key);
                m_query_def = query_joining.get_query_def();

                auto subject_query = m_joined_queries.find_subject_query(query_joining, subject_ref);

                if (!subject_query)
                {
                    if (m_joined_queries.find_joined_query(query_joining, subject_ref))
                    {
                        Transaction t(m_joined_queries.m_db);
                        m_joined_queries.reset_joined_query(query_joining, subject_ref, t);
                        t.commit();                       
                    }

                    return nullptr;
                }

                auto base_query = m_joined_queries.find_base_query(query_joining, subject_ref);

                if (auto joined_query = m_joined_queries.find_joined_query(query_joining, subject_ref))
                {
                    if (joined_query->m_subject_query_uuid == subject_query->m_uuid &&
                        joined_query->m_base_query_uuid == base_query.m_uuid)
                    {
                        return nullptr;
                    }
                }

                m_subject_query_uuid = subject_query->m_uuid;
                m_base_query_uuid = base_query.m_uuid;

                return [subject_query = subject_query->m_query_doc, base_query = base_query.m_query_doc]() -> std::unique_ptr<XmlQueryDoc>
                {               
                    return XML::join_queries(*base_query, *subject_query);
                };
            }

            void on_completed(std::unique_ptr<XmlQueryDoc> joined_query) const
            {
                auto job = m_job.lock();
                if (!job)
                {
                    return;
                }

                Transaction t(m_joined_queries.m_db);

                auto subject_ref = get_subject_ref(m_registry, job->m_subject_key);
                auto query_joining = m_joined_queries.get_query_joining(job->m_query_joining_key);

                m_joined_queries.set_joined_query({
                    subject_ref, 
                    query_joining.get_key(),
                    stl_tools::gen_uuid(),
                    std::move(joined_query),                    
                    m_base_query_uuid,
                    m_subject_query_uuid
                    }, t);

                t.commit();

                job->m_job_manager.m_executive.run_processing(Routine(job));
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
            JoinedQueries& m_joined_queries;
            Registry& m_registry;
            std::weak_ptr<QueryJoiningJob> m_job;
            std::shared_ptr<const XmlDefDoc> m_query_def;
            UUID m_subject_query_uuid;
            UUID m_base_query_uuid;
        };

        QueryJoiningJob(QueryJoiningManager& job_manager, SubjectRef subject_ref, const QueryJoining& query_joining):
            m_job_manager(job_manager),
            m_query_joining_key(query_joining.get_key()),
            m_subject_key(get_subject_key(subject_ref))
        {
        }

    protected:
        using Job::emit_completed;
        using Job::emit_failed;

        //override Job
        virtual void do_run()
        {
            m_job_manager.m_executive.run_processing(Routine(shared_from_this()));
        }
        
    private:
        QueryJoiningManager& m_job_manager;
        QueryJoiningKey m_query_joining_key;
        SubjectKey m_subject_key;
    };

    QueryJoiningManager(JoinedQueries& joined_queries, Executive& executive):
        m_joined_queries(joined_queries),
        m_executive(executive)
    {
    }

    void start_query_joining(SubjectRef subject_ref, const QueryJoining& query_joining)
    {
        auto starnd = std::make_pair(get_subject_key(subject_ref), query_joining.get_key());
        if(m_job_manager.is_empty(starnd))
        {
            m_job_manager.run_job(starnd, std::make_shared<QueryJoiningJob>(*this, subject_ref, query_joining));
        }
    }

private:
    JoinedQueries& m_joined_queries;
    Executive& m_executive;
    JobManager<std::pair<SubjectKey, QueryJoiningKey>> m_job_manager;
};

class QueryJoiningSelection: public Selection
{
public:
    QueryJoiningSelection(const QueryJoining& query_joining, JoinedQueries& joined_queries):
        Selection(QUERY_JOINING_SELECTION_UUID),
        m_query_joining(query_joining),
        m_joined_queries(joined_queries)
    {
    }

    virtual SelectionPlan get_plan() const override
    {
        return SelectionPlan(m_query_joining.get_input_ref(), m_query_joining.get_output_ref());
    }

    virtual Functor prepare_functor(SourceRef source_ref) const override
    {
        auto& output = dynamic_cast<const RegularReportType&>(*m_query_joining.get_output_ref());
        auto make_content = output.prepare_make_content_from_xml();
        auto query_doc = m_joined_queries.get_joined_query(m_query_joining, source_ref);

        return[make_content, query_doc](std::vector<std::shared_ptr<Content>>& inputs){
            _ASSERT(inputs.size() == 1);
            auto& regular_content = dynamic_cast<RegularContent&>(*inputs[0]);
            return make_content(do_query(*query_doc, regular_content.as_regular_doc()));
        };
    }

private:
    QueryJoining m_query_joining;
    JoinedQueries& m_joined_queries;
};

QueryJoining::QueryJoining(ReportTypeRef input_ref, ReportTypeRef output_ref, std::unique_ptr<XmlQueryDoc> default_query):
    m_key(0),
    m_input_ref(input_ref),
    m_output_ref(output_ref),
    m_default_query(std::move(default_query))
{
}

QueryJoining::~QueryJoining()
{
}

QueryJoiningKey QueryJoining::get_key() const
{
    return m_key;
}

ReportTypeRef QueryJoining::get_input_ref() const
{
    return m_input_ref;
}

ReportTypeRef QueryJoining::get_output_ref() const
{
    return m_output_ref;
}

std::shared_ptr<const XmlQueryDoc> QueryJoining::get_default_query() const
{
    return m_default_query.share();
}

std::shared_ptr<const XmlDefDoc> QueryJoining::get_query_def() const
{
    auto& input = dynamic_cast<const RegularReportType&>(*m_input_ref);
    return input.get_def_doc();
}

QueryJoining::QueryJoining(QueryJoiningKey key, ReportTypeRef input_ref, ReportTypeRef output_ref, stl_tools::lazy_unique_ptr<XmlQueryDoc> default_query):
    m_key(key),
    m_input_ref(input_ref),
    m_output_ref(output_ref),
    m_default_query(std::move(default_query))
{
}

JoinedQueries::JoinedQueries(Database& db, Basis& basis, Registry& registry):
    m_db(db),
    m_basis(basis),
    m_registry(registry),
    m_report_types(basis.m_report_types),
    m_sources(registry.m_sources),
    m_roles(registry.m_roles),
    m_resource_policies(registry.m_resource_policies),
    m_query_joining_dataset(std::make_unique<QueryJoiningDataset>(m_db)),
    m_subject_query_dataset(std::make_unique<SubjectQueryDataset>(m_db)),
    m_subject_joined_query_dataset(std::make_unique<SubjectJoinedQueryDataset>(m_db)),
    m_executive(nullptr)
{
    m_sources.connect_rerole_source([this](SourceRef source_ref, RoleRef prev_role, RoleRef next_role, Transaction& t){
        on_change_subject_role(source_ref, prev_role, next_role, t);
    }, JOINED_QUERIES_PRIORITY);

    m_sources.connect_remove_source([this](SourceKey source_key, const std::vector<SourceKey>& removed_sources, Transaction& t){
        on_remove_sources(removed_sources, t);
    }, JOINED_QUERIES_PRIORITY);

    m_roles.connect_replace_role([this](RoleRef role_ref, RoleRef prev_parent_ref, RoleRef next_parent_ref, Transaction& t){
        on_change_subject_role(role_ref, prev_parent_ref, next_parent_ref, t);
    }, JOINED_QUERIES_PRIORITY);

    m_roles.connect_remove_role([this](RoleKey role_key, Transaction& t){
        on_remove_role(role_key, t);
    }, JOINED_QUERIES_PRIORITY);
}

JoinedQueries::~JoinedQueries()
{
}

void JoinedQueries::restore(Transaction& t)
{
    for (auto& query_joining_data : m_query_joining_dataset->query())
    {
        auto query_joining = unpack_query_joining(query_joining_data);
        m_basis.m_transformations.m_selections.add_selection(std::make_shared<QueryJoiningSelection>(query_joining, *this), t);
    }
}

void JoinedQueries::run(Executive* executive, Transaction& t)
{
    transact_assign(m_executive, executive, t);
    transact_assign(m_query_joining_manager, std::make_unique<QueryJoiningManager>(*this, *executive), t);

    t.connect_commit([this] {
        for (auto query_joining : get_query_joinings())
        {
            for (auto subject_ref : get_subject_refs(m_registry))
            {
                m_query_joining_manager->start_query_joining(subject_ref, query_joining);
            }
        }
    });
}

void JoinedQueries::add_query_joining(QueryJoining& query_joining, Transaction& t)
{
    query_joining.m_key = m_query_joining_dataset->persist(pack_query_joining(query_joining), t);
    m_basis.m_transformations.m_selections.add_selection(std::make_shared<QueryJoiningSelection>(query_joining, *this), t);
}

QueryJoining JoinedQueries::get_query_joining(QueryJoiningKey query_joining_key) const
{
    ReadOnlyTransaction t(m_db);
    auto query_joining_data = m_query_joining_dataset->load(query_joining_key);
    return unpack_query_joining(query_joining_data);
}

std::vector<QueryJoining> JoinedQueries::get_query_joinings() const
{
    std::vector<QueryJoining> query_joinings;
    for (auto& query_joining_data : m_query_joining_dataset->query())
    {
        query_joinings.push_back(unpack_query_joining(query_joining_data));
    }
    return query_joinings;
}

void JoinedQueries::remove_query_joining(QueryJoiningKey query_joining_key, Transaction& t)
{
    QueryJoiningData query_joining_data = m_query_joining_dataset->load(query_joining_key);
    auto selection_direction = SelectionDirection(unpack(query_joining_data.m_input_uuid, &m_report_types), unpack(query_joining_data.m_output_uuid, &m_report_types));
    m_basis.m_transformations.m_selections.remove_selection(selection_direction, t);

    m_subject_joined_query_dataset->erase_query_joining_equal(query_joining_key, t);
    m_subject_query_dataset->erase_query_joining_equal(query_joining_key, t);
    m_query_joining_dataset->erase(query_joining_key, t);
}

std::vector<QueryJoining> JoinedQueries::find_by_input(ReportTypeRef input_ref) const
{
    std::vector<QueryJoining> query_joinings;
    for (auto& query_joining_data : m_query_joining_dataset->query_input_equal(input_ref.get_uuid()))
    {
        query_joinings.push_back(unpack_query_joining(query_joining_data));
    }
    return query_joinings;
}

boost::optional<QueryJoining> JoinedQueries::find_by_output(ReportTypeRef output_ref) const
{
    if (auto query_joinings_data = m_query_joining_dataset->find_by_output(output_ref.get_uuid()))
    {
        return unpack_query_joining(*query_joinings_data);      
    }

    return boost::none; 
}

bool JoinedQueries::has_subject_query(const QueryJoining& query_joining, SubjectRef subject_ref) const
{
    return m_subject_query_dataset->exist({pack(subject_ref), query_joining.get_key()});    
}

std::shared_ptr<const XmlQueryDoc> JoinedQueries::get_subject_query(const QueryJoining& query_joining, SubjectRef subject_ref) const
{
    ReadOnlyTransaction t(m_db);
    if (auto suject_query = find_subject_query(query_joining, subject_ref))
    {
        return suject_query->m_query_doc;
    }

    return nullptr; 
}

void JoinedQueries::set_subject_query(const QueryJoining& query_joining, SubjectRef subject_ref, std::unique_ptr<XmlQueryDoc> query_doc, Transaction& t)
{
    if (!query_doc)
    {
        reset_subject_query(query_joining, subject_ref, t);
        return;
    }

    set_subject_query({
        subject_ref,
        query_joining.get_key(),
        stl_tools::gen_uuid(),
        std::move(query_doc) }, t);
}

std::shared_ptr<const XmlQueryDoc> JoinedQueries::get_base_query(const QueryJoining& query_joining, SubjectRef subject_ref) const
{
    return find_base_query(query_joining, subject_ref).m_query_doc;   
}

std::shared_ptr<const XmlQueryDoc> JoinedQueries::get_joined_query(const QueryJoining& query_joining, SubjectRef subject_ref) const
{
    ReadOnlyTransaction t(m_db);
    if (auto joined_query = find_joined_query(query_joining, subject_ref))
    {
        return joined_query->m_query_doc;
    }   

    return get_base_query(query_joining, subject_ref);
}

void JoinedQueries::set_subject_query(const SubjectQuery& subject_query, Transaction& t)
{
    m_subject_query_dataset->put({
        {pack(subject_query.m_subject), subject_query.m_query_joining_key},
        pack(*subject_query.m_query_doc),
        subject_query.m_uuid }, t);

    auto query_joining = get_query_joining(subject_query.m_query_joining_key);
    invalidate_joined_query(query_joining, subject_query.m_subject, t);
}

void JoinedQueries::reset_subject_query(const QueryJoining& query_joining, SubjectRef subject_ref, Transaction& t)
{
    m_subject_query_dataset->clear({ pack(subject_ref), query_joining.get_key() }, t);
    invalidate_joined_query(query_joining, subject_ref, t);
}

boost::optional<SubjectQuery> JoinedQueries::find_subject_query(const QueryJoining& query_joining, SubjectRef subject_ref) const
{
    ReadOnlyTransaction t(m_db);
    if (auto suject_query_data = m_subject_query_dataset->get({ pack(subject_ref), query_joining.get_key() }))
    {
        return SubjectQuery{
            unpack(suject_query_data->m_identity.m_subject, &m_sources, &m_roles),
            suject_query_data->m_identity.m_query_joining_key,
            suject_query_data->m_uuid,
            unpack(suject_query_data->m_query_xml, *query_joining.get_query_def())           
        };
    }
    return boost::none;
}

boost::optional<JoinedQuery> JoinedQueries::find_joined_query(const QueryJoining& query_joining, SubjectRef subject_ref) const
{
    ReadOnlyTransaction t(m_db);
    if (auto suject_joined_query_data = m_subject_joined_query_dataset->get({ pack(subject_ref), query_joining.get_key() }))
    {
        return JoinedQuery{
            unpack(suject_joined_query_data->m_identity.m_subject, &m_sources, &m_roles),
            suject_joined_query_data->m_identity.m_query_joining_key,
            suject_joined_query_data->m_uuid,
            unpack(suject_joined_query_data->m_query_xml, *query_joining.get_query_def()),
            suject_joined_query_data->m_base_query_uuid,
            suject_joined_query_data->m_subject_query_uuid
        };
    }
  
    return boost::none;
}

JoinedQuery JoinedQueries::find_base_query(const QueryJoining& query_joining, SubjectRef subject_ref) const
{
    ReadOnlyTransaction t(m_db);
    if (auto base_ref = get_base_ref(subject_ref))
    {
        if (auto joined_query = find_joined_query(query_joining, *base_ref))
        {
            return *joined_query;
        }
        return find_base_query(query_joining, *base_ref);
    }

    auto default_query = query_joining.get_default_query();
    if (!default_query)
    {
        default_query = XML::create_query(*query_joining.get_query_def());
    }

    return JoinedQuery{
        subject_ref,
        query_joining.get_key(),
        stl_tools::null_uuid(),
        default_query,
        stl_tools::null_uuid(),
        stl_tools::null_uuid()
    };
}

void JoinedQueries::set_joined_query(const JoinedQuery& joined_query, Transaction& t)
{
    m_subject_joined_query_dataset->put({        
        {pack(joined_query.m_subject), joined_query.m_query_joining_key},
        pack(*joined_query.m_query_doc),
        joined_query.m_uuid,
        joined_query.m_base_query_uuid,
        joined_query.m_subject_query_uuid }, t);

    auto query_joining = get_query_joining(joined_query.m_query_joining_key);
    m_resource_policies.set_setup_uuid(joined_query.m_subject, query_joining.get_output_ref(), joined_query.m_uuid, t);

    for (auto derived_ref : get_derived_refs(m_registry, joined_query.m_subject))
    {
        invalidate_joined_query(query_joining, derived_ref, t);
    }
}

void JoinedQueries::reset_joined_query(const QueryJoining& query_joining, SubjectRef subject_ref, Transaction& t)
{
    m_subject_joined_query_dataset->clear({ pack(subject_ref), query_joining.get_key() }, t);
    m_resource_policies.set_setup_uuid(subject_ref, query_joining.get_output_ref(), stl_tools::null_uuid(), t);

    for (auto derived_ref : get_derived_refs(m_registry, subject_ref))
    {
        invalidate_joined_query(query_joining, derived_ref, t);
    }
}

void JoinedQueries::invalidate_joined_query(const QueryJoining& query_joining, SubjectRef subject_ref, Transaction& t)
{
    t.connect_commit([subject_ref, query_joining, this] {
        m_query_joining_manager->start_query_joining(subject_ref, query_joining);
    });
}

QueryJoining JoinedQueries::unpack_query_joining(const QueryJoiningData& query_joining_data) const
{
    auto input_ref = unpack(query_joining_data.m_input_uuid, &m_report_types);
    auto default_query_xml = query_joining_data.m_default_query_xml;
    stl_tools::lazy_unique_ptr<XmlQueryDoc> default_query = [input_ref, default_query_xml]{
        auto& input = dynamic_cast<const RegularReportType&>(*input_ref);
        return unpack(default_query_xml, *input.get_def_doc());
    };

    return QueryJoining(
        QueryJoiningKey(query_joining_data.m_key),
        unpack(query_joining_data.m_input_uuid, &m_report_types),
        unpack(query_joining_data.m_output_uuid, &m_report_types),
        std::move(default_query)
    );  
}

QueryJoiningData JoinedQueries::pack_query_joining(const QueryJoining& query_joining) const
{
    return QueryJoiningData(
        query_joining.m_key,
        pack(query_joining.m_input_ref),
        pack(query_joining.m_output_ref),
        pack(query_joining.m_default_query.share())
    );
}

void JoinedQueries::on_change_subject_role(SubjectRef subject_ref, RoleRef prev_role_ref, RoleRef next_role_ref, Transaction& t)
{
    for (auto& query_joining : get_query_joinings())
    {
        invalidate_joined_query(query_joining, subject_ref, t);
    }
}

void JoinedQueries::on_remove_sources(const std::vector<SourceKey>& removed_sources, Transaction& t)
{
    m_subject_joined_query_dataset->erase_source_equal(removed_sources, t);
    m_subject_query_dataset->erase_source_equal(removed_sources, t);
}

void JoinedQueries::on_remove_role(RoleKey role_key, Transaction& t)
{
    m_subject_joined_query_dataset->erase_subject_equal(role_key, t);
    m_subject_query_dataset->erase_subject_equal(role_key, t);
}

}} //namespace TR { namespace Core {