#include "stdafx.h"
#include "Services.h"
namespace TR { namespace Core {

Services::Services(Database& db, Basis& basis, Registry& registry):
    m_joined_queries(db, basis, registry),
    m_custom_queries(db, basis, registry),
    m_custom_filters(db, basis, registry),
    m_tracking_streams(db, basis),
    m_stream_filters(db, basis, registry),
    m_grouping_reports(db, basis, registry),
    m_compliances(db, basis, registry),
    m_validations(db, basis, registry),
    m_integrity_checking(db, basis, registry),
    m_archive(db, basis, registry),
    m_custom_actions(db, basis),
    m_settings_capture(db, basis, registry)
{
    m_compliances.set_joined_queries(&m_joined_queries);
    m_validations.set_joined_queries(&m_joined_queries);
}

void Services::restore(Transaction& t)
{
    m_joined_queries.restore(t);
    m_grouping_reports.restore(t);
    m_custom_queries.restore(t);
    m_compliances.restore(t);
    m_validations.restore(t);
    m_custom_actions.restore(t);
}

void Services::run(Executive* executive, Transaction& t)
{   
    m_custom_filters.run(executive, t);
    m_stream_filters.run(executive, t);
    m_joined_queries.run(executive, t);
    m_settings_capture.run(executive, t);
    m_compliances.run(executive, t);
    m_validations.run(executive, t);    
    m_integrity_checking.run(executive, t);
    m_archive.run(executive, t);
}

}} //namespace TR { namespace Core {