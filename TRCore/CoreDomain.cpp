#include "stdafx.h"
#include "CoreDomain.h"
#include "Transaction.h"
namespace TR { namespace Core {

CoreDomain::CoreDomain(Database& db):
    m_basis(&db),
    m_registry(db, m_basis),
    m_services(db, m_basis, m_registry)
{
    Transaction t(db);
    m_basis.restore(t);
    m_registry.restore(t);
    m_services.restore(t);
    t.commit();
}

CoreDomain::~CoreDomain()
{
}

void CoreDomain::run(Executive* executive, Transaction& t)
{    
    transact_assign(m_basis_executive, branch(*executive), t);
    m_basis.run(m_basis_executive.get(), t);

    transact_assign(m_registry_executive, branch(*executive), t);
    m_registry.run(m_registry_executive.get(), t);

    transact_assign(m_services_executive, branch(*executive), t);
    m_services.run(m_services_executive.get(), t);
}

}} //namespace TR { namespace Core {