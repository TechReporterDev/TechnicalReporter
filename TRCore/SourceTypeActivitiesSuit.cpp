#include "stdafx.h"
#include "SourceTypesSuit.h"
#include "CoreTestAssist.h"
namespace {

void add_download_activity_test()
{
    CLEAN_AND_INSTALL();

    auto download_1_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& download_1 = report_types.add_report_type(std::make_unique<PlainTextReportType>(download_1_uuid, L""));

    auto download_2_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& download_2 = report_types.add_report_type(std::make_unique<PlainTextReportType>(download_2_uuid, L""));

    auto report_type_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& report_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(report_type_uuid, L""));

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_download(*source_type_mockup, download_1, "");
    push_download(*source_type_mockup, download_2, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));

    std::set<Activity> expected_activities{
        LoadingActivity(download_1),
        LoadingActivity(download_2)
    };

    auto activities = stl_tools::copy_vector(source_type_activities.get_activities(source_type));
    TEST_ASSERT(boost::distance(activities) == 2);
    boost::sort(activities);

    TEST_ASSERT(boost::equal(expected_activities, activities)); 
}

void add_action_activity_test()
{
    CLEAN_AND_INSTALL();

    auto action_uuid = ActionUUID(stl_tools::gen_uuid());
    Transaction t(db);
    auto& action = actions.add_action(std::make_unique<ActionMockup>(action_uuid), t);
    t.commit();

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_action(*source_type_mockup, action);
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));

    auto activities = stl_tools::copy_vector(source_type_activities.get_activities(source_type));
    TEST_ASSERT(boost::distance(activities) == 1);
    TEST_ASSERT(activities.front() == Activity(ActionActivity(action)));
}

void add_selection_activity_test()
{
    CLEAN_AND_INSTALL();

    auto download_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& download = report_types.add_report_type(std::make_unique<PlainTextReportType>(download_uuid, L""));

    auto output_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(output_uuid, L""));

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_download(*source_type_mockup, download, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));
    
    auto selection_uuid = stl_tools::gen_uuid();
    std::set<Activity> expected_activities{
        LoadingActivity(download),
        SelectionActivity({download}, output_type, selection_uuid)
    };

    int add_count = 0;
    source_type_activities.connect_add_activity([&](const SourceTypeActivity& activity, Transaction&)
    {
        TEST_ASSERT(activity.m_source_type_ref == source_type);
        TEST_NO_EXCEPTION(boost::get<SelectionActivity>(activity.m_activity));
        auto& selection_activity = boost::get<SelectionActivity>(activity.m_activity);
        TEST_ASSERT(selection_activity.m_input_refs == std::vector<ReportTypeRef>({download}));
        TEST_ASSERT(selection_activity.m_output_ref == output_type);
        ++add_count;
    }, 0);

    Transaction t(db);
    selections.add_selection(std::make_unique<SelectionMockup>(SelectionDirection(download, output_type), nullptr, selection_uuid), t);
    t.commit();

    TEST_ASSERT(add_count == 1);

    auto activities = stl_tools::copy_vector(source_type_activities.get_activities(source_type));
    TEST_ASSERT(boost::distance(activities) == 2);
    boost::sort(activities);

    TEST_ASSERT(boost::equal(expected_activities, activities)); 
}

void duplicate_activity_output_test()
{
    CLEAN_AND_INSTALL();

    auto download_1_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& download_1 = report_types.add_report_type(std::make_unique<PlainTextReportType>(download_1_uuid, L""));

    auto download_2_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& download_2 = report_types.add_report_type(std::make_unique<PlainTextReportType>(download_2_uuid, L""));

    auto output_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(output_uuid, L""));

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_download(*source_type_mockup, download_1, "");
    push_download(*source_type_mockup, download_2, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));
    
    Transaction t(db);
    selections.add_selection(std::make_unique<SelectionMockup>(SelectionDirection(download_1, output_type)), t);
    TEST_EXCEPTION(selections.add_selection(std::make_unique<SelectionMockup>(SelectionDirection(download_2, output_type)), t));
    t.rollback();
}

void remove_selection_activity_test()
{
    CLEAN_AND_INSTALL();

    auto download_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& download = report_types.add_report_type(std::make_unique<PlainTextReportType>(download_uuid, L""));

    auto output_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(output_uuid, L""));

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_download(*source_type_mockup, download, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));
    
    Transaction t1(db);
    selections.add_selection(std::make_unique<SelectionMockup>(SelectionDirection(download, output_type), nullptr, stl_tools::null_uuid()), t1);
    t1.commit();

    int remove_count = 0;
    source_type_activities.connect_remove_activity([&](SourceTypeRef source_type_ref, ActivityIdentity activity_identity, Transaction&)
    {
        TEST_ASSERT(source_type_ref == source_type);
        TEST_NO_EXCEPTION(boost::get<SelectionIdentity>(activity_identity));
        ++remove_count;
    }, 0);

    auto initial_activities = stl_tools::copy_vector(source_type_activities.get_activities(source_type));
    boost::sort(initial_activities);

    TEST_ASSERT(boost::equal(initial_activities, std::set<Activity>{
        LoadingActivity(download),
        SelectionActivity({download}, output_type, stl_tools::null_uuid())
    }));
    
    Transaction t2(db);
    selections.remove_selection({download, output_type}, t2);
    t2.commit();

    TEST_ASSERT(remove_count == 1);

    auto activities = stl_tools::copy_vector(source_type_activities.get_activities(source_type));
    TEST_ASSERT(activities.size() == 1);
    TEST_ASSERT(activities.front() == Activity(LoadingActivity(download)));
}

void add_grouping_activity_test()
{
    CLEAN_AND_INSTALL();

    auto input_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(input_uuid, L""));

    auto output_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(output_uuid, L""));

    int add_count = 0;
    source_type_activities.connect_add_activity([&](const SourceTypeActivity& activity, Transaction&)
    {
        TEST_ASSERT(activity.m_source_type_ref == group_source_type);
        TEST_NO_EXCEPTION(boost::get<GroupingActivity>(activity.m_activity));
        auto& grouping_activity = boost::get<GroupingActivity>(activity.m_activity);
        TEST_ASSERT(grouping_activity.m_input_ref == input_type);
        TEST_ASSERT(grouping_activity.m_output_ref == output_type);
        ++add_count;
    }, 0);

    Transaction t(db);
    auto grouping_uuid = stl_tools::gen_uuid();
    groupings.add_grouping(std::make_unique<GroupingMockup>(
        input_type, output_type, 
        [](const std::vector<std::shared_ptr<Content>>&)
        {
            return std::unique_ptr<Content>();
        }, 
        grouping_uuid), t);

    t.commit();

    TEST_ASSERT(add_count == 1);

    auto activities = stl_tools::copy_vector(source_type_activities.get_activities(group_source_type));
    TEST_ASSERT(boost::distance(activities) == 1);
    TEST_ASSERT(activities.front() == Activity(GroupingActivity(input_type, output_type, grouping_uuid)));
}

void remove_grouping_activity_test()
{
    CLEAN_AND_INSTALL();

    auto input_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(input_uuid, L""));

    auto output_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(output_uuid, L""));

    Transaction t(db);
    auto grouping_uuid = stl_tools::gen_uuid();
    groupings.add_grouping(std::make_unique<GroupingMockup>(input_type, output_type, [](const std::vector<std::shared_ptr<Content>>&){
        return std::unique_ptr<Content>();
    }, grouping_uuid), t);
    t.commit();

    int remove_count = 0;
    source_type_activities.connect_remove_activity([&](SourceTypeRef source_type_ref, ActivityIdentity activity_identity, Transaction&)
    {
        TEST_ASSERT(source_type_ref == group_source_type);
        TEST_NO_EXCEPTION(boost::get<GroupingIdentity>(activity_identity));
        ++remove_count;
    }, 0);

    auto initial_activities = stl_tools::copy_vector(source_type_activities.get_activities(group_source_type));
    TEST_ASSERT(boost::distance(initial_activities) == 1);
    TEST_ASSERT(initial_activities.front() == Activity(GroupingActivity(input_type, output_type, grouping_uuid)));

    Transaction t2(db);
    groupings.remove_grouping({input_type, output_type}, t2);
    t2.commit();

    TEST_ASSERT(remove_count == 1);

    auto activities = stl_tools::copy_vector(source_type_activities.get_activities(group_source_type));
    TEST_ASSERT(activities.empty());
}

void add_action_delegate_activity_test()
{
    CLEAN_AND_INSTALL();

    auto host_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto guest_action_uuid = ActionUUID(stl_tools::gen_uuid());
    Transaction t(db);
    auto& host_action = actions.add_action(std::make_unique<ActionMockup>(host_action_uuid), t);
    auto& guest_action = actions.add_action(std::make_unique<ActionMockup>(guest_action_uuid), t);
    t.commit();

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_action(*source_type_mockup, host_action);
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));

    int add_count = 0;
    source_type_activities.connect_add_activity([&](const SourceTypeActivity& activity, Transaction&)
    {
        TEST_ASSERT(activity.m_source_type_ref == source_type);
        TEST_ASSERT(activity.m_activity == Activity(ActionDelegateActivity({}, {host_action}, guest_action)));
        ++add_count;
    }, 0);

    Transaction t2(db);
    action_delegates.add_action_delegate(std::unique_ptr<ActionDelegateMockup>(new ActionDelegateMockup({}, {host_action}, guest_action)), t2);
    t2.commit();

    TEST_ASSERT(add_count == 1);
    auto activities = stl_tools::copy_vector(source_type_activities.get_activities(source_type));
    TEST_ASSERT(boost::distance(activities) == 2);
    boost::sort(activities);
    TEST_ASSERT(boost::equal(activities, std::set<Activity>{
        ActionActivity(host_action),
        ActionDelegateActivity({}, {host_action}, guest_action)
    }));
}

void remove_action_delegate_activity_test()
{
    CLEAN_AND_INSTALL();

    auto host_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto guest_action_uuid = ActionUUID(stl_tools::gen_uuid());
    Transaction t(db);
    auto& host_action = actions.add_action(std::make_unique<ActionMockup>(host_action_uuid), t);
    auto& guest_action = actions.add_action(std::make_unique<ActionMockup>(guest_action_uuid), t);
    action_delegates.add_action_delegate(std::unique_ptr<ActionDelegateMockup>(new ActionDelegateMockup({}, {host_action}, guest_action)), t);
    t.commit();

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_action(*source_type_mockup, host_action);
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));

    int remove_count = 0;
    source_type_activities.connect_remove_activity([&](SourceTypeRef source_type_ref, ActivityIdentity activity_identity, Transaction&)
    {
        TEST_ASSERT(source_type_ref == source_type);
        TEST_ASSERT(activity_identity == ActivityIdentity(ActionDelegateIdentity({host_action_uuid}, guest_action_uuid)));
        ++remove_count;
    }, 0);

    auto initial_activities = stl_tools::copy_vector(source_type_activities.get_activities(source_type));
    TEST_ASSERT(boost::distance(initial_activities) == 2);
    boost::sort(initial_activities);
    TEST_ASSERT(boost::equal(initial_activities, std::set<Activity>{
        ActionActivity(host_action),
        ActionDelegateActivity({}, {host_action}, guest_action)
    }));

    Transaction t2(db);
    action_delegates.remove_action_delegate({{host_action}, guest_action}, t2);
    t2.commit();

    TEST_ASSERT(remove_count == 1);
    auto activities = stl_tools::copy_vector(source_type_activities.get_activities(source_type));
    TEST_ASSERT(boost::distance(activities) == 1);
    TEST_ASSERT(activities.front() == Activity(ActionActivity(host_action)));
}

void add_multi_host_action_delegate_activity_test()
{
    CLEAN_AND_INSTALL();

    auto host_action_1_uuid = ActionUUID(stl_tools::gen_uuid());
    auto host_action_2_uuid = ActionUUID(stl_tools::gen_uuid());
    auto guest_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto source_type_1_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_2_uuid = SourceTypeUUID(stl_tools::gen_uuid());

    Transaction t(db);
    auto& host_action_1 = actions.add_action(std::make_unique<ActionMockup>(host_action_1_uuid), t);
    auto& host_action_2 = actions.add_action(std::make_unique<ActionMockup>(host_action_2_uuid), t);
    auto& guest_action = actions.add_action(std::make_unique<ActionMockup>(guest_action_uuid), t);

    auto source_type_1_mockup = std::make_unique<SourceTypeMockup>(source_type_1_uuid);
    push_action(*source_type_1_mockup, host_action_1);
    push_action(*source_type_1_mockup, host_action_2);
    auto& source_type_1 = source_types.add_source_type(std::move(source_type_1_mockup), t);

    auto source_type_2_mockup = std::make_unique<SourceTypeMockup>(source_type_2_uuid);
    push_action(*source_type_2_mockup, host_action_1);
    auto& source_type_2 = source_types.add_source_type(std::move(source_type_2_mockup), t);
    
    action_delegates.add_action_delegate(std::unique_ptr<ActionDelegateMockup>(new ActionDelegateMockup({}, {host_action_1, host_action_2}, guest_action)), t);
    t.commit();

    auto source_type_1_activities = stl_tools::copy_vector(source_type_activities.get_activities(source_type_1));
    boost::sort(source_type_1_activities);
    TEST_ASSERT(boost::equal(source_type_1_activities, std::set<Activity>{
        ActionActivity(host_action_1),
        ActionActivity(host_action_2),
        ActionDelegateActivity({}, {host_action_1, host_action_2}, guest_action)
    }));

    auto source_type_2_activities = stl_tools::copy_vector(source_type_activities.get_activities(source_type_2));
    TEST_ASSERT(boost::equal(source_type_2_activities, std::set<Activity>{
        ActionActivity(host_action_1)
    }));
}

void add_multi_input_action_delegate_activity_test()
{
    CLEAN_AND_INSTALL();

    auto host_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto guest_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto input_1_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto input_2_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto source_type_1_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_2_uuid = SourceTypeUUID(stl_tools::gen_uuid());

    Transaction t(db);
    auto& host_action = actions.add_action(std::make_unique<ActionMockup>(host_action_uuid), t);
    auto& guest_action = actions.add_action(std::make_unique<ActionMockup>(guest_action_uuid), t);
    auto& input_1 = report_types.add_report_type(std::make_unique<PlainTextReportType>(input_1_uuid, L""), t);
    auto& input_2 = report_types.add_report_type(std::make_unique<PlainTextReportType>(input_2_uuid, L""), t);

    auto source_type_1_mockup = std::make_unique<SourceTypeMockup>(source_type_1_uuid);
    push_action(*source_type_1_mockup, host_action);
    push_download(*source_type_1_mockup, input_1, "");
    push_download(*source_type_1_mockup, input_2, "");
    auto& source_type_1 = source_types.add_source_type(std::move(source_type_1_mockup), t);

    auto source_type_2_mockup = std::make_unique<SourceTypeMockup>(source_type_2_uuid);
    push_action(*source_type_2_mockup, host_action);
    push_download(*source_type_2_mockup, input_1, "");
    auto& source_type_2 = source_types.add_source_type(std::move(source_type_2_mockup), t);

    action_delegates.add_action_delegate(std::unique_ptr<ActionDelegateMockup>(new ActionDelegateMockup({input_1, input_2}, {host_action}, guest_action)), t);
    t.commit();
        
    auto source_type_1_activities = stl_tools::copy_vector(source_type_activities.get_activities(source_type_1));
    boost::sort(source_type_1_activities);
    TEST_ASSERT(boost::equal(source_type_1_activities, std::set<Activity>{
        LoadingActivity(input_1),
        LoadingActivity(input_2),
        ActionActivity(host_action),
        ActionDelegateActivity({input_1, input_2}, {host_action}, guest_action)
    }));

    auto source_type_2_activities = stl_tools::copy_vector(source_type_activities.get_activities(source_type_2));
    TEST_ASSERT(boost::equal(source_type_2_activities, std::set<Activity>{
        LoadingActivity(input_1),
        ActionActivity(host_action)
    }));
}

void add_action_delegates_chain_test()
{
    CLEAN_AND_INSTALL();

    auto host_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto guest_action_1_uuid = ActionUUID(stl_tools::gen_uuid());
    auto guest_action_2_uuid = ActionUUID(stl_tools::gen_uuid());
    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());

    Transaction t(db);
    auto& host_action = actions.add_action(std::make_unique<ActionMockup>(host_action_uuid), t);
    auto& guest_action_1 = actions.add_action(std::make_unique<ActionMockup>(guest_action_1_uuid), t);
    auto& guest_action_2 = actions.add_action(std::make_unique<ActionMockup>(guest_action_2_uuid), t);

    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_action(*source_type_mockup, host_action);
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup), t);
    t.commit();
    
    auto source_type_activities_1 = stl_tools::copy_vector(source_type_activities.get_activities(source_type));
    TEST_ASSERT(boost::equal(source_type_activities_1, std::set<Activity>{
        ActionActivity(host_action),
    }));

    Transaction t2(db);
    action_delegates.add_action_delegate(std::unique_ptr<ActionDelegateMockup>(new ActionDelegateMockup({}, {guest_action_1}, guest_action_2)), t2);
    t2.commit();

    auto source_type_activities_2 = stl_tools::copy_vector(source_type_activities.get_activities(source_type));
    TEST_ASSERT(boost::equal(source_type_activities_2, std::set<Activity>{
        ActionActivity(host_action),
    }));

    Transaction t3(db);
    action_delegates.add_action_delegate(std::unique_ptr<ActionDelegateMockup>(new ActionDelegateMockup({}, {host_action}, guest_action_1)), t3);
    t3.commit();

    auto source_type_activities_3 = stl_tools::copy_vector(source_type_activities.get_activities(source_type));
    boost::sort(source_type_activities_3);
    TEST_ASSERT(boost::equal(source_type_activities_3, std::set<Activity>{
        ActionActivity(host_action),
        ActionDelegateActivity({}, {host_action}, guest_action_1),
        ActionDelegateActivity({}, {guest_action_1}, guest_action_2)
    }));

    Transaction t4(db);
    action_delegates.remove_action_delegate({{host_action}, guest_action_1}, t4);
    t4.commit();

    auto source_type_activities_4 = stl_tools::copy_vector(source_type_activities.get_activities(source_type));
    TEST_ASSERT(boost::equal(source_type_activities_4, std::set<Activity>{
        ActionActivity(host_action),
    }));
}

void add_action_shortcut_activity_test()
{
    CLEAN_AND_INSTALL();

    auto host_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto output_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto shortcut_uuid = UUID(stl_tools::gen_uuid());

    Transaction t1(db);
    auto& host_action = actions.add_action(std::make_unique<ActionMockup>(host_action_uuid), t1);
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(output_uuid, L""), t1);
    t1.commit();

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_action(*source_type_mockup, host_action);
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));

    int add_count = 0;
    source_type_activities.connect_add_activity([&](const SourceTypeActivity& activity, Transaction&)
    {
        TEST_ASSERT(activity.m_source_type_ref == source_type);
        TEST_NO_EXCEPTION(boost::get<const ShortcutActivity>(activity.m_activity));
        auto& shortcut_activity = boost::get<const ShortcutActivity>(activity.m_activity);
        TEST_ASSERT(shortcut_activity.m_shortcut_ref.get_uuid() == shortcut_uuid);
        TEST_ASSERT(shortcut_activity.m_host_action_ref == host_action);
        TEST_ASSERT(shortcut_activity.m_output_ref == output_type.get_ref());
        ++add_count;
    }, 0);

    Transaction t2(db);
    auto& shortcut = action_shortcuts.add_shortcut(std::make_unique<ActionShortcutMockup>(shortcut_uuid, host_action, output_type.get_ref()), t2);
    t2.commit();

    TEST_ASSERT(add_count == 1);
    auto activities = stl_tools::copy_vector(source_type_activities.get_activities(source_type));
    TEST_ASSERT(boost::distance(activities) == 2);
    boost::sort(activities);
    TEST_ASSERT(boost::equal(activities, std::set<Activity>{
        ActionActivity(host_action),
        ShortcutActivity(shortcut, host_action, output_type.get_ref())
    }));
}

void remove_action_shortcut_activity_test()
{
    CLEAN_AND_INSTALL();

    auto host_action_uuid = ActionUUID(stl_tools::gen_uuid());
    auto output_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto shortcut_uuid = UUID(stl_tools::gen_uuid());

    Transaction t1(db);
    auto& host_action = actions.add_action(std::make_unique<ActionMockup>(host_action_uuid), t1);
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(output_uuid, L""), t1);
    auto& shortcut = action_shortcuts.add_shortcut(std::make_unique<ActionShortcutMockup>(shortcut_uuid, host_action, output_type.get_ref()), t1);
    t1.commit();

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_action(*source_type_mockup, host_action);
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));

    int remove_count = 0;
    source_type_activities.connect_remove_activity([&](SourceTypeRef source_type_ref, ActivityIdentity activity_identity, Transaction&)
    {
        TEST_ASSERT(source_type_ref == source_type);
        TEST_ASSERT(activity_identity == ActivityIdentity(ShortcutIdentity(shortcut_uuid) ));
        ++remove_count;
    }, 0);

    auto initial_activities = stl_tools::copy_vector(source_type_activities.get_activities(source_type));
    TEST_ASSERT(boost::distance(initial_activities) == 2);
    boost::sort(initial_activities);
    TEST_ASSERT(boost::equal(initial_activities, std::set<Activity>{
        ActionActivity(host_action),
        ShortcutActivity(shortcut, host_action, output_type.get_ref())
    }));

    Transaction t2(db);
    action_shortcuts.remove_shortcut(shortcut_uuid, t2);
    t2.commit();

    TEST_ASSERT(remove_count == 1);
    auto activities = stl_tools::copy_vector(source_type_activities.get_activities(source_type));
    TEST_ASSERT(boost::distance(activities) == 1);
    TEST_ASSERT(activities.front() == Activity(ActionActivity(host_action)));
}

void remove_source_type_test()
{
    CLEAN_AND_INSTALL();

    auto download_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& download = report_types.add_report_type(std::make_unique<PlainTextReportType>(download_uuid, L""));

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_download(*source_type_mockup, download, "");
    SourceTypeRef source_type_ref = source_types.add_source_type(std::move(source_type_mockup));

    TEST_ASSERT(!source_type_activities.get_activities(source_type_ref).empty());
    source_types.remove_source_type(source_type_uuid);
    TEST_ASSERT(source_type_activities.get_activities(source_type_ref).empty());
}

void strict_selection_activity_test()
{
    CLEAN_AND_INSTALL();

    auto download_1_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& download_1 = report_types.add_report_type(std::make_unique<PlainTextReportType>(download_1_uuid, L""));

    auto download_2_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& download_2 = report_types.add_report_type(std::make_unique<PlainTextReportType>(download_2_uuid, L""));

    auto output_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(output_uuid, L""));

    auto source_type_1_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_1_mockup = std::make_unique<SourceTypeMockup>(source_type_1_uuid);
    push_download(*source_type_1_mockup, download_1, "");
    push_download(*source_type_1_mockup, download_2, "");
    auto& source_type_1 = source_types.add_source_type(std::move(source_type_1_mockup));
    
    std::set<Activity> expected_activities_1{
        LoadingActivity(download_1),
        LoadingActivity(download_2),
        SelectionActivity({download_1, download_2}, output_type, stl_tools::null_uuid())
    };

    auto source_type_2_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_2_mockup = std::make_unique<SourceTypeMockup>(source_type_2_uuid);
    push_download(*source_type_2_mockup, download_1, "");
    auto& source_type_2 = source_types.add_source_type(std::move(source_type_2_mockup));
    
    std::set<Activity> expected_activities_2{
        LoadingActivity(download_1)
    };

    auto source_type_3_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto& source_type_3 = source_types.add_source_type(std::make_unique<SourceTypeMockup>(source_type_3_uuid));
    
    Transaction t(db);
    selections.add_selection(std::make_unique<SelectionMockup>(SelectionDirection({download_1, download_2}, output_type), nullptr, stl_tools::null_uuid()), t);
    t.commit();

    auto source_type_1_activities = stl_tools::copy_vector(source_type_activities.get_activities(source_type_1));
    TEST_ASSERT(boost::distance(source_type_1_activities) == 3);
    boost::sort(source_type_1_activities);
    TEST_ASSERT(boost::equal(expected_activities_1, source_type_1_activities));

    auto source_type_2_activities = stl_tools::copy_vector(source_type_activities.get_activities(source_type_2));
    TEST_ASSERT(boost::distance(source_type_2_activities) == 1);
    boost::sort(source_type_2_activities);
    TEST_ASSERT(boost::equal(expected_activities_2, source_type_2_activities)); 

    auto source_type_3_activities = stl_tools::copy_vector(source_type_activities.get_activities(source_type_3));
    TEST_ASSERT(boost::distance(source_type_3_activities) == 0);
}

void not_strict_selection_activity_test()
{
    CLEAN_AND_INSTALL();

    auto download_1_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& download_1 = report_types.add_report_type(std::make_unique<PlainTextReportType>(download_1_uuid, L""));

    auto download_2_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& download_2 = report_types.add_report_type(std::make_unique<PlainTextReportType>(download_2_uuid, L""));

    auto output_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(output_uuid, L""));

    auto source_type_1_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_1_mockup = std::make_unique<SourceTypeMockup>(source_type_1_uuid);
    push_download(*source_type_1_mockup, download_1, "");
    push_download(*source_type_1_mockup, download_2, "");
    auto& source_type_1 = source_types.add_source_type(std::move(source_type_1_mockup));
    
    std::set<Activity> expected_activities_1{
        LoadingActivity(download_1),
        LoadingActivity(download_2),
        SelectionActivity({download_1, download_2}, output_type, stl_tools::null_uuid())
    };

    auto source_type_2_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_2_mockup = std::make_unique<SourceTypeMockup>(source_type_2_uuid);
    push_download(*source_type_2_mockup, download_1, "");
    auto& source_type_2 = source_types.add_source_type(std::move(source_type_2_mockup));
    
    std::set<Activity> expected_activities_2{
        LoadingActivity(download_1),
        SelectionActivity({download_1, download_2}, output_type, stl_tools::null_uuid())
    };

    auto source_type_3_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto& source_type_3 = source_types.add_source_type(std::make_unique<SourceTypeMockup>(source_type_3_uuid));
    
    Transaction t(db);
    auto selection_plan = SelectionPlan({
        { download_1, SelectionInput::Mode::OPTIONAL_MODE },
        { download_2, SelectionInput::Mode::OPTIONAL_MODE } },
        output_type);

    selections.add_selection(std::make_unique<SelectionMockup>(selection_plan, nullptr, stl_tools::null_uuid()), t);
    t.commit();

    auto source_type_1_activities = stl_tools::copy_vector(source_type_activities.get_activities(source_type_1));
    TEST_ASSERT(boost::distance(source_type_1_activities) == 3);
    boost::sort(source_type_1_activities);
    TEST_ASSERT(boost::equal(expected_activities_1, source_type_1_activities));

    auto source_type_2_activities = stl_tools::copy_vector(source_type_activities.get_activities(source_type_2));
    TEST_ASSERT(boost::distance(source_type_2_activities) == 2);
    boost::sort(source_type_2_activities);
    TEST_ASSERT(boost::equal(expected_activities_2, source_type_2_activities)); 

    auto source_type_3_activities = stl_tools::copy_vector(source_type_activities.get_activities(source_type_3));
    TEST_ASSERT(boost::distance(source_type_3_activities) == 0);
}

void selection_diamond_test()
{
    CLEAN_AND_INSTALL();

    auto download_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& download = report_types.add_report_type(std::make_unique<PlainTextReportType>(download_uuid, L""));

    auto top_report_type_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& top_report_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(top_report_type_uuid, L""));

    auto left_report_type_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& left_report_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(left_report_type_uuid, L""));

    auto right_report_type_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& right_report_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(right_report_type_uuid, L""));

    auto bottom_report_type_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& bottom_report_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(bottom_report_type_uuid, L""));

    
    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_download(*source_type_mockup, download, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));

    Transaction t(db);
    auto selection_1_uuid = stl_tools::gen_uuid();
    selections.add_selection(std::make_unique<SelectionMockup>(
        SelectionDirection({left_report_type, right_report_type}, bottom_report_type), nullptr, selection_1_uuid), t);
    auto selection_2_uuid = stl_tools::gen_uuid();
    selections.add_selection(std::make_unique<SelectionMockup>(
        SelectionDirection(top_report_type, left_report_type), nullptr, selection_2_uuid), t);
    auto selection_3_uuid = stl_tools::gen_uuid();
    selections.add_selection(std::make_unique<SelectionMockup>(
        SelectionDirection(top_report_type, right_report_type), nullptr, selection_3_uuid), t);
    auto selection_4_uuid = stl_tools::gen_uuid();
    selections.add_selection(std::make_unique<SelectionMockup>(
        SelectionDirection(download, top_report_type), nullptr, selection_4_uuid), t);
    t.commit();

    auto source_type_activities_1 = stl_tools::copy_vector(source_type_activities.get_activities(source_type));
    TEST_ASSERT(boost::distance(source_type_activities_1) == 5);
    boost::sort(source_type_activities_1);
    TEST_ASSERT(boost::equal(source_type_activities_1, std::set<Activity>{
        LoadingActivity(download),
        SelectionActivity({download}, top_report_type, selection_4_uuid),
        SelectionActivity({top_report_type}, right_report_type, selection_3_uuid),
        SelectionActivity({top_report_type}, left_report_type, selection_2_uuid),
        SelectionActivity({left_report_type, right_report_type}, bottom_report_type, selection_1_uuid)
    }));

    Transaction t2(db);
    selections.remove_selection({download, top_report_type}, t);
    t2.commit();
    
    auto source_type_activities_2 = stl_tools::copy_vector(source_type_activities.get_activities(source_type));
    TEST_ASSERT(boost::distance(source_type_activities_2) == 1);
    TEST_ASSERT(source_type_activities_2.front() == Activity(LoadingActivity(download)));
}

} //namespace {

test_assist::test_suit get_source_type_activities_suit()
{
    test_assist::test_suit ts(L"source_type_activities_suit");
    ts
        .add({L"add_download_activity_test", &add_download_activity_test})
        .add({L"add_action_activity_test", &add_action_activity_test})
        .add({L"add_selection_activity_test", &add_selection_activity_test})
        .add({L"duplicate_activity_output_test", &duplicate_activity_output_test})
        .add({L"remove_selection_activity_test", &remove_selection_activity_test})
        .add({L"add_grouping_activity_test", &add_grouping_activity_test})
        .add({L"remove_grouping_activity_test", &remove_grouping_activity_test})
        .add({L"add_action_delegate_activity_test", &add_action_delegate_activity_test})
        .add({L"remove_action_delegate_activity_test", &remove_action_delegate_activity_test})
        .add({L"add_multi_host_action_delegate_activity_test", &add_multi_host_action_delegate_activity_test})
        .add({L"add_multi_input_action_delegate_activity_test", &add_multi_input_action_delegate_activity_test})
        .add({L"add_action_delegates_chain_test", &add_action_delegates_chain_test})
        .add({L"add_action_shortcut_activity_test", &add_action_shortcut_activity_test})
        .add({L"remove_action_shortcut_activity_test", &remove_action_shortcut_activity_test})
        .add({L"remove_source_type_test", &remove_source_type_test})
        .add({L"strict_selection_activity_test", &strict_selection_activity_test})
        .add({L"not_strict_selection_activity_test", &not_strict_selection_activity_test})
        .add({L"selection_diamond_test", &selection_diamond_test});

    return ts;
}