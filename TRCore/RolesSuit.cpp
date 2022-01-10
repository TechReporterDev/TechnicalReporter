#include "stdafx.h"
#include "ReportTypesSuit.h"
#include "CoreTestAssist.h"
namespace {

void add_role_test()
{
    CLEAN_AND_INSTALL();

    int add_counter = 0;
    roles.connect_add_role([&](RoleRef ref, Transaction& t) {
        ++add_counter;
    }, 0);

    TEST_ASSERT(boost::distance(roles.get_roles()) == 1); // only root

    Role role(L"role", default_role);
    TEST_EXCEPTION(role.get_ref()); 
    
    auto role_ref = roles.add_role(role);
    
    TEST_ASSERT(boost::distance(roles.get_roles()) == 2);   
    TEST_ASSERT(add_counter == 1);
    TEST_ASSERT(roles.has_role(role_ref.get_key()));
        
    auto added_role = roles.get_role(role_ref.get_key());
    
    TEST_ASSERT(added_role.get_ref() == role_ref);
    TEST_ASSERT(added_role.get_name() == L"role");
    TEST_ASSERT(added_role.has_parent());
    TEST_ASSERT(added_role.get_parent() == default_role);
}

void update_role_test()
{
    CLEAN_AND_INSTALL();

    auto parent_role = *roles.add_role({L"parent_role", default_role});
    auto role = *roles.add_role({L"role", default_role});
        
    int update_counter = 0;
    roles.connect_update_role([&](RoleRef ref, Transaction& t) {
        TEST_ASSERT(ref == role);
        ++update_counter;
    }, 0);

    int replace_counter = 0;
    roles.connect_replace_role([&](RoleRef ref, RoleRef prev_parent_ref, RoleRef next_parent_ref, Transaction& t){
        TEST_ASSERT(ref == role);
        TEST_ASSERT(prev_parent_ref == default_role);
        TEST_ASSERT(next_parent_ref == parent_role);
        ++replace_counter;
    }, 0);

    role.set_name(L"updated_role");
    role.set_parent(parent_role);
    roles.update_role(role);

    auto updated_role = roles.get_role(role.get_ref().get_key());
    TEST_ASSERT(update_counter == 1);
    TEST_ASSERT(replace_counter == 1);
    TEST_ASSERT(updated_role.get_ref() == role.get_ref());
    TEST_ASSERT(updated_role.get_name() == L"updated_role");
    TEST_ASSERT(updated_role.get_parent() == parent_role);
}

void remove_role_test()
{
    CLEAN_AND_INSTALL();

    auto role_ref = roles.add_role({L"role", default_role});
    auto subrole_ref = roles.add_role({L"subrole", role_ref});
        
    int remove_counter = 0;
    roles.connect_remove_role([&](RoleKey role_key, Transaction& t) {
        TEST_ASSERT(role_ref.get_key() == role_key || subrole_ref.get_key() == role_key);
        ++remove_counter;
    }, 0);

    TEST_ASSERT(boost::distance(roles.get_roles()) == 3);
    roles.remove_role(role_ref.get_key());

    TEST_ASSERT(remove_counter == 2);
    TEST_ASSERT(boost::distance(roles.get_roles()) == 1);
    TEST_ASSERT(!roles.has_role(role_ref.get_key()));
    TEST_ASSERT(!roles.has_role(subrole_ref.get_key()));
}

void roles_tree_test()
{
    CLEAN_AND_INSTALL();

    TEST_ASSERT(boost::distance(roles.get_roles()) == 1); // only root
    TEST_ASSERT(boost::distance(roles.get_roles(default_role)) == 0);

    auto role_1 = roles.add_role({L"role_1", default_role});

    TEST_ASSERT(boost::distance(roles.get_roles()) == 2);
    TEST_ASSERT(boost::distance(roles.get_roles(default_role)) == 1);

    auto role_2 = roles.add_role({L"role_2", role_1});

    TEST_ASSERT(boost::distance(roles.get_roles()) == 3);
    TEST_ASSERT(boost::distance(roles.get_roles(default_role)) == 1);
    TEST_ASSERT(boost::distance(roles.get_roles(role_1)) == 1);
    
    auto role_3 = roles.add_role({L"role_3", role_1});
    auto role_4 = *roles.add_role({L"role_4", role_1});
    
    TEST_ASSERT(boost::distance(roles.get_roles()) == 5);
    TEST_ASSERT(boost::distance(roles.get_roles(default_role)) == 1);
    TEST_ASSERT(boost::distance(roles.get_roles(role_1)) == 3);
    TEST_ASSERT(boost::distance(roles.get_roles(role_2)) == 0);
    
    role_4.set_parent(default_role);
    roles.update_role(role_4);
    
    TEST_ASSERT(boost::distance(roles.get_roles()) == 5);
    TEST_ASSERT(boost::distance(roles.get_roles(default_role)) == 2);
    TEST_ASSERT(boost::distance(roles.get_roles(role_1)) == 2);
    
    roles.remove_role(role_1.get_key());
    
    TEST_ASSERT(boost::distance(roles.get_roles()) == 2); //default_role and role_4
    TEST_ASSERT(boost::distance(roles.get_roles(default_role)) == 1);
}

void cyclic_dependency_test()
{
    CLEAN_AND_INSTALL();
    auto role_1 = *roles.add_role({L"role_1", default_role});
    role_1.set_parent(role_1);
    TEST_EXCEPTION(roles.update_role(role_1));

    auto role_2 = *roles.add_role({L"role_2", role_1});
    role_1.set_parent(role_2);
    TEST_EXCEPTION(roles.update_role(role_1));
}

} //namespace {

test_assist::test_suit get_roles_suit()
{
    test_assist::test_suit ts(L"roles_suit");
    ts
        .add({ L"add_role_test", &add_role_test })
        .add({ L"update_role_test", &update_role_test })
        .add({ L"remove_role_test", &remove_role_test })
        .add({ L"roles_tree_test", &roles_tree_test })
        .add({L"cyclic_dependency_test", &cyclic_dependency_test});

    return ts;
}