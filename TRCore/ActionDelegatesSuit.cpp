#include "stdafx.h"
#include "ActionDelegatesSuit.h"
#include "CoreTestAssist.h"

namespace {

void add_action_delegate_test()
{
    CLEAN_AND_INSTALL();

    auto host_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto guest_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto input_uuid = ReportTypeUUID(stl_tools::gen_uuid());

    Transaction t1(db);
    auto& host_action = actions.add_action(std::make_unique<ActionMockup>(host_action_uuid), t1);
    auto& guest_action = actions.add_action(std::make_unique<ActionMockup>(guest_action_uuid), t1);
    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(input_uuid, L""), t1);
    t1.commit();

    TEST_EXCEPTION(action_delegates.get_action_delegate({{host_action}, guest_action}));
    TEST_ASSERT(action_delegates.find_by_host_action(host_action).empty());
    TEST_ASSERT(action_delegates.find_by_input(input_type).empty());

    int add_counter = 0;
    action_delegates.connect_add_delegate([&](const ActionDelegate& action_delegate, Transaction& t) {
        TEST_ASSERT(action_delegate.get_host_action_refs() == std::vector<ActionRef>{host_action});
        TEST_ASSERT(action_delegate.get_guest_action_ref() == guest_action);
        ++add_counter;
    }, 0);

    Transaction t2(db);
    std::unique_ptr<ActionDelegateMockup> action_delegate_mockup(new ActionDelegateMockup({input_type}, {host_action}, guest_action));
    auto& action_delegate = action_delegates.add_action_delegate(std::move(action_delegate_mockup), t2);
    t2.commit();

    TEST_ASSERT(add_counter == 1);
    TEST_NO_EXCEPTION(action_delegates.get_action_delegate({{host_action}, guest_action}));
    TEST_ASSERT(boost::distance(action_delegates.find_by_host_action(host_action)) == 1);
    TEST_ASSERT(boost::distance(action_delegates.find_by_input(input_type)) == 1);
}

void break_add_action_delegate_test()
{
    CLEAN_AND_INSTALL();

    auto host_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto guest_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto input_uuid = ReportTypeUUID(stl_tools::gen_uuid());

    Transaction t1(db);
    auto& host_action = actions.add_action(std::make_unique<ActionMockup>(host_action_uuid), t1);
    auto& guest_action = actions.add_action(std::make_unique<ActionMockup>(guest_action_uuid), t1);
    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(input_uuid, L""), t1);
    t1.commit();

    TEST_EXCEPTION(action_delegates.get_action_delegate({{host_action}, guest_action}));
    TEST_ASSERT(action_delegates.find_by_host_action(host_action).empty());
    TEST_ASSERT(action_delegates.find_by_input(input_type).empty());

    Transaction t2(db);
    std::unique_ptr<ActionDelegateMockup> action_delegate_mockup(new ActionDelegateMockup({input_type}, {host_action}, guest_action));
    auto& action_delegate = action_delegates.add_action_delegate(std::move(action_delegate_mockup), t2);
    TEST_NO_EXCEPTION(action_delegates.get_action_delegate({{host_action}, guest_action}));
    TEST_ASSERT(boost::distance(action_delegates.find_by_host_action(host_action)) == 1);
    TEST_ASSERT(boost::distance(action_delegates.find_by_input(input_type)) == 1);
    t2.rollback();

    TEST_EXCEPTION(action_delegates.get_action_delegate({{host_action}, guest_action}));
    TEST_ASSERT(action_delegates.find_by_host_action(host_action).empty());
    TEST_ASSERT(action_delegates.find_by_input(input_type).empty());
}

void duplicate_action_delegate_test()
{
    CLEAN_AND_INSTALL();

    auto host_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto guest_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto input_uuid = ReportTypeUUID(stl_tools::gen_uuid());

    Transaction t1(db);
    auto& host_action = actions.add_action(std::make_unique<ActionMockup>(host_action_uuid), t1);
    auto& guest_action = actions.add_action(std::make_unique<ActionMockup>(guest_action_uuid), t1);
    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(input_uuid, L""), t1);
    std::unique_ptr<ActionDelegateMockup> action_delegate_mockup_1(new ActionDelegateMockup({input_type}, {host_action}, guest_action));
    action_delegates.add_action_delegate(std::move(action_delegate_mockup_1), t1);
    t1.commit();    

    TEST_NO_EXCEPTION(action_delegates.get_action_delegate({{host_action}, guest_action}));
    TEST_ASSERT(boost::distance(action_delegates.find_by_host_action(host_action)) == 1);

    Transaction t2(db);
    std::unique_ptr<ActionDelegateMockup> action_delegate_mockup_2(new ActionDelegateMockup({input_type}, {host_action}, guest_action));
    TEST_EXCEPTION(action_delegates.add_action_delegate(std::move(action_delegate_mockup_2), t2));
    TEST_ASSERT(boost::distance(action_delegates.find_by_host_action(host_action)) == 1);
    t2.rollback();
}

void remove_action_delegate_test()
{
    CLEAN_AND_INSTALL();

    auto host_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto guest_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto input_uuid = ReportTypeUUID(stl_tools::gen_uuid());

    Transaction t1(db);
    auto& host_action = actions.add_action(std::make_unique<ActionMockup>(host_action_uuid), t1);
    auto& guest_action = actions.add_action(std::make_unique<ActionMockup>(guest_action_uuid), t1);
    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(input_uuid, L""), t1);
    std::unique_ptr<ActionDelegateMockup> action_delegate_mockup_1(new ActionDelegateMockup({input_type}, {host_action}, guest_action));
    action_delegates.add_action_delegate(std::move(action_delegate_mockup_1), t1);
    t1.commit();

    TEST_NO_EXCEPTION(action_delegates.get_action_delegate({{host_action}, guest_action}));
    TEST_ASSERT(boost::distance(action_delegates.find_by_host_action(host_action)) == 1);
    TEST_ASSERT(boost::distance(action_delegates.find_by_input(input_type)) == 1);

    int remove_counter = 0;
    action_delegates.connect_remove_delegate([&](ActionDependency action_dependency, Transaction& t){
        TEST_ASSERT(action_dependency.m_host_action_refs == std::vector < ActionRef > {host_action});
        TEST_ASSERT(action_dependency.m_guest_action_ref == guest_action);
        ++remove_counter;
    }, 0);

    Transaction t2(db);
    action_delegates.remove_action_delegate({{host_action}, guest_action}, t2);
    t2.commit();

    TEST_EXCEPTION(action_delegates.get_action_delegate({{host_action}, guest_action}));
    TEST_ASSERT(action_delegates.find_by_host_action(host_action).empty());
    TEST_ASSERT(action_delegates.find_by_input(input_type).empty());
}

void break_remove_action_delegate_test()
{
    CLEAN_AND_INSTALL();

    auto host_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto guest_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto input_uuid = ReportTypeUUID(stl_tools::gen_uuid());

    Transaction t1(db);
    auto& host_action = actions.add_action(std::make_unique<ActionMockup>(host_action_uuid), t1);
    auto& guest_action = actions.add_action(std::make_unique<ActionMockup>(guest_action_uuid), t1);
    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(input_uuid, L""), t1);
    std::unique_ptr<ActionDelegateMockup> action_delegate_mockup_1(new ActionDelegateMockup({input_type}, {host_action}, guest_action));
    action_delegates.add_action_delegate(std::move(action_delegate_mockup_1), t1);
    t1.commit();

    TEST_NO_EXCEPTION(action_delegates.get_action_delegate({{host_action}, guest_action}));
    TEST_ASSERT(boost::distance(action_delegates.find_by_host_action(host_action)) == 1);
    TEST_ASSERT(boost::distance(action_delegates.find_by_input(input_type)) == 1);

    Transaction t2(db);
    action_delegates.remove_action_delegate({{host_action}, guest_action}, t2);
    TEST_EXCEPTION(action_delegates.get_action_delegate({{host_action}, guest_action}));
    TEST_ASSERT(action_delegates.find_by_host_action(host_action).empty());
    TEST_ASSERT(action_delegates.find_by_input(input_type).empty());
    t2.rollback();

    TEST_NO_EXCEPTION(action_delegates.get_action_delegate({{host_action}, guest_action}));
    TEST_ASSERT(boost::distance(action_delegates.find_by_host_action(host_action)) == 1);
    TEST_ASSERT(boost::distance(action_delegates.find_by_input(input_type)) == 1);
}

void break_remove_input_test()
{
    CLEAN_AND_INSTALL();

    auto host_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto guest_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto input_uuid = ReportTypeUUID(stl_tools::gen_uuid());

    Transaction t1(db);
    auto& host_action = actions.add_action(std::make_unique<ActionMockup>(host_action_uuid), t1);
    auto& guest_action = actions.add_action(std::make_unique<ActionMockup>(guest_action_uuid), t1);
    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(input_uuid, L""), t1);
    std::unique_ptr<ActionDelegateMockup> action_delegate_mockup_1(new ActionDelegateMockup({input_type}, {host_action}, guest_action));
    action_delegates.add_action_delegate(std::move(action_delegate_mockup_1), t1);
    t1.commit();

    TEST_EXCEPTION(report_types.remove_report_type(input_uuid));    

    Transaction t3(db);
    action_delegates.remove_action_delegate({{host_action}, guest_action}, t3);
    TEST_NO_EXCEPTION(report_types.remove_report_type(input_uuid, t3)); 
    t3.commit();
}

void break_remove_host_action_test()
{
    CLEAN_AND_INSTALL();

    auto host_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto guest_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto input_uuid = ReportTypeUUID(stl_tools::gen_uuid());

    Transaction t1(db);
    auto& host_action = actions.add_action(std::make_unique<ActionMockup>(host_action_uuid), t1);
    auto& guest_action = actions.add_action(std::make_unique<ActionMockup>(guest_action_uuid), t1);
    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(input_uuid, L""), t1);
    std::unique_ptr<ActionDelegateMockup> action_delegate_mockup_1(new ActionDelegateMockup({input_type}, {host_action}, guest_action));
    action_delegates.add_action_delegate(std::move(action_delegate_mockup_1), t1);
    t1.commit();

    Transaction t2(db);
    TEST_EXCEPTION(actions.remove_action(host_action_uuid, t2));    
    t2.rollback();

    Transaction t3(db);
    action_delegates.remove_action_delegate({{host_action}, guest_action}, t3);
    TEST_NO_EXCEPTION(actions.remove_action(host_action_uuid, t3)); 
    t3.commit();
}

void break_remove_guest_action_test()
{
    CLEAN_AND_INSTALL();

    auto host_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto guest_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto input_uuid = ReportTypeUUID(stl_tools::gen_uuid());

    Transaction t1(db);
    auto& host_action = actions.add_action(std::make_unique<ActionMockup>(host_action_uuid), t1);
    auto& guest_action = actions.add_action(std::make_unique<ActionMockup>(guest_action_uuid), t1);
    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(input_uuid, L""), t1);
    std::unique_ptr<ActionDelegateMockup> action_delegate_mockup_1(new ActionDelegateMockup({input_type}, {host_action}, guest_action));
    action_delegates.add_action_delegate(std::move(action_delegate_mockup_1), t1);
    t1.commit();

    Transaction t2(db);
    TEST_EXCEPTION(actions.remove_action(guest_action_uuid, t2));   
    t2.rollback();

    Transaction t3(db);
    action_delegates.remove_action_delegate({{host_action}, guest_action}, t3);
    TEST_NO_EXCEPTION(actions.remove_action(guest_action_uuid, t3));    
    t3.commit();
}

} //namespace {

test_assist::test_suit get_action_delegates_suit()
{
    test_assist::test_suit ts(L"actions_delegates_suit");
    ts
        .add({L"add_action_delegate_test", &add_action_delegate_test})
        .add({L"break_add_action_delegate_test", &break_add_action_delegate_test})
        .add({L"duplicate_action_delegate_test", &duplicate_action_delegate_test})
        .add({L"remove_action_delegate_test", &remove_action_delegate_test})
        .add({L"break_remove_action_delegate_test", &break_remove_action_delegate_test})
        .add({L"break_remove_input_test", &break_remove_input_test})
        .add({L"break_remove_host_action_test", &break_remove_host_action_test})
        .add({L"break_remove_guest_action_test", &break_remove_guest_action_test});
    return ts;
}