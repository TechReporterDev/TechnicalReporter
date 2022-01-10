#pragma once
#include "stdafx.h"
#include "Registry.h"
#include "Basis.h"
#include "Executive.h"
#include "Transaction.h"
namespace TR { namespace Core {

Registry::Registry(Database& db, Basis& basis):
    m_resource_policies(db, basis),
    m_conveyor(db, basis),
    m_source_resources(db, basis)
{
    m_roles.set_database(&db);
    m_sources.set_database(&db);
    m_sources.set_source_types(&basis.m_source_types);
    m_sources.set_roles(&m_roles);
    m_links.set_database(&db);
    m_links.set_sources(&m_sources);
    m_collector.set_database(&db);
    m_collector.set_report_types(&basis.m_report_types);
    m_collector.set_source_types(&basis.m_source_types);
    m_collector.set_sources(&m_sources);
    m_reservoir.set_database(&db);
    m_reservoir.set_stream_types(&basis.m_stream_types);
    m_reservoir.set_sources(&m_sources);

    m_resource_policies.set_sources(&m_sources);
    m_resource_policies.set_roles(&m_roles);

    m_source_resources.set_sources(&m_sources);
    m_source_resources.set_links(&m_links);
    m_source_resources.set_roles(&m_roles);
    m_source_resources.set_resource_policies(&m_resource_policies);
    
    m_conveyor.set_sources(&m_sources);
    m_conveyor.set_links(&m_links);
    m_conveyor.set_collector(&m_collector);
    m_conveyor.set_reservoir(&m_reservoir);
    m_conveyor.set_source_resources(&m_source_resources);
};

void Registry::restore(Transaction& t)
{
}

void Registry::run(Executive* executive, Transaction& t)
{
    transact_assign(m_collector_executive, branch(*executive), t);
    m_collector.run(m_collector_executive.get(), t);
    
    m_reservoir.run(executive, t);
    m_source_resources.run(executive, t);

    transact_assign(m_conveyor_executive, branch(*executive), t);
    m_conveyor_executive->m_processing = std::make_unique<ExpandingExecutor2>(std::vector<Executor*>{ 
        m_collector_executive->m_processing.get(), 
        executive->m_processing.get() });
    m_conveyor.run(m_conveyor_executive.get(), t);
}

}} //namespace TR { namespace Core {