#include "stdafx.h"
#include "ShortcutsSuit.h"
#include "CoreTestAssist.h"

namespace {

void add_shortcut_test()
{
    CLEAN_AND_INSTALL();

    auto action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto output_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto shortcut_uuid = ActionUUID(stl_tools::gen_uuid());

    Transaction t1(db);
    auto& action = actions.add_action(std::make_unique<ActionMockup>(action_uuid), t1);
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(output_uuid, L""), t1);
    t1.commit();

    TEST_EXCEPTION(action_shortcuts.get_shortcut(shortcut_uuid));
    TEST_ASSERT(!action_shortcuts.has_shortcut(shortcut_uuid));
    TEST_ASSERT(action_shortcuts.get_shortcuts().empty());

    int add_counter = 0;
    action_shortcuts.connect_add_shortcut([&](const ActionShortcut& action_shortcut, Transaction& t) {
        TEST_ASSERT(action_shortcut.get_action_ref() == action);
        TEST_ASSERT(action_shortcut.get_output_ref() && *action_shortcut.get_output_ref() == output_type.get_ref());
        ++add_counter;
    }, 0);

    Transaction t2(db);
    std::unique_ptr<ActionShortcutMockup> action_shortcut_mockup(new ActionShortcutMockup(shortcut_uuid, action, output_type.get_ref()));
    auto& action_shortcut = action_shortcuts.add_shortcut(std::move(action_shortcut_mockup), t2);
    t2.commit();

    TEST_ASSERT(add_counter == 1);
    TEST_NO_EXCEPTION(action_shortcuts.get_shortcut(shortcut_uuid));
    TEST_ASSERT(action_shortcuts.has_shortcut(shortcut_uuid));
    TEST_ASSERT(boost::distance(action_shortcuts.get_shortcuts()) == 1);
}

void duplicate_shortcut_test()
{
    CLEAN_AND_INSTALL();

    auto action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto shortcut_uuid = ActionUUID(stl_tools::gen_uuid());

    Transaction t1(db);
    auto& action = actions.add_action(std::make_unique<ActionMockup>(action_uuid), t1);
    auto& action_shortcut = action_shortcuts.add_shortcut(std::make_unique<ActionShortcutMockup>(shortcut_uuid, action), t1);
    t1.commit();

    TEST_ASSERT(action_shortcuts.has_shortcut(shortcut_uuid));

    Transaction t2(db);
    TEST_EXCEPTION(action_shortcuts.add_shortcut(std::make_unique<ActionShortcutMockup>(shortcut_uuid, action), t2));
    t2.rollback();
}

void remove_shortcut_test()
{
    CLEAN_AND_INSTALL();

    auto host_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto shortcut_uuid = ActionUUID(stl_tools::gen_uuid());

    Transaction t1(db);
    auto& host_action = actions.add_action(std::make_unique<ActionMockup>(host_action_uuid), t1);
    auto& action_shortcut = action_shortcuts.add_shortcut(std::make_unique<ActionShortcutMockup>(shortcut_uuid, host_action), t1);
    t1.commit();

    TEST_NO_EXCEPTION(action_shortcuts.get_shortcut(shortcut_uuid));
    TEST_ASSERT(action_shortcuts.has_shortcut(shortcut_uuid));
    TEST_ASSERT(boost::distance(action_shortcuts.get_shortcuts()) == 1);

    int remove_counter = 0;
    action_shortcuts.connect_remove_shortcut([&](UUID removed_shortcut_uuid, Transaction& t){
        TEST_ASSERT(shortcut_uuid == removed_shortcut_uuid);
        ++remove_counter;
    }, 0);

    Transaction t2(db);
    action_shortcuts.remove_shortcut(shortcut_uuid, t2);
    t2.commit();

    TEST_EXCEPTION(action_shortcuts.get_shortcut(shortcut_uuid));
    TEST_ASSERT(!action_shortcuts.has_shortcut(shortcut_uuid));
    TEST_ASSERT(boost::distance(action_shortcuts.get_shortcuts()) == 0);

    Transaction t3(db);
    TEST_EXCEPTION(action_shortcuts.remove_shortcut(shortcut_uuid, t3));
    t3.rollback();
}

void break_remove_host_action_test()
{
    CLEAN_AND_INSTALL();

    auto host_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto shortcut_uuid = ActionUUID(stl_tools::gen_uuid());

    Transaction t1(db);
    auto& host_action = actions.add_action(std::make_unique<ActionMockup>(host_action_uuid), t1);
    auto& action_shortcut = action_shortcuts.add_shortcut(std::make_unique<ActionShortcutMockup>(shortcut_uuid, host_action), t1);
    t1.commit();

    TEST_ASSERT(action_shortcuts.has_shortcut(shortcut_uuid));

    Transaction t2(db);
    TEST_EXCEPTION(actions.remove_action(host_action_uuid, t2));
    t2.rollback();

    Transaction t3(db);
    action_shortcuts.remove_shortcut(shortcut_uuid, t3);
    TEST_NO_EXCEPTION(actions.remove_action(host_action_uuid, t3)); 
    t3.commit();
}

void break_remove_output_test()
{
    CLEAN_AND_INSTALL();

    auto host_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto output_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto shortcut_uuid = ActionUUID(stl_tools::gen_uuid());

    Transaction t1(db);
    auto& host_action = actions.add_action(std::make_unique<ActionMockup>(host_action_uuid), t1);
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(output_uuid, L""), t1);
    auto& action_shortcut = action_shortcuts.add_shortcut(std::make_unique<ActionShortcutMockup>(shortcut_uuid, host_action, output_type.get_ref()), t1);
    t1.commit();

    TEST_ASSERT(action_shortcuts.has_shortcut(shortcut_uuid));

    Transaction t2(db);
    TEST_EXCEPTION(report_types.remove_report_type(output_uuid, t2));
    t2.rollback();

    Transaction t3(db);
    action_shortcuts.remove_shortcut(shortcut_uuid, t3);
    TEST_NO_EXCEPTION(report_types.remove_report_type(output_uuid, t3));    
    t3.commit();
}

} //namespace {

test_assist::test_suit get_shortcuts_suit()
{
    test_assist::test_suit ts(L"shortcuts_suit");
    ts
        .add({L"add_shortcut_test", &add_shortcut_test})
        .add({L"duplicate_shortcut_test", &duplicate_shortcut_test})
        .add({L"remove_shortcut_test", &remove_shortcut_test})
        .add({L"break_remove_host_action_test", &break_remove_host_action_test})
        .add({L"break_remove_output_test", &break_remove_output_test});
    return ts;
}