#include "stdafx.h"
#include "SelectionsSuit.h"
#include "CoreTestAssist.h"

namespace {

void add_selection_test()
{
    CLEAN_AND_INSTALL();

    auto input_uuid_1 = ReportTypeUUID(stl_tools::gen_uuid());
    auto input_uuid_2 = ReportTypeUUID(stl_tools::gen_uuid());
    auto output_uuid = ReportTypeUUID(stl_tools::gen_uuid());

    auto& input_type_1 = report_types.add_report_type(std::make_unique<PlainTextReportType>(input_uuid_1, L""));
    auto& input_type_2 = report_types.add_report_type(std::make_unique<PlainTextReportType>(input_uuid_2, L""));
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(output_uuid, L""));
    auto direction = SelectionDirection({input_type_1, input_type_2}, output_type);

    TEST_ASSERT(!selections.find_selection(direction));
    TEST_EXCEPTION(selections.get_selection(direction));
    TEST_ASSERT(selections.find_by_input(input_type_1).empty());
    TEST_ASSERT(selections.find_by_input(input_type_2).empty());
    TEST_ASSERT(selections.find_by_output(output_type).empty());

    int add_counter = 0;
    selections.connect_add_selection([&](const SelectionDirection& _direction, Transaction& t) {
        TEST_ASSERT(direction == _direction);
        ++add_counter;
    }, 0);

    Transaction t1(db);
    std::unique_ptr<SelectionMockup> selection_mockup(new SelectionMockup(direction));
    selections.add_selection(std::move(selection_mockup), t1);
    t1.commit();

    TEST_ASSERT(add_counter == 1);
    TEST_ASSERT(selections.find_selection(direction));
    TEST_NO_EXCEPTION(selections.get_selection(direction));
    TEST_ASSERT(selections.find_by_input(input_type_1).size() == 1);
    TEST_ASSERT(selections.find_by_input(input_type_2).size() == 1);
    TEST_ASSERT(selections.find_by_output(output_type).size() == 1);    
}

void duplicate_selection_test()
{
    CLEAN_AND_INSTALL();

    auto input_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto output_uuid = ReportTypeUUID(stl_tools::gen_uuid());

    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(input_uuid, L""));
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(output_uuid, L""));
    auto direction = SelectionDirection(input_type, output_type);

    TEST_ASSERT(!selections.find_selection(direction));

    Transaction t1(db);
    selections.add_selection(std::make_unique<SelectionMockup>(direction), t1);
    t1.commit();

    TEST_ASSERT(selections.find_selection(direction));

    Transaction t2(db);
    TEST_EXCEPTION(selections.add_selection(std::make_unique<SelectionMockup>(direction), t2));
    t2.rollback();
}

void remove_selection_test()
{
    CLEAN_AND_INSTALL();

    auto input_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto output_uuid = ReportTypeUUID(stl_tools::gen_uuid());

    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(input_uuid, L""));
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(output_uuid, L""));
    auto direction = SelectionDirection(input_type, output_type);

    TEST_ASSERT(!selections.find_selection(direction));

    Transaction t1(db);
    selections.add_selection(std::make_unique<SelectionMockup>(direction), t1);
    t1.commit();

    TEST_ASSERT(selections.find_selection(direction));
    TEST_ASSERT(selections.find_by_input(input_type).size() == 1);
    TEST_ASSERT(selections.find_by_output(output_type).size() == 1);

    int remove_counter = 0;
    selections.connect_remove_selection([&](SelectionDirection removed_direction, Transaction& t){
        TEST_ASSERT(removed_direction == direction);
        ++remove_counter;
    }, 0);

    Transaction t2(db);
    selections.remove_selection(direction, t2);
    t2.commit();

    TEST_ASSERT(!selections.find_selection(direction));
    TEST_ASSERT(selections.find_by_input(input_type).empty());
    TEST_ASSERT(selections.find_by_output(output_type).empty());    

    Transaction t3(db);
    TEST_EXCEPTION(selections.remove_selection(direction, t3));
    t3.rollback();
}

void prevent_remove_input_test()
{
    CLEAN_AND_INSTALL();

    auto input_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto output_uuid = ReportTypeUUID(stl_tools::gen_uuid());

    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(input_uuid, L""));
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(output_uuid, L""));
    auto direction = SelectionDirection(input_type, output_type);

    Transaction t1(db);
    selections.add_selection(std::make_unique<SelectionMockup>(direction), t1);
    t1.commit();

    TEST_ASSERT(selections.find_selection(direction));

    Transaction t2(db);
    TEST_EXCEPTION(report_types.remove_report_type(input_uuid, t2));
    t2.rollback();

    Transaction t3(db);
    selections.remove_selection(direction, t3);
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
    auto direction = SelectionDirection(input_type, output_type);

    Transaction t1(db);
    selections.add_selection(std::make_unique<SelectionMockup>(direction), t1);
    t1.commit();

    TEST_ASSERT(selections.find_selection(direction));

    Transaction t2(db);
    TEST_EXCEPTION(report_types.remove_report_type(output_uuid, t2));
    t2.rollback();

    Transaction t3(db);
    selections.remove_selection(direction, t3);
    TEST_NO_EXCEPTION(report_types.remove_report_type(output_uuid, t3));    
    t3.commit();
}

} //namespace {

test_assist::test_suit get_selections_suit()
{
    test_assist::test_suit ts(L"selections_suit");
    ts
        .add({L"add_selection_test", &add_selection_test})
        .add({L"duplicate_selection_test", &duplicate_selection_test})
        .add({L"remove_selection_test", &remove_selection_test})
        .add({L"prevent_remove_input_test", &prevent_remove_input_test})
        .add({L"prevent_remove_output_test", &prevent_remove_output_test});
    return ts;
}