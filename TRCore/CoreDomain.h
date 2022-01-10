#pragma once
#include "BackgndFwd.h"
#include "Basis.h"
#include "Registry.h"
#include "Services.h"
#include "Executive.h"

namespace TR { namespace Core {

struct CoreDomain
{   
    CoreDomain(Database& db);
    ~CoreDomain();

    CoreDomain(const CoreDomain&) = delete;
    CoreDomain& operator = (const CoreDomain&) = delete;
    CoreDomain(CoreDomain&&) = delete;  
    CoreDomain& operator = (CoreDomain&&) = delete;

    void run(Executive* executive, Transaction& t);

    Basis m_basis;
    Registry m_registry;
    Services m_services;

    std::unique_ptr<Executive> m_basis_executive;
    std::unique_ptr<Executive> m_registry_executive;
    std::unique_ptr<Executive> m_services_executive;
};

}} //namespace TR { namespace Core {