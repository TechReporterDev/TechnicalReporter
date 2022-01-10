#pragma once
#include "BackgndFwd.h"
#include "BasisFwd.h"
#include "Roles.h"
#include "Sources.h"
#include "Links.h"
#include "Collector.h"
#include "Reservoir.h"
#include "ResourcePolicies.h"
#include "SourceResources.h"
#include "Conveyor.h"

namespace TR { namespace Core {

struct Registry
{
    static const Priority REGISTRY_PRIORITY = 2000;

    Registry(Database& db, Basis& basis);

    void restore(Transaction& t);
    void run(Executive* executive, Transaction& t);
    
    Roles               m_roles;
    Sources             m_sources;
    Links               m_links;
    Collector           m_collector;
    Reservoir           m_reservoir;
    ResourcePolicies    m_resource_policies;
    SourceResources     m_source_resources;
    Conveyor            m_conveyor;

    std::unique_ptr<Executive> m_collector_executive;
    std::unique_ptr<Executive> m_conveyor_executive;
};

}} //namespace TR { namespace Core {