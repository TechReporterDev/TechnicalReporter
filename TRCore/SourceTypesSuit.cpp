#include "stdafx.h"
#include "SourceTypesSuit.h"
#include "CoreTestAssist.h"
namespace {

void add_source_type_test()
{
    CLEAN_AND_INSTALL();

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    TEST_ASSERT(! source_types.has_source_type(source_type_uuid));
    TEST_EXCEPTION(source_types.get_source_type(source_type_uuid));
    TEST_ASSERT(boost::distance(source_types.get_source_types()) == 1);
    
    auto source_type = std::make_unique<SourceTypeMockup>(source_type_uuid);
    TEST_EXCEPTION(source_type->get_ref());

    int add_counter = 0;
    source_types.connect_add_source_type([&](SourceTypeRef ref, Transaction& t){
        TEST_ASSERT(ref.get_uuid() == source_type_uuid);
        ++add_counter;
    }, 0);

    auto& source_type_ref = source_types.add_source_type(std::move(source_type));
    TEST_NO_EXCEPTION(source_type_ref.get_ref());

    TEST_ASSERT(add_counter == 1);
    TEST_ASSERT(source_types.has_source_type(source_type_uuid));
    TEST_NO_EXCEPTION(source_types.get_source_type(source_type_uuid));
    TEST_ASSERT(boost::distance(source_types.get_source_types()) == 2);
}

void break_add_source_type_test()
{
    CLEAN_AND_INSTALL();

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    source_types.connect_add_source_type([&](SourceTypeRef ref, Transaction& t){
        throw std::logic_error("");
    }, 0);

    TEST_EXCEPTION(source_types.add_source_type(std::make_unique<SourceTypeMockup>(source_type_uuid)));
    TEST_ASSERT(! source_types.has_source_type(source_type_uuid));
}

void duplicate_source_type_test()
{
    CLEAN_AND_INSTALL();

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    source_types.add_source_type(std::make_unique<SourceTypeMockup>(source_type_uuid));
    TEST_ASSERT(source_types.has_source_type(source_type_uuid));
    TEST_EXCEPTION(source_types.add_source_type(std::make_unique<SourceTypeMockup>(source_type_uuid)));
}

void remove_source_type_test()
{
    CLEAN_AND_INSTALL();

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto& source_type_ref = source_types.add_source_type(std::make_unique<SourceTypeMockup>(source_type_uuid));
    TEST_ASSERT(source_types.has_source_type(source_type_uuid));

    int remove_counter = 0;
    source_types.connect_remove_source_type([&](UUID removed_uuid, Transaction& t){
        TEST_ASSERT(source_type_uuid == removed_uuid);
        ++remove_counter;
    }, 0);

    source_types.remove_source_type(source_type_uuid);

    TEST_ASSERT(remove_counter == 1);
    TEST_ASSERT(! source_types.has_source_type(source_type_uuid));
}

void break_remove_source_type_test()
{
    CLEAN_AND_INSTALL();

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto& source_type = source_types.add_source_type(std::make_unique<SourceTypeMockup>(source_type_uuid));
    TEST_ASSERT(source_types.has_source_type(source_type_uuid));

    source_types.connect_remove_source_type([&](UUID removed_uuid, Transaction& t){
        throw std::logic_error("");
    }, 0);

    TEST_EXCEPTION(source_types.remove_source_type(source_type_uuid));
    TEST_ASSERT(source_types.has_source_type(source_type_uuid));
}

void break_remove_download_test()
{
    CLEAN_AND_INSTALL();

    auto report_type_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& report_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(report_type_uuid, L"report_type"));

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    source_type_mockup->push_download(report_type, nullptr);
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));

    TEST_EXCEPTION(report_types.remove_report_type(report_type_uuid));
}

void break_remove_action_test()
{
    CLEAN_AND_INSTALL();

    auto action_uuid = ActionUUID(stl_tools::gen_uuid());
    Transaction t1(db);
    auto& action = actions.add_action(std::make_unique<ActionMockup>(action_uuid), t1);
    t1.commit();

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    source_type_mockup->push_action(action, nullptr);
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));

    Transaction t2(db);
    TEST_EXCEPTION(actions.remove_action(action_uuid, t2));
    t2.rollback();
}

void collect_source_types_test()
{
    CLEAN_AND_INSTALL();

    auto source_type_uuid_1 = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_uuid_2 = SourceTypeUUID(stl_tools::gen_uuid());

    TEST_ASSERT(boost::distance(source_types.get_source_types()) == 1);
    TEST_ASSERT(! source_types.has_source_type(source_type_uuid_1));
    TEST_ASSERT(! source_types.has_source_type(source_type_uuid_2));
    
    source_types.add_source_type(std::make_unique<SourceTypeMockup>(source_type_uuid_1));

    TEST_ASSERT(boost::distance(source_types.get_source_types()) == 2);
    TEST_ASSERT(source_types.has_source_type(source_type_uuid_1));
    TEST_ASSERT(! source_types.has_source_type(source_type_uuid_2));
    
    source_types.add_source_type(std::make_unique<SourceTypeMockup>(source_type_uuid_2));

    TEST_ASSERT(boost::distance(source_types.get_source_types()) == 3);
    TEST_ASSERT(source_types.has_source_type(source_type_uuid_1));
    TEST_ASSERT(source_types.has_source_type(source_type_uuid_2));

    source_types.remove_source_type(source_type_uuid_1);

    TEST_ASSERT(boost::distance(source_types.get_source_types()) == 2);
    TEST_ASSERT(! source_types.has_source_type(source_type_uuid_1));
    TEST_ASSERT(source_types.has_source_type(source_type_uuid_2));
    
    source_types.remove_source_type(source_type_uuid_2);

    TEST_ASSERT(boost::distance(source_types.get_source_types()) == 1);
    TEST_ASSERT(! source_types.has_source_type(source_type_uuid_1));
    TEST_ASSERT(! source_types.has_source_type(source_type_uuid_2));
}

} //namespace {

test_assist::test_suit get_source_types_suit()
{
    test_assist::test_suit ts(L"source_types_suit");
    ts
        .add({L"add_source_type_test", &add_source_type_test})
        .add({L"break_add_source_type_test", &break_add_source_type_test})
        .add({L"duplicate_source_type_test", &duplicate_source_type_test})
        .add({L"remove_source_type_test", &remove_source_type_test})
        .add({L"break_remove_source_type_test", &break_remove_source_type_test})
        .add({L"break_remove_download_test", &break_remove_download_test})
        .add({L"break_remove_action_test", &break_remove_action_test})
        .add({L"collect_source_types_test", &collect_source_types_test});

    return ts;
}