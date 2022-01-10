#include "stdafx.h"
#include "ActionsSuit.h"
#include "CoreTestAssist.h"

namespace {

void add_action_test()
{
    CLEAN_AND_INSTALL();

    auto action_uuid = ActionUUID(stl_tools::gen_uuid());
    TEST_ASSERT(!actions.has_action(action_uuid));
    TEST_EXCEPTION(actions.get_action(action_uuid));
    TEST_ASSERT(boost::distance(actions.get_actions()) == 0);

    auto action = std::make_unique<ActionMockup>(action_uuid);
    TEST_EXCEPTION(action->get_ref());

    int add_counter = 0;
    actions.connect_add_action([&](ActionRef ref, Transaction& t) {
        TEST_ASSERT(ref.get_uuid() == action_uuid);
        ++add_counter;
    }, 0);

    Transaction t(db);
    auto& action_ref = actions.add_action(std::move(action), t);
    t.commit();
    TEST_NO_EXCEPTION(action_ref.get_ref());

    TEST_ASSERT(add_counter == 1);
    TEST_ASSERT(actions.has_action(action_uuid));
    TEST_NO_EXCEPTION(actions.get_action(action_uuid));
    TEST_ASSERT(boost::distance(actions.get_actions()) == 1);
}

void duplicate_action_test()
{
    CLEAN_AND_INSTALL();

    auto action_uuid = ActionUUID(stl_tools::gen_uuid());
    Transaction t1(db);
    actions.add_action(std::make_unique<ActionMockup>(action_uuid), t1);
    t1.commit();

    Transaction t2(db);
    TEST_EXCEPTION(actions.add_action(std::make_unique<ActionMockup>(action_uuid), t2));
    t2.rollback();
}

void remove_action_test()
{
    CLEAN_AND_INSTALL();

    auto action_uuid = ActionUUID(stl_tools::gen_uuid());
    Transaction t1(db);
    actions.add_action(std::make_unique<ActionMockup>(action_uuid), t1);
    t1.commit();

    int remove_counter = 0;
    actions.connect_remove_action([&](UUID removed_uuid, Transaction& t){
        TEST_ASSERT(action_uuid == removed_uuid);
        ++remove_counter;
    }, 0);

    Transaction t2(db);
    actions.remove_action(action_uuid, t2);
    t2.commit();

    TEST_ASSERT(remove_counter == 1);
    TEST_ASSERT(! actions.has_action(action_uuid));
}

} //namespace {

test_assist::test_suit get_actions_suit()
{
    test_assist::test_suit ts(L"actions_suit");
    ts
        .add({L"add_action_test", &add_action_test})
        .add({L"duplicate_action_test", &duplicate_action_test})
        .add({L"remove_action_test", &remove_action_test});
    return ts;
}