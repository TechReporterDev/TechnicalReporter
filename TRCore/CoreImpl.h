#pragma once
#include "BackgndFwd.h"
#include "CoreDomain.h"
#include "Security.h"
#include "Scheduler.h"
#include "Notifier.h"
#include "Mailing.h"
#include "SummaryCollector.h"
#include "Syslog.h"
#include "Executive.h"
#include "Transaction.h"

namespace TR { namespace Core {

struct CoreImpl
{   
    CoreImpl(std::unique_ptr<Database> db);
    ~CoreImpl();

    CoreImpl(const CoreImpl&) = delete;
    CoreImpl& operator = (const CoreImpl&) = delete;
    CoreImpl(CoreImpl&&) = delete;  
    CoreImpl& operator = (CoreImpl&&) = delete;

    void run(std::unique_ptr<Executive> executive, Transaction& t);
    void stop();

    std::unique_ptr<Database> m_db; 
    std::unique_ptr<Executive> m_executive;
    std::unique_ptr<Executive> m_domain_executive;
    std::unique_ptr<Executive> m_app_executive;

    CoreDomain m_core_domain;
    Security m_security;
    Scheduler m_scheduler;
    Notifier m_notifier;
    Mailing m_mailing;
    SummaryCollector m_summary_collector;
    Syslog m_syslog;
};

}} //namespace TR { namespace Core {