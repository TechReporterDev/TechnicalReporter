#include "stdafx.h"
#include "GroupingsSuit.h"
#include "CoreTestAssist.h"

namespace {

void add_grouping_test()
{
    CLEAN_AND_INSTALL();

    auto input_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto output_uuid = ReportTypeUUID(stl_tools::gen_uuid());

    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(input_uuid, L""));
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(output_uuid, L""));
    auto direction = GroupingDirection(input_type, output_type);

    TEST_ASSERT(!groupings.find_grouping(direction));
    TEST_EXCEPTION(groupings.get_grouping(direction));
    TEST_ASSERT(groupings.find_by_input(input_type).empty());
    TEST_ASSERT(groupings.find_by_output(output_type).empty());

    int add_counter = 0;
    groupings.connect_add_grouping([&](const GroupingDirection& added_direction, Transaction& t) {
        TEST_ASSERT(direction == added_direction);
        ++add_counter;
    }, 0);

    Transaction t1(db);
    groupings.add_grouping(std::make_unique<GroupingMockup>(direction), t1);
    t1.commit();

    TEST_ASSERT(add_counter == 1);
    TEST_ASSERT(groupings.find_grouping(direction));
    TEST_NO_EXCEPTION(groupings.get_grouping(direction));
    TEST_ASSERT(groupings.find_by_input(input_type).size() == 1);
    TEST_ASSERT(groupings.find_by_output(output_type).size() == 1); 
}

void duplicate_grouping_test()
{
    CLEAN_AND_INSTALL();

    auto input_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto output_uuid = ReportTypeUUID(stl_tools::gen_uuid());

    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(input_uuid, L""));
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(output_uuid, L""));
    auto direction = GroupingDirection(input_type, output_type);

    TEST_ASSERT(!groupings.find_grouping(direction));

    Transaction t1(db);
    groupings.add_grouping(std::make_unique<GroupingMockup>(direction), t1);
    t1.commit();

    TEST_ASSERT(groupings.find_grouping(direction));

    Transaction t2(db);
    TEST_EXCEPTION(groupings.add_grouping(std::make_unique<GroupingMockup>(direction), t2));
    t2.rollback();
}

void remove_grouping_test()
{
    CLEAN_AND_INSTALL();

    auto input_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto output_uuid = ReportTypeUUID(stl_tools::gen_uuid());

    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(input_uuid, L""));
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(output_uuid, L""));
    auto direction = GroupingDirection(input_type, output_type);

    Transaction t1(db);
    groupings.add_grouping(std::make_unique<GroupingMockup>(direction), t1);
    t1.commit();

    TEST_ASSERT(groupings.find_grouping(direction));
    TEST_ASSERT(groupings.find_by_input(input_type).size() == 1);
    TEST_ASSERT(groupings.find_by_output(output_type).size() == 1);

    int remove_counter = 0;
    groupings.connect_remove_grouping([&](GroupingDirection removed_direction, Transaction& t){
        TEST_ASSERT(removed_direction == direction);
        ++remove_counter;
    }, 0);

    Transaction t2(db);
    groupings.remove_grouping(direction, t2);
    t2.commit();

    TEST_ASSERT(!groupings.find_grouping(direction));
    TEST_ASSERT(groupings.find_by_input(input_type).empty());
    TEST_ASSERT(groupings.find_by_output(output_type).empty()); 

    Transaction t3(db);
    TEST_EXCEPTION(groupings.remove_grouping(direction, t3));
    t3.rollback();
}

void prevent_remove_input_test()
{
    CLEAN_AND_INSTALL();

    auto input_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto output_uuid = ReportTypeUUID(stl_tools::gen_uuid());

    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(input_uuid, L""));
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(output_uuid, L""));
    auto direction = GroupingDirection(input_type, output_type);

    Transaction t1(db);
    groupings.add_grouping(std::make_unique<GroupingMockup>(direction), t1);
    t1.commit();

    TEST_ASSERT(groupings.find_grouping(direction));
    TEST_ASSERT(report_types.has_report_type(input_uuid));

    Transaction t2(db);
    TEST_EXCEPTION(report_types.remove_report_type(input_uuid, t2));
    t2.rollback();

    Transaction t3(db);
    groupings.remove_grouping(direction, t3);
    TEST_NO_EXCEPTION(report_types.remove_report_type(input_uuid, t3)); 
    t3.commit();
}

void prevent_remove_output_test()
{
    CLEAN_AND_INSTALL();

    auto input_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto output_uuid = ReportTypeUUID(stl_tools::gen_uuid());

    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(input_uuid, L""));
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(output_uuid, L""));
    auto direction = GroupingDirection(input_type, output_type);

    Transaction t1(db);
    groupings.add_grouping(std::make_unique<GroupingMockup>(direction), t1);
    t1.commit();

    TEST_ASSERT(groupings.find_grouping(direction));
    TEST_ASSERT(report_types.has_report_type(output_uuid));

    Transaction t2(db);
    TEST_EXCEPTION(report_types.remove_report_type(output_uuid, t2));
    t2.rollback();

    Transaction t3(db);
    groupings.remove_grouping(direction, t3);
    TEST_NO_EXCEPTION(report_types.remove_report_type(output_uuid, t3));    
    t3.commit();
}

} //namespace {

test_assist::test_suit get_groupings_suit()
{
    test_assist::test_suit ts(L"groupings_suit");
    ts
        .add({L"add_grouping_test", &add_grouping_test})
        .add({L"duplicate_grouping_test", &duplicate_grouping_test})
        .add({L"remove_grouping_test", &remove_grouping_test})
        .add({L"prevent_remove_input_test", &prevent_remove_input_test})
        .add({L"prevent_remove_output_test", &prevent_remove_output_test});
    return ts;
}