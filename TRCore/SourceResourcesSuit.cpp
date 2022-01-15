#include "stdafx.h"
#include "SourceResourcesSuit.h"
#include "CoreTestAssist.h"
namespace {

void add_loading_output_test()
{
    CLEAN_AND_INSTALL();

    auto download_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& download = report_types.add_report_type(std::make_unique<PlainTextReportType>(download_uuid, L""));

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_download(*source_type_mockup, download, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));

    int update_count = 0;
    source_resources.connect_update_source_resource([&](const SourceResource& source_resource, Transaction& t){
        ++update_count;
    }, 0);

    auto source_1 = *sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});
    auto source_1_resources = stl_tools::copy_vector(source_resources.get_source_resources(source_1));

    TEST_ASSERT(update_count == 1);
    TEST_ASSERT(source_1_resources.size() == 1);
    TEST_ASSERT(source_1_resources[0] == SourceResource(
        source_1,
        ResourceRef(download),
        LoadingActivity(download),
        true,
        true,
        true,
        false,
        true,
        stl_tools::null_uuid()
    ));
}

void add_selection_test()
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

    auto source_1 = *sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});
    auto source_1_resources = stl_tools::copy_vector(source_resources.get_source_resources(source_1));
    TEST_ASSERT(source_1_resources.size() == 2);
    
    boost::sort(source_1_resources);
    TEST_ASSERT(boost::equal(source_1_resources, std::set<SourceResource>{
        {
            source_1,
            ResourceRef(download),
            LoadingActivity(download),
            true,
            true,
            true,
            false,
            true,
            stl_tools::null_uuid()
        },
        {
            source_1,
            ResourceRef(output_type),
            SelectionActivity({download}, output_type, stl_tools::null_uuid()),
            true,
            true,
            true,
            false,
            true,
            stl_tools::null_uuid()
        }   
    }));
}

void add_grouping_test()
{
    CLEAN_AND_INSTALL();

    auto input_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(input_uuid, L""));

    auto output_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(output_uuid, L""));

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto& source_type = source_types.add_source_type(std::make_unique<SourceTypeMockup>(source_type_uuid));

    Transaction t1(db);
    auto grouping_uuid = stl_tools::gen_uuid();
    groupings.add_grouping(std::make_unique<GroupingMockup>(input_type, output_type, [](const std::vector<std::shared_ptr<Content>>&){
        return std::unique_ptr<Content>();
    }, grouping_uuid), t1);
    t1.commit();

    auto source_1 = *sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});
    auto source_1_resources = stl_tools::copy_vector(source_resources.get_source_resources(source_1));
    TEST_ASSERT(source_1_resources.size() == 1);
    TEST_ASSERT(source_1_resources[0] == SourceResource(
            source_1,
            ResourceRef(output_type),
            GroupingActivity(input_type, output_type, grouping_uuid),
            true,
            true,
            true,
            false,
            true,
            stl_tools::null_uuid()
    ));
}

void add_shortcut_test()
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

    auto source_1 = *sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});
    auto source_1_resources = stl_tools::copy_vector(source_resources.get_source_resources(source_1));
    TEST_ASSERT(source_1_resources.size() == 1);
    TEST_ASSERT(source_1_resources[0] == SourceResource(
            source_1,
            ResourceRef(output_type),
            ShortcutActivity(shortcut, host_action, output_type.get_ref()),
            true,
            true,
            true,
            false,
            true,
            stl_tools::null_uuid()
    ));
}

void enable_resource_policy_test()
{
    CLEAN_AND_INSTALL();

    auto download_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& download = report_types.add_report_type(std::make_unique<PlainTextReportType>(download_uuid, L""));

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_download(*source_type_mockup, download, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));

    auto source_1 = *sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});

    int update_count = 0;
    source_resources.connect_update_source_resource([&](const SourceResource& source_resource, Transaction& t){
        ++update_count;
    }, 0);

    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_significant);

    Transaction t2(db);
    resource_policies.set_enable_policy(source_1, download, EnablePolicy::DISABLE, t2);
    t2.commit();
    
    TEST_ASSERT(update_count == 1);
    TEST_ASSERT(!source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(!source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(!source_resources.get_source_resource(source_1, download).m_significant);
}

void disabled_selection_input_test()
{
    CLEAN_AND_INSTALL();

    auto download_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& download = report_types.add_report_type(std::make_unique<PlainTextReportType>(download_uuid, L""));

    auto selection_output_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& selection_output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(selection_output_uuid, L""));

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_download(*source_type_mockup, download, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));

    Transaction t1(db);
    selections.add_selection(std::make_unique<SelectionMockup>(SelectionDirection(download, selection_output_type), [](const std::vector<std::shared_ptr<Content>>&){
        return std::unique_ptr<Content>();
    }), t1);
    t1.commit();

    auto source_1 = *sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});

    TEST_ASSERT(source_resources.get_source_resource(source_1, selection_output_type).m_available);
    TEST_ASSERT(source_resources.get_source_resource(source_1, selection_output_type).m_visible);
    TEST_ASSERT(source_resources.get_source_resource(source_1, selection_output_type).m_significant);

    Transaction t2(db);
    resource_policies.set_enable_policy(source_1, download, EnablePolicy::DISABLE, t2);
    t2.commit();
    
    TEST_ASSERT(!source_resources.get_source_resource(source_1, selection_output_type).m_available);
    TEST_ASSERT(!source_resources.get_source_resource(source_1, selection_output_type).m_visible);
    TEST_ASSERT(!source_resources.get_source_resource(source_1, selection_output_type).m_significant);
}

void disabled_grouping_input_test()
{
    CLEAN_AND_INSTALL();

    auto download_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& download = report_types.add_report_type(std::make_unique<PlainTextReportType>(download_uuid, L""));

    auto grouping_output_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& grouping_output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(grouping_output_uuid, L""));

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_download(*source_type_mockup, download, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));

    Transaction t1(db);
    groupings.add_grouping(std::make_unique<GroupingMockup>(download, grouping_output_type, [](const std::vector<std::shared_ptr<Content>>&){
        return std::unique_ptr<Content>();
    }), t1);
    t1.commit();

    auto source_1 = *sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});

    TEST_ASSERT(source_resources.get_source_resource(source_1, grouping_output_type).m_available);
    TEST_ASSERT(source_resources.get_source_resource(source_1, grouping_output_type).m_visible);
    TEST_ASSERT(source_resources.get_source_resource(source_1, grouping_output_type).m_significant);

    TEST_ASSERT(source_resources.get_source_resource(root, grouping_output_type).m_available);
    TEST_ASSERT(source_resources.get_source_resource(root, grouping_output_type).m_visible);
    TEST_ASSERT(source_resources.get_source_resource(root, grouping_output_type).m_significant);

    Transaction t2(db);
    resource_policies.set_enable_policy(source_1, download, EnablePolicy::DISABLE, t2);
    t2.commit();

    TEST_ASSERT(source_resources.get_source_resource(source_1, grouping_output_type).m_available);
    TEST_ASSERT(source_resources.get_source_resource(source_1, grouping_output_type).m_visible);
    TEST_ASSERT(source_resources.get_source_resource(source_1, grouping_output_type).m_significant);

    TEST_ASSERT(source_resources.get_source_resource(root, grouping_output_type).m_available);
    TEST_ASSERT(source_resources.get_source_resource(root, grouping_output_type).m_visible);
    TEST_ASSERT(source_resources.get_source_resource(root, grouping_output_type).m_significant);
}

void report_visibility_test()
{
    CLEAN_AND_INSTALL();

    auto download_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& download = report_types.add_report_type(std::make_unique<PlainTextReportType>(download_uuid, L""));

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_download(*source_type_mockup, download, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));

    auto source_1 = *sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});

    int update_count = 0;
    source_resources.connect_update_source_resource([&](const SourceResource& source_resource, Transaction& t){
        ++update_count;
    }, 0);

    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_significant);

    Transaction t(db);
    resource_policies.set_visibility(source_1, download, Visibility::INVISIBLE, t);
    t.commit();

    TEST_ASSERT(update_count == 1);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(!source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(!source_resources.get_source_resource(source_1, download).m_significant);
}

void hidden_selection_input_test()
{
    CLEAN_AND_INSTALL();

    auto download_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& download = report_types.add_report_type(std::make_unique<PlainTextReportType>(download_uuid, L""));

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_download(*source_type_mockup, download, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));

    auto selection_output_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& selection_output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(selection_output_uuid, L""));

    Transaction t1(db);
    selections.add_selection(std::make_unique<SelectionMockup>(SelectionDirection(download, selection_output_type), [](const std::vector<std::shared_ptr<Content>>&){
        return std::unique_ptr<Content>();
    }), t1);
    t1.commit();

    auto source_1 = *sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});

    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_significant);

    Transaction t2(db);
    resource_policies.set_visibility(source_1, download, Visibility::INVISIBLE, t2);
    t2.commit();

    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(!source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_significant);

    Transaction t3(db);
    resource_policies.set_visibility(source_1, selection_output_type, Visibility::INVISIBLE, t3);
    t3.commit();

    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(!source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(!source_resources.get_source_resource(source_1, download).m_significant);
}

void hidden_grouping_input_test()
{
    CLEAN_AND_INSTALL();

    auto download_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& download = report_types.add_report_type(std::make_unique<PlainTextReportType>(download_uuid, L""));

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_download(*source_type_mockup, download, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));

    auto grouping_output_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& grouping_output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(grouping_output_uuid, L""));

    Transaction t1(db);
    groupings.add_grouping(std::make_unique<GroupingMockup>(download, grouping_output_type, [](const std::vector<std::shared_ptr<Content>>&){
        return std::unique_ptr<Content>();
    }), t1);
    t1.commit();

    auto source_1 = *sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});

    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_significant);

    Transaction t2(db);
    resource_policies.set_visibility(source_1, download, Visibility::INVISIBLE, t2);
    t2.commit();

    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(!source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_significant);

    Transaction t3(db);
    resource_policies.set_visibility(source_1, grouping_output_type, Visibility::INVISIBLE, t3);
    t3.commit();

    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(!source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_significant);

    TEST_ASSERT(source_resources.get_source_resource(source_1, grouping_output_type).m_available);
    TEST_ASSERT(!source_resources.get_source_resource(source_1, grouping_output_type).m_visible);
    TEST_ASSERT(!source_resources.get_source_resource(source_1, grouping_output_type).m_significant);

    TEST_ASSERT(source_resources.get_source_resource(root, grouping_output_type).m_available);
    TEST_ASSERT(source_resources.get_source_resource(root, grouping_output_type).m_visible);
    TEST_ASSERT(source_resources.get_source_resource(root, grouping_output_type).m_significant);

    Transaction t4(db);
    resource_policies.set_visibility(root, grouping_output_type, Visibility::INVISIBLE, t4);
    t4.commit();

    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(!source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(!source_resources.get_source_resource(source_1, download).m_significant);

    TEST_ASSERT(source_resources.get_source_resource(source_1, grouping_output_type).m_available);
    TEST_ASSERT(!source_resources.get_source_resource(source_1, grouping_output_type).m_visible);
    TEST_ASSERT(!source_resources.get_source_resource(source_1, grouping_output_type).m_significant);

    TEST_ASSERT(source_resources.get_source_resource(root, grouping_output_type).m_available);
    TEST_ASSERT(!source_resources.get_source_resource(root, grouping_output_type).m_visible);
    TEST_ASSERT(!source_resources.get_source_resource(root, grouping_output_type).m_significant);
}

void defer_report_policy_test()
{
    CLEAN_AND_INSTALL();

    auto download_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& download = report_types.add_report_type(std::make_unique<PlainTextReportType>(download_uuid, L""));

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_download(*source_type_mockup, download, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));

    auto source_1 = *sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});

    int update_count = 0;
    source_resources.connect_update_source_resource([&](const SourceResource& source_resource, Transaction& t){
        ++update_count;
    }, 0);

    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_significant);

    Transaction t(db);
    resource_policies.set_defer_policy(source_1, download, DeferPolicy::DEFER, t);
    t.commit();

    TEST_ASSERT(update_count == 1);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(!source_resources.get_source_resource(source_1, download).m_significant);
}

void deferred_selection_input_test()
{
    CLEAN_AND_INSTALL();

    auto download_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& download = report_types.add_report_type(std::make_unique<PlainTextReportType>(download_uuid, L""));

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_download(*source_type_mockup, download, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));

    auto selection_output_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& selection_output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(selection_output_uuid, L""));

    Transaction t1(db);
    selections.add_selection(std::make_unique<SelectionMockup>(SelectionDirection(download, selection_output_type), [](const std::vector<std::shared_ptr<Content>>&){
        return std::unique_ptr<Content>();
    }), t1);
    t1.commit();

    auto source_1 = *sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});

    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_significant);

    Transaction t2(db);
    resource_policies.set_defer_policy(source_1, download, DeferPolicy::DEFER, t2);
    t2.commit();

    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_significant);

    Transaction t3(db);
    resource_policies.set_defer_policy(source_1, selection_output_type, DeferPolicy::DEFER, t3);
    t3.commit();

    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(!source_resources.get_source_resource(source_1, download).m_significant);

    Transaction t4(db);
    resource_policies.set_defer_policy(source_1, selection_output_type, DeferPolicy::DEFAULT_POLICY, t4);
    t4.commit();

    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_significant);

    Transaction t5(db);
    resource_policies.set_visibility(source_1, selection_output_type, Visibility::INVISIBLE, t5);
    t5.commit();

    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(!source_resources.get_source_resource(source_1, download).m_significant);
}

void deferred_grouping_input_test()
{
    CLEAN_AND_INSTALL();

    auto download_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& download = report_types.add_report_type(std::make_unique<PlainTextReportType>(download_uuid, L""));

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_download(*source_type_mockup, download, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));

    auto grouping_output_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& grouping_output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(grouping_output_uuid, L""));

    Transaction t1(db);
    groupings.add_grouping(std::make_unique<GroupingMockup>(download, grouping_output_type, [](const std::vector<std::shared_ptr<Content>>&){
        return std::unique_ptr<Content>();
    }), t1);
    t1.commit();

    auto source_1 = *sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});

    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_significant);

    Transaction t2(db);
    resource_policies.set_defer_policy(source_1, download, DeferPolicy::DEFER, t2);
    t2.commit();

    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_significant);

    Transaction t3(db);
    resource_policies.set_defer_policy(source_1, grouping_output_type, DeferPolicy::DEFER, t3);
    t3.commit();

    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_significant);

    Transaction t4(db);
    resource_policies.set_defer_policy(root, grouping_output_type, DeferPolicy::DEFER, t3);
    t4.commit();

    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(!source_resources.get_source_resource(source_1, download).m_significant);

    Transaction t5(db);
    resource_policies.set_defer_policy(root, grouping_output_type, DeferPolicy::DEFAULT_POLICY, t5);
    t5.commit();

    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_significant);

    Transaction t6(db);
    resource_policies.set_visibility(root, grouping_output_type, Visibility::INVISIBLE, t6);
    t6.commit();

    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_available);
    TEST_ASSERT(source_resources.get_source_resource(source_1, download).m_visible);
    TEST_ASSERT(!source_resources.get_source_resource(source_1, download).m_significant);
}

} //namespace {

test_assist::test_suit get_source_resources_suit()
{
    test_assist::test_suit ts(L"source_reports_suit");
    ts
        .add({ L"add_loading_output_test", &add_loading_output_test })
        .add({ L"add_selection_test", &add_selection_test })
        .add({ L"add_grouping_test", &add_grouping_test })
        .add({ L"add_shortcut_test", &add_shortcut_test })
        .add({ L"enable_resource_policy_test", &enable_resource_policy_test })
        .add({ L"disabled_selection_input_test", &disabled_selection_input_test })
        .add({ L"disabled_grouping_input_test", &disabled_grouping_input_test })
        .add({ L"report_visibility_test", &report_visibility_test })
        .add({ L"hidden_selection_input_test", &hidden_selection_input_test })
        .add({ L"hidden_grouping_input_test", &hidden_grouping_input_test })
        .add({ L"defer_report_policy_test", &defer_report_policy_test })
        .add({ L"deferred_selection_input_test", &deferred_selection_input_test })
        .add({ L"deferred_grouping_input_test", &deferred_grouping_input_test });

    return ts;
}