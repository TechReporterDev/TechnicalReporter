#include "stdafx.h"
#include "ResourcePoliciesSuit.h"
#include "CoreTestAssist.h"
namespace {

void update_source_policy_test()
{
    CLEAN_AND_INSTALL();

    auto report_type_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& report_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(report_type_uuid, L"report_type"));

    int update_counter = 0;
    resource_policies.connect_update_resource_policy([&](SubjectRef ref, ResourceRef resource_ref, Transaction& t) {
        TEST_ASSERT(ref == SubjectRef(root));
        TEST_ASSERT(boost::get<ReportTypeRef>(resource_ref) == report_type);
        ++update_counter;
    }, 0);

    TEST_ASSERT(resource_policies.get_enable_policy(root, report_type) == EnablePolicy::DEFAULT_POLICY);
    TEST_ASSERT(resource_policies.is_enabled(root, report_type));

    Transaction t1(db);
    resource_policies.set_enable_policy(root, report_type, EnablePolicy::DISABLE, t1);
    t1.commit();

    TEST_ASSERT(update_counter == 1);
    TEST_ASSERT(resource_policies.get_enable_policy(root, report_type) == EnablePolicy::DISABLE);
    TEST_ASSERT(!resource_policies.is_enabled(root, report_type));

    TEST_ASSERT(resource_policies.get_visibility(root, report_type) == Visibility::DEFAULT_VISIBILITY);
    TEST_ASSERT(resource_policies.is_visible(root, report_type));

    Transaction t2(db);
    resource_policies.set_visibility(root, report_type, Visibility::INVISIBLE, t2);
    t2.commit();
    
    TEST_ASSERT(update_counter == 2);
    TEST_ASSERT(resource_policies.get_visibility(root, report_type) == Visibility::INVISIBLE);
    TEST_ASSERT(!resource_policies.is_visible(root, report_type));

    TEST_ASSERT(resource_policies.get_defer_policy(root, report_type) == DeferPolicy::DEFAULT_POLICY);
    TEST_ASSERT(!resource_policies.is_deferred(root, report_type));

    Transaction t3(db);
    resource_policies.set_defer_policy(root, report_type, DeferPolicy::DEFER, t3);
    t3.commit();

    TEST_ASSERT(update_counter == 3);
    TEST_ASSERT(resource_policies.get_defer_policy(root, report_type) == DeferPolicy::DEFER);
    TEST_ASSERT(resource_policies.is_deferred(root, report_type));
}

void update_role_policy_test()
{
    CLEAN_AND_INSTALL();

    auto report_type_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& report_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(report_type_uuid, L"report_type"));

    int role_update_counter = 0;
    int source_update_counter = 0;
    resource_policies.connect_update_resource_policy([&](SubjectRef ref, ResourceRef resource_ref, Transaction& t) {
        TEST_ASSERT(boost::get<ReportTypeRef>(resource_ref) == report_type);
        if (ref == SubjectRef(root))
        {
            ++source_update_counter;
        }
        else if (ref == SubjectRef(default_role))
        {
            ++role_update_counter;
        }
        else
        {
            TEST_ASSERT(false);
        }           
    }, 0);

    TEST_ASSERT(resource_policies.get_enable_policy(default_role, report_type) == EnablePolicy::DEFAULT_POLICY);
    TEST_ASSERT(resource_policies.is_enabled(default_role, report_type));
    TEST_ASSERT(resource_policies.get_enable_policy(root, report_type) == EnablePolicy::DEFAULT_POLICY);
    TEST_ASSERT(resource_policies.is_enabled(root, report_type));
        
    Transaction t1(db);
    resource_policies.set_enable_policy(default_role, report_type, EnablePolicy::DISABLE, t1);
    t1.commit();

    TEST_ASSERT(role_update_counter == 1 && source_update_counter == 0);
    TEST_ASSERT(resource_policies.get_enable_policy(default_role, report_type) == EnablePolicy::DISABLE);
    TEST_ASSERT(!resource_policies.is_enabled(default_role, report_type));
    TEST_ASSERT(resource_policies.get_enable_policy(root, report_type) == EnablePolicy::DEFAULT_POLICY);
    TEST_ASSERT(!resource_policies.is_enabled(root, report_type));
    

    TEST_ASSERT(resource_policies.get_visibility(default_role, report_type) == Visibility::DEFAULT_VISIBILITY);
    TEST_ASSERT(resource_policies.is_visible(default_role, report_type));
    TEST_ASSERT(resource_policies.get_visibility(root, report_type) == Visibility::DEFAULT_VISIBILITY);
    TEST_ASSERT(resource_policies.is_visible(root, report_type));
    
    Transaction t2(db);
    resource_policies.set_visibility(default_role, report_type, Visibility::INVISIBLE, t2);
    t2.commit();
    
    TEST_ASSERT(role_update_counter == 2 && source_update_counter == 0);
    TEST_ASSERT(resource_policies.get_visibility(default_role, report_type) == Visibility::INVISIBLE);
    TEST_ASSERT(!resource_policies.is_visible(default_role, report_type));
    TEST_ASSERT(resource_policies.get_visibility(root, report_type) == Visibility::DEFAULT_VISIBILITY);
    TEST_ASSERT(!resource_policies.is_visible(root, report_type));

    TEST_ASSERT(resource_policies.get_defer_policy(default_role, report_type) == DeferPolicy::DEFAULT_POLICY);
    TEST_ASSERT(!resource_policies.is_deferred(default_role, report_type));
    TEST_ASSERT(resource_policies.get_defer_policy(root, report_type) == DeferPolicy::DEFAULT_POLICY);
    TEST_ASSERT(!resource_policies.is_deferred(default_role, report_type));

    Transaction t3(db);
    resource_policies.set_defer_policy(default_role, report_type, DeferPolicy::DEFER, t3);
    t3.commit();

    TEST_ASSERT(role_update_counter == 3 && source_update_counter == 0);

    TEST_ASSERT(resource_policies.get_defer_policy(default_role, report_type) == DeferPolicy::DEFER);
    TEST_ASSERT(resource_policies.is_deferred(default_role, report_type));
    TEST_ASSERT(resource_policies.get_defer_policy(root, report_type) == DeferPolicy::DEFAULT_POLICY);
    TEST_ASSERT(resource_policies.is_deferred(default_role, report_type));
}

void update_source_role_test()
{
    CLEAN_AND_INSTALL();

    auto report_type_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& report_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(report_type_uuid, L"report_type"));
    auto role = *roles.add_role({L"role", default_role});

    Transaction t1(db);
    resource_policies.set_enable_policy(role, report_type, EnablePolicy::DISABLE, t1);
    resource_policies.set_visibility(role, report_type, Visibility::INVISIBLE, t1);
    resource_policies.set_defer_policy(role, report_type, DeferPolicy::DEFER, t1);
    t1.commit();

    TEST_ASSERT(resource_policies.is_enabled(root, report_type));
    TEST_ASSERT(resource_policies.is_visible(root, report_type));
    TEST_ASSERT(!resource_policies.is_deferred(root, report_type));

    Transaction t2(db);
    root.set_role(role);
    sources.update_source(root, t2);
    t2.commit();

    TEST_ASSERT(!resource_policies.is_enabled(root, report_type));
    TEST_ASSERT(!resource_policies.is_visible(root, report_type));
    TEST_ASSERT(resource_policies.is_deferred(root, report_type));
}

void update_role_parent_test()
{
    CLEAN_AND_INSTALL();

    auto report_type_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& report_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(report_type_uuid, L"report_type"));

    auto role_1 = *roles.add_role({L"role_1", default_role});
    auto role_2 = *roles.add_role({L"role_2", default_role});

    Transaction t1(db);
    resource_policies.set_enable_policy(role_1, report_type, EnablePolicy::DISABLE, t1);
    resource_policies.set_visibility(role_1, report_type, Visibility::INVISIBLE, t1);
    resource_policies.set_defer_policy(role_1, report_type, DeferPolicy::DEFER, t1);
    root.set_role(role_2);
    sources.update_source(root, t1);
    t1.commit();

    TEST_ASSERT(resource_policies.is_enabled(role_2, report_type));
    TEST_ASSERT(resource_policies.is_visible(role_2, report_type));
    TEST_ASSERT(!resource_policies.is_deferred(role_2, report_type));
    
    TEST_ASSERT(resource_policies.is_enabled(root, report_type));
    TEST_ASSERT(resource_policies.is_visible(root, report_type));
    TEST_ASSERT(!resource_policies.is_deferred(root, report_type));

    Transaction t2(db);
    role_2.set_parent(role_1);
    roles.update_role(role_2, t2);
    t2.commit();

    TEST_ASSERT(!resource_policies.is_enabled(role_2, report_type));
    TEST_ASSERT(!resource_policies.is_visible(role_2, report_type));
    TEST_ASSERT(resource_policies.is_deferred(role_2, report_type));

    TEST_ASSERT(!resource_policies.is_enabled(root, report_type));
    TEST_ASSERT(!resource_policies.is_visible(root, report_type));
    TEST_ASSERT(resource_policies.is_deferred(root, report_type));
}

void remove_subject_policy_test()
{
    CLEAN_AND_INSTALL();

    auto report_type_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    ReportTypeRef report_type_ref = report_types.add_report_type(std::make_unique<PlainTextReportType>(report_type_uuid, L"report_type"));

    auto role = *roles.add_role({L"role", default_role});
    auto source = *sources.add_source({L"source", root.get_ref(), default_role, group_source_type, group_source_type.get_default_config()});

    Transaction t1(db);
    resource_policies.set_enable_policy(default_role, report_type_ref, EnablePolicy::ENABLE, t1);
    resource_policies.set_enable_policy(root, report_type_ref, EnablePolicy::ENABLE, t1);
    resource_policies.set_enable_policy(role, report_type_ref, EnablePolicy::ENABLE, t1);
    resource_policies.set_enable_policy(source, report_type_ref, EnablePolicy::ENABLE, t1);
    t1.commit();

    TEST_ASSERT(resource_policies.get_enable_policy(default_role, report_type_ref) == EnablePolicy::ENABLE);
    TEST_ASSERT(resource_policies.get_enable_policy(root, report_type_ref) == EnablePolicy::ENABLE);
    TEST_ASSERT(resource_policies.get_enable_policy(role, report_type_ref) == EnablePolicy::ENABLE);
    TEST_ASSERT(resource_policies.get_enable_policy(source, report_type_ref) == EnablePolicy::ENABLE);

    sources.remove_source(source.get_ref().get_key());
    TEST_ASSERT(resource_policies.get_enable_policy(source, report_type_ref) == EnablePolicy::DEFAULT_POLICY);

    roles.remove_role(role.get_ref().get_key());
    TEST_ASSERT(resource_policies.get_enable_policy(role, report_type_ref) == EnablePolicy::DEFAULT_POLICY);
    
    report_types.remove_report_type(report_type_ref.get_uuid());
    TEST_ASSERT(resource_policies.get_enable_policy(default_role, report_type_ref) == EnablePolicy::DEFAULT_POLICY);
    TEST_ASSERT(resource_policies.get_enable_policy(root, report_type_ref) == EnablePolicy::DEFAULT_POLICY)
}
} //namespace {

test_assist::test_suit get_resource_policies_suit()
{
    test_assist::test_suit ts(L"report_policies_suit");
    ts
        .add({ L"update_source_policy_test", &update_source_policy_test })
        .add({ L"update_role_policy_test", &update_role_policy_test })
        .add({ L"update_source_role_test", &update_source_role_test })
        .add({ L"update_role_parent_test", &update_role_parent_test })
        .add({ L"remove_subject_policy_test", &remove_subject_policy_test });
    return ts;
}