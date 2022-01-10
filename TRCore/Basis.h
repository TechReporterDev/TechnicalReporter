#pragma once
#include "Database.h"
#include "ReportTypes.h"
#include "StreamTypes.h"
#include "Actions.h"
#include "ActionDelegates.h"
#include "ActionShortcuts.h"
#include "SourceTypes.h"
#include "Transformations.h"
#include "SourceTypeActivities.h"
#include "Content.h"
#include "Diff.h"
namespace TR { namespace Core {

struct Basis
{
    static const Priority BASIS_PRIORITY = 1000;

    Basis(Database* db);

    void restore(Transaction& t);
    void run(Executive* executive, Transaction& t);

    ReportTypes             m_report_types;
    StreamTypes             m_stream_types;
    Actions                 m_actions;
    ActionDelegates         m_action_delegates;
    SourceTypes             m_source_types;
    ActionShortcuts         m_action_shortcuts;
    Transformations         m_transformations;  
    SourceTypeActivities    m_source_type_activities;
};

}} //namespace TR { namespace Core {