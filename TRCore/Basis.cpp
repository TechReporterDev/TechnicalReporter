#include "stdafx.h"
#include "Basis.h"
namespace TR { namespace Core {

Basis::Basis(Database* db)  
{
    m_report_types.set_database(db);
    m_stream_types.set_database(db);
    m_source_types.set_database(db);
    m_source_types.set_report_types(&m_report_types);
    m_source_types.set_actions(&m_actions);
    
    m_action_delegates.set_report_types(&m_report_types);
    m_action_delegates.set_actions(&m_actions);
    m_action_shortcuts.set_report_types(&m_report_types);
    m_action_shortcuts.set_actions(&m_actions);
    
    m_transformations.m_selections.set_report_types(&m_report_types);
    m_transformations.m_groupings.set_report_types(&m_report_types);
    m_transformations.m_convertions.set_stream_types(&m_stream_types);
    m_transformations.m_trackings.set_report_types(&m_report_types);
    m_transformations.m_trackings.set_stream_types(&m_stream_types);
    m_transformations.m_generators.set_source_types(&m_source_types);
    m_transformations.m_generators.set_report_types(&m_report_types);

    m_source_type_activities.set_source_types(&m_source_types);
    m_source_type_activities.set_report_types(&m_report_types);
    m_source_type_activities.set_transformations(&m_transformations);
    m_source_type_activities.set_action_delegates(&m_action_delegates);
    m_source_type_activities.set_shortcuts(&m_action_shortcuts);
};

void Basis::restore(Transaction& t)
{
    m_source_types.add_source_type(std::make_unique<GroupSourceType>(), t);
}

void Basis::run(Executive* executive, Transaction& t)
{   
    m_source_type_activities.run(executive, t);
}

}} //namespace TR { namespace Core {