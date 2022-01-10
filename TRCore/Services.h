#pragma once
#include "BackgndFwd.h"
#include "BasisFwd.h"
#include "RegistryFwd.h"
#include "CoreFwd.h"
#include "JoinedQueries.h"
#include "CustomQueries.h"
#include "Archive.h"
#include "IntegrityChecking.h"
#include "GroupingReports.h"
#include "Compliance.h"
#include "Validation.h"
#include "CustomActions.h"
#include "CustomFilters.h"
#include "TrackingStreams.h"
#include "StreamFilters.h"

namespace TR { namespace Core {

struct Services
{
    Services(Database& db, Basis& basis, Registry& registry);

    void restore(Transaction& t);
    void run(Executive* executive, Transaction& t);

    JoinedQueries m_joined_queries;
    CustomQueries m_custom_queries;
    CustomFilters m_custom_filters;
    TrackingStreams m_tracking_streams;
    StreamFilters m_stream_filters;
    GroupingReports m_grouping_reports;
    Compliances m_compliances;
    Validations m_validations;   
    IntegrityChecking m_integrity_checking;
    Archive m_archive;
    CustomActions m_custom_actions; 
};

}} //namespace TR { namespace Core {