#include "stdafx.h"
#include "SummaryCollector.h"
#include "CoreDomain.h"
#include "Security.h"
#include "Scheduler.h"
#include "JobManager.h"
#include "Transaction.h"
#include "ExportTasks.h"
#include "ServicesPacking.h"
#include "SummaryData-odb.hxx"
#include <limits>
namespace TR { namespace Core {
namespace {
static const time_t one_min = 60;
static const time_t one_hour = 60 * one_min;
static const time_t one_day = 24 * one_hour;
static const time_t one_week = 7 * one_day;
static const time_t COLLECT_SUMMARY_TIME = one_hour;
} //namespace {

Summary::Summary(Key user_key, stl_tools::lazy_shared_ptr<Blob> summary_pdf, time_t time) :
    m_user_key(user_key),
    m_summary_pdf(summary_pdf),
    m_time(time)
{
}

Key Summary::get_user_key() const
{
    return m_user_key;
}

Blob& Summary::get_summary_pdf() const
{
    return *m_summary_pdf;
}

time_t Summary::get_time() const
{
    return m_time;
}

class CollectSummaryTask : public Task
{
public:
    CollectSummaryTask(SummaryCollector& summary_collector, Key user_key) :
        Task(*summary_collector.m_executive),
        m_summary_collector(summary_collector),
        m_core_domain(summary_collector.m_core_domain),
        m_security(*summary_collector.m_security),
        m_scheduler(*summary_collector.m_scheduler),
        m_user_key(user_key)
    {
    }

protected:
    // Job override
    virtual void do_run() override
    {
        if (!check_conditions())
        {
            emit_completed();
            return;
        }

        auto export_task = std::make_shared<ExportSummary>(
            m_core_domain,
            m_scheduler,
            m_user_key,
            m_executive);

        run_subtasks({ export_task }, [this, export_task] {
            collect_summary(std::move(export_task->get_output()));
            emit_completed();
        });
    }

    bool check_conditions()
    {
        if (!m_security.find_user(m_user_key))
        {            
            return false;
        }

        if (auto last_summary = m_summary_collector.find_last_summary(m_user_key))
        {
            if (time(nullptr) - last_summary->get_time() < COLLECT_SUMMARY_TIME)
            {                
                return false;
            }
        }

        return true;
    }

    void collect_summary(Blob summary_content)
    {
        if (!m_security.find_user(m_user_key))
        {            
            return;
        }

        Summary summary(
            m_user_key,
            std::make_shared<Blob>(std::move(summary_content)),
            time(nullptr));

        m_summary_collector.collect_summary(summary);
    }

private:
    SummaryCollector& m_summary_collector;
    CoreDomain& m_core_domain;
    Security& m_security;
    Scheduler& m_scheduler;
    Key m_user_key;
};

SummaryCollector::SummaryCollector(Database& db, CoreDomain& core_domain) :
    m_db(db),
    m_core_domain(core_domain),
    m_security(nullptr),
    m_scheduler(nullptr),
    m_executive(nullptr)
{
}

SummaryCollector::~SummaryCollector()
{
}

void SummaryCollector::set_security(Security* security)
{
    _ASSERT(!m_security);
    m_security = security;
    m_security->connect_remove_user([this](Key user_key, Transaction& t) {
        on_remove_user(user_key, t);
        }, SUMMARY_COLLECTOR_PRIORITY);
}

void SummaryCollector::set_scheduler(Scheduler* scheduler)
{
    _ASSERT(!m_scheduler);
    m_scheduler = scheduler;
}

void SummaryCollector::run(Executive* executive, Transaction& t)
{
    _ASSERT(!m_executive);
    transact_assign(m_executive, executive, t);

    t.connect_commit([this]()
    {
        //collect_summary_reports();
        m_executive->async(std::bind(&SummaryCollector::collect_summary_reports, this), time(nullptr) + one_min);
    });
}

boost::optional<Summary> SummaryCollector::find_last_summary(Key user_key) const
{
    if (auto summary_data = find_last_summary_data(user_key))
    {
        return unpack_summary(*summary_data);        
    }

    return boost::none;   
}

std::vector<Summary> SummaryCollector::get_summary(Key user_key)
{
    ReadOnlyTransaction t(m_db);
    auto user_summary_data = m_db.query<SummaryData>(
        odb::query<SummaryData>::user_key == user_key);

    std::vector<Summary> result;
    for(auto& summary_data: m_db.query<SummaryData>(
        odb::query<SummaryData>::user_key == user_key))    
    {
        result.push_back(unpack_summary(summary_data));
    }

    return result;
}

Summary SummaryCollector::unpack_summary(const SummaryData& summary_data) const
{
    return {
        summary_data.m_user_key,
         [this, summary_data_key = summary_data.m_key] {
            ReadOnlyTransaction t(m_db);
            SummaryContentData summary_content_data;
            m_db.load<SummaryContentData>(summary_data_key, summary_content_data);
            return std::make_shared<Blob>(std::move(summary_content_data.m_content));
        },
        summary_data.m_time
    };
}

boost::optional<SummaryData> SummaryCollector::find_last_summary_data(Key user_key) const
{
    ReadOnlyTransaction t(m_db);
    auto summary_data = m_db.query<SummaryData>(
        odb::query<SummaryData>::user_key == user_key &&
        odb::query<SummaryData>::last == true);

    if (summary_data.empty())
    {
        return boost::none;
    }

    return *summary_data.begin();
}

void SummaryCollector::collect_summary(const Summary& summary)
{
    Transaction t(m_db);
    if (auto last_summary_data = find_last_summary_data(summary.get_user_key()))
    {
        last_summary_data->m_last = false;
        m_db.update(*last_summary_data);
    }

    auto summary_data = SummaryData( 0, summary.get_user_key(), summary.get_time(), true);
    auto summary_data_key = m_db.persist(summary_data);
    m_db.persist(SummaryContentData(summary_data_key, summary.get_summary_pdf()));
    t.commit();
}

void SummaryCollector::on_remove_user(Key user_key, Transaction& t)
{
    std::vector<Key> remove_keys;
    for (auto& summary_data : m_db.query<SummaryData>(odb::query<SummaryData>::user_key == user_key))
    {
        remove_keys.push_back(summary_data.m_key);
    }

    if (!remove_keys.empty())
    {
        m_db.erase_query<SummaryData>(odb::query<SummaryData>::key.in_range(remove_keys.begin(), remove_keys.end()));
        m_db.erase_query<SummaryContentData>(odb::query<SummaryContentData>::key.in_range(remove_keys.begin(), remove_keys.end()));
    }
}

void SummaryCollector::collect_summary_reports()
{
    no_except([&] {
        if (m_task_manager.is_empty(0))
        {
            for (auto& user : m_security->get_users())
            {
                m_task_manager.run_job(0, std::make_shared<CollectSummaryTask>(*this, user.m_key));
            }
        }

        m_executive->async(std::bind(&SummaryCollector::collect_summary_reports, this), time(nullptr) + one_min);
    });
}

}} //namespace TR { namespace Core {