#include "stdafx.h"
#include "CoreImpl.h"
namespace TR { namespace Core {

CoreImpl::CoreImpl(std::unique_ptr<Database> db):
    m_db(std::move(db)),
    m_core_domain(*m_db),
    m_security(*m_db, m_core_domain),
    m_scheduler(*m_db, m_core_domain),
    m_notifier(*m_db, m_core_domain),
    m_mailing(*m_db, m_core_domain),
    m_summary_collector(*m_db, m_core_domain),
    m_syslog(*m_db, m_core_domain),
    m_executive(nullptr)
{
    ::xmlKeepBlanksDefault(0);
    m_notifier.set_security(&m_security);

    m_mailing.set_security(&m_security);
    m_mailing.set_notifier(&m_notifier);
    m_mailing.set_summary_collector(&m_summary_collector);

    m_summary_collector.set_security(&m_security);
    m_summary_collector.set_scheduler(&m_scheduler);
}

CoreImpl::~CoreImpl()
{
}

void CoreImpl::run(std::unique_ptr<Executive> executive, Transaction& t)
{
    _ASSERT(!m_executive);

    transact_assign(m_executive, std::move(executive), t);
    transact_assign(m_domain_executive, branch(*m_executive), t);
    transact_assign(m_app_executive, branch(*m_executive), t);

    m_core_domain.run(m_domain_executive.get(), t);
    m_scheduler.run(m_app_executive.get(), t);
    m_mailing.run(m_app_executive.get(), t);
    m_summary_collector.run(m_app_executive.get(), t);
    m_syslog.run(m_app_executive.get(), t);
}

void CoreImpl::stop()
{
    m_executive->m_io->stop();
    m_executive->m_processing->stop();
    m_executive->m_control->stop();    
}

}} //namespace TR { namespace Core {