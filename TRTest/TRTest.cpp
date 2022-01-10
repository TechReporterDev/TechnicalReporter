#include "stdafx.h"
#include "STLTools\stl_tools_suit.h"
#include "XMLTools\libxml2_wrap_suit.h"
#include "XMLTools\libxml2_iterator_suit.h"
#include "TRXML\XMLDefinitionSuit.h"
#include "TRXML\XMLQuerySuit.h"
#include "TRCore\ReportTypesSuit.h"
#include "TRCore\StreamTypesSuit.h"
#include "TRCore\SourceTypesSuit.h"
#include "TRCore\ActionsSuit.h"
#include "TRCore\ShortcutsSuit.h"
#include "TRCore\SelectionsSuit.h"
#include "TRCore\GroupingsSuit.h"
#include "TRCore\SourceTypeActivitiesSuit.h"
#include "TRCore\ActionDelegatesSuit.h"
#include "TRCore\RolesSuit.h"
#include "TRCore\SourcesSuit.h"
#include "TRCore\CollectorSuit.h"
#include "TRCore\ConveyorSuit.h"
#include "TRCore\ResourcePoliciesSuit.h"
#include "TRCore\SourceResourcesSuit.h"
#include "TRCore\RoutineRunnerSuit.h"
#include "TRCore\XmlDiffSuit.h"
#include "TRCore\TextDiffSuit.h"

#define stdin  (__acrt_iob_func(0))
#define stdout (__acrt_iob_func(1))
#define stderr (__acrt_iob_func(2))

FILE _iob[] = { *stdin, *stdout, *stderr };
extern "C" FILE * __cdecl __iob_func(void) { return _iob; }

test_assist::test_suit test_suits[] =
{
    get_stl_tools_suit(),
    get_libxml2_wrap_suit(),
    get_libxml2_iterator_suit(),
    get_xml_definition_suit(),
    get_xml_query_suit(),
    get_routine_runner_suit(),
    get_xml_diff_suit(),
    get_text_diff_suit(),
    get_report_types_suit(),
    get_stream_types_suit(),
    get_source_types_suit(),
    get_actions_suit(),
    get_action_delegates_suit(),
    get_shortcuts_suit(),
    get_selections_suit(),
    get_groupings_suit(),
    get_source_type_activities_suit(),
    get_sources_suit(),
    get_roles_suit(),
    get_resource_policies_suit(),
    get_source_resources_suit(),
    get_collector_suit(),
    get_conveyor_suit(),
};

int _tmain(int argc, _TCHAR* argv[])
{
    test_assist::test_runner test_runner;
    for(const auto& test_suit : test_suits)
    {
        test_runner.run(test_suit);     
    }
    test_runner.show_stats();
    std::wcin.get();    
    return 0;
}

