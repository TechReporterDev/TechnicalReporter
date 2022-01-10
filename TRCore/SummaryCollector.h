#pragma once
#include "AppDecl.h"
#include "JobManager.h"

namespace TR { namespace Core {
struct SummaryData;

class Summary
{
public:
    Summary(Key user_key, stl_tools::lazy_shared_ptr<Blob> summary_pdf, time_t time);

    Key                             get_user_key() const;
    Blob&                           get_summary_pdf() const;
    time_t                          get_time() const;

protected:
    Key m_user_key;
    stl_tools::lazy_shared_ptr<Blob> m_summary_pdf;
    time_t m_time;
};

class SummaryCollector
{
public:
    static const Priority SUMMARY_COLLECTOR_PRIORITY = 1950;

    SummaryCollector(Database& db, CoreDomain& core_domain);
    ~SummaryCollector();

    void                            set_security(Security* security);
    void                            set_scheduler(Scheduler* scheduler);
    void                            run(Executive* executive, Transaction& t);

    boost::optional<Summary>        find_last_summary(Key user_key) const;
    std::vector<Summary>            get_summary(Key user_key);

private:
    Summary                         unpack_summary(const SummaryData& summary_data) const;
    boost::optional<SummaryData>    find_last_summary_data(Key user_key) const;

    void                            collect_summary(const Summary& summary);
    void                            on_remove_user(Key user_key, Transaction& t);
    void                            collect_summary_reports();

    friend class CollectSummaryTask;

    CoreDomain& m_core_domain;
    Database& m_db;
    Security* m_security;
    Scheduler* m_scheduler;
    Executive* m_executive;
    JobManager<int> m_task_manager;
};

}} //namespace TR { namespace Core {
