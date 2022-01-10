#include "stdafx.h"
#include "ReportTypesSuit.h"
#include "CoreTestAssist.h"
namespace {

void add_report_type_test()
{
    CLEAN_AND_INSTALL();

    auto report_type_uuid = ReportTypeUUID(stl_tools::gen_uuid());      
    auto report_type = std::make_unique<PlainTextReportType>(report_type_uuid, L"report_type");
    TEST_EXCEPTION(report_type->get_ref());

    int add_counter = 0;
    report_types.connect_add_report_type([&](ReportTypeRef ref, Transaction& t){
        TEST_ASSERT(ref.get_uuid() == report_type_uuid);
        ++add_counter;
    }, 0);

    TEST_ASSERT(! report_types.has_report_type(report_type_uuid));
    TEST_EXCEPTION(report_types.get_report_type(report_type_uuid));
    TEST_ASSERT(boost::distance(report_types.get_report_types()) == 0);

    auto& report_type_ref = report_types.add_report_type(std::move(report_type));
    TEST_NO_EXCEPTION(report_type_ref.get_ref());

    TEST_ASSERT(add_counter == 1);
    TEST_ASSERT(report_types.has_report_type(report_type_uuid));
    TEST_NO_EXCEPTION(report_types.get_report_type(report_type_uuid));
    TEST_ASSERT(boost::distance(report_types.get_report_types()) == 1);
}

void break_add_report_type_test()
{
    CLEAN_AND_INSTALL();

    auto report_type_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    report_types.connect_add_report_type([&](ReportTypeRef ref, Transaction& t){
        throw std::logic_error("");
    }, 0);

    TEST_EXCEPTION(report_types.add_report_type(std::make_unique<PlainTextReportType>(report_type_uuid, L"report_type")));
    TEST_ASSERT(! report_types.has_report_type(report_type_uuid));
}

void duplicate_report_type_test()
{
    CLEAN_AND_INSTALL();

    auto report_type_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    report_types.add_report_type(std::make_unique<PlainTextReportType>(report_type_uuid, L"report_type_1"));
    TEST_ASSERT(report_types.has_report_type(report_type_uuid));

    TEST_EXCEPTION(report_types.add_report_type(std::make_unique<PlainTextReportType>(report_type_uuid, L"report_type_1")));
}

void remove_report_type_test()
{
    CLEAN_AND_INSTALL();

    auto report_type_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto report_type = std::make_unique<PlainTextReportType>(report_type_uuid, L"report_type");
    auto& report_type_ref = report_types.add_report_type(std::move(report_type));

    TEST_ASSERT(report_types.has_report_type(report_type_uuid));

    int remove_counter = 0;
    report_types.connect_remove_report_type([&](UUID removed_uuid, Transaction& t){
        TEST_ASSERT(report_type_uuid == removed_uuid);
        ++remove_counter;
    }, 0);

    report_types.remove_report_type(report_type_uuid);

    TEST_ASSERT(remove_counter == 1);
    TEST_ASSERT(report_types.has_report_type(report_type_uuid) == false);
}

void break_remove_report_type_test()
{
    CLEAN_AND_INSTALL();

    auto report_type_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto report_type = std::make_unique<PlainTextReportType>(report_type_uuid, L"report_type");
    auto& report_type_ref = report_types.add_report_type(std::move(report_type));

    TEST_ASSERT(report_types.has_report_type(report_type_uuid));

    report_types.connect_remove_report_type([&](UUID removed_uuid, Transaction& t){
        throw std::logic_error("");
    }, 0);

    TEST_EXCEPTION(report_types.remove_report_type(report_type_uuid));
    TEST_ASSERT(report_types.has_report_type(report_type_uuid));
}

void collect_report_types_test()
{
    CLEAN_AND_INSTALL();

    auto report_type_uuid_1 = ReportTypeUUID(stl_tools::gen_uuid());
    auto report_type_uuid_2 = ReportTypeUUID(stl_tools::gen_uuid());

    TEST_ASSERT(boost::distance(report_types.get_report_types()) == 0);
    TEST_ASSERT(! report_types.has_report_type(report_type_uuid_1));
    TEST_ASSERT(! report_types.has_report_type(report_type_uuid_2));
    
    report_types.add_report_type(std::make_unique<PlainTextReportType>(report_type_uuid_1, L"report_type_1"));

    TEST_ASSERT(boost::distance(report_types.get_report_types()) == 1);
    TEST_ASSERT(report_types.has_report_type(report_type_uuid_1));
    TEST_ASSERT(! report_types.has_report_type(report_type_uuid_2));
    
    report_types.add_report_type(std::make_unique<PlainTextReportType>(report_type_uuid_2, L"report_type_2"));

    TEST_ASSERT(boost::distance(report_types.get_report_types()) == 2);
    TEST_ASSERT(report_types.has_report_type(report_type_uuid_1));
    TEST_ASSERT(report_types.has_report_type(report_type_uuid_2));

    report_types.remove_report_type(report_type_uuid_1);

    TEST_ASSERT(boost::distance(report_types.get_report_types()) == 1);
    TEST_ASSERT(! report_types.has_report_type(report_type_uuid_1));
    TEST_ASSERT(report_types.has_report_type(report_type_uuid_2));
    
    report_types.remove_report_type(report_type_uuid_2);

    TEST_ASSERT(boost::distance(report_types.get_report_types()) == 0);
    TEST_ASSERT(! report_types.has_report_type(report_type_uuid_1));
    TEST_ASSERT(! report_types.has_report_type(report_type_uuid_2));
}

} //namespace {

test_assist::test_suit get_report_types_suit()
{
    test_assist::test_suit ts(L"report_types_suit");
    ts
        .add({L"add_report_type_test", &add_report_type_test})
        .add({L"break_add_report_type_test", &break_add_report_type_test})
        .add({L"duplicate_report_type_test", &duplicate_report_type_test})
        .add({L"remove_report_type_test", &remove_report_type_test})
        .add({L"break_remove_report_type_test", &break_remove_report_type_test})
        .add({L"collect_report_types_test", &collect_report_types_test});

    return ts;
}