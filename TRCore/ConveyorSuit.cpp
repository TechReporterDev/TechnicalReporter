#include "stdafx.h"
#include "ConveyorSuit.h"
#include "CoreTestAssist.h"
namespace {
auto INPUT_TIME = 1000;
auto MAX_TIME = INPUT_TIME + 1;

auto INPUT_CONTENT = PlainTextContent("111", nullptr);
auto OUTPUT_CONTENT = PlainTextContent("abc", nullptr);
auto INPUT_CONTENT_1 = PlainTextContent("123", nullptr);
auto INPUT_CONTENT_2 = PlainTextContent("456", nullptr);
auto INPUT_CONTENT_3 = PlainTextContent("789", nullptr);
auto OUTPUT_CONTENT_1 = PlainTextContent("abc1", nullptr);
auto OUTPUT_CONTENT_2 = PlainTextContent("abc2", nullptr);
auto OUTPUT_CONTENT_3 = PlainTextContent("abc3", nullptr);
auto ROOT_OUTPUT_CONTENT = PlainTextContent("root output", nullptr);
auto SOURCE_OUTPUT_CONTENT = PlainTextContent("source output", nullptr);

auto INPUT_UUID = ReportTypeUUID(stl_tools::gen_uuid());
auto OUTPUT_UUID = ReportTypeUUID(stl_tools::gen_uuid());
auto INPUT_UUID_1 = ReportTypeUUID(stl_tools::gen_uuid());
auto INPUT_UUID_2 = ReportTypeUUID(stl_tools::gen_uuid());
auto INPUT_UUID_3 = ReportTypeUUID(stl_tools::gen_uuid());
auto SOURCE_TYPE_UUID = SourceTypeUUID(stl_tools::gen_uuid());

void add_strict_selection_test()
{
    CLEAN_AND_INSTALL();

    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(INPUT_UUID, L""));
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(OUTPUT_UUID, L""));  
    
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(SOURCE_TYPE_UUID);
    push_download(*source_type_mockup, input_type, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));
    auto source = sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});
    
    TEST_ASSERT(!collector.find_current_report(source, output_type));

    Transaction t1(db);
    selections.add_selection(std::make_unique<SelectionMockup>(SelectionDirection(input_type, output_type), [](const std::vector<std::shared_ptr<Content>>& input){
        TEST_ASSERT(false);
        return nullptr;
    }), t1);
    t1.commit();

    TEST_ASSERT(!collector.find_current_report(source, output_type));
}

void add_strict_selection_input_ready_test()
{
    CLEAN_AND_INSTALL();

    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(INPUT_UUID, L""));
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(OUTPUT_UUID, L""));  

    auto source_type_mockup = std::make_unique<SourceTypeMockup>(SOURCE_TYPE_UUID);
    push_download(*source_type_mockup, input_type, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));
    auto source = sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});
    
    collector.collect_report({source, input_type, std::make_unique<PlainTextContent>(INPUT_CONTENT), INPUT_TIME});
    auto input = collector.get_current_report(source, input_type);
    TEST_ASSERT(!collector.find_current_report(source, output_type));

    Transaction t1(db);
    selections.add_selection(std::make_unique<SelectionMockup>(SelectionDirection(input_type, output_type), [](const std::vector<std::shared_ptr<Content>>& input){
        TEST_ASSERT(input.size() == 1);
        TEST_ASSERT(input[0]->is_equal(INPUT_CONTENT));
        return std::make_unique<PlainTextContent>(OUTPUT_CONTENT);
    }), t1);
    t1.commit();

    TEST_ASSERT(collector.find_current_report(source, output_type));
    auto output = collector.get_current_report(source, output_type);

    TEST_ASSERT(output.get_time() == INPUT_TIME);
    TEST_ASSERT(output.get_content()->is_equal(OUTPUT_CONTENT));
    TEST_ASSERT(output.get_transformation_circumstances().m_input_uuids == std::vector<TR::Core::UUID>{input.get_uuid()});
}

void add_not_strict_selection_test()
{
    CLEAN_AND_INSTALL();

    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(INPUT_UUID, L""));
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(OUTPUT_UUID, L""));  

    auto source_type_uuid = stl_tools::gen_uuid();
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(SOURCE_TYPE_UUID);
    push_download(*source_type_mockup, input_type, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));
    auto source = sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});

    TEST_ASSERT(!collector.find_current_report(source, output_type));

    Transaction t1(db);
    auto selection_plan = SelectionPlan({ {input_type, SelectionInput::Mode::OPTIONAL_MODE} }, output_type);
    selections.add_selection(std::make_unique<SelectionMockup>(selection_plan, [](const std::vector<std::shared_ptr<Content>>& input) {
        TEST_ASSERT(input.size() == 1);
        TEST_ASSERT(input[0] == nullptr);
        return std::make_unique<PlainTextContent>(OUTPUT_CONTENT);
    }), t1);
    t1.commit();

    TEST_ASSERT(collector.find_current_report(source, output_type));
    auto output = collector.get_current_report(source, output_type);

    TEST_ASSERT(output.get_time() == 0);
    TEST_ASSERT(output.get_content()->is_equal(OUTPUT_CONTENT));
}

void add_not_strict_selection_input_ready_test()
{
    CLEAN_AND_INSTALL();

    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(INPUT_UUID, L""));
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(OUTPUT_UUID, L""));  

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_download(*source_type_mockup, input_type, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));
    auto source = sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});

    collector.collect_report({source, input_type, std::make_unique<PlainTextContent>(INPUT_CONTENT), INPUT_TIME});
    TEST_ASSERT(!collector.find_current_report(source, output_type));
    
    Transaction t1(db);
    auto selection_plan = SelectionPlan({ {input_type, SelectionInput::Mode::OPTIONAL_MODE} }, output_type);
    selections.add_selection(std::make_unique<SelectionMockup>(selection_plan, [](const std::vector<std::shared_ptr<Content>>& input) -> std::unique_ptr<Content> {
        TEST_ASSERT(input.size() == 1);
        TEST_ASSERT(input[0] && input[0]->is_equal(INPUT_CONTENT));
        return std::make_unique<PlainTextContent>(OUTPUT_CONTENT);        
    }), t1);
    t1.commit();

    TEST_ASSERT(collector.find_current_report(source, output_type));
    auto output = collector.get_current_report(source, output_type);

    TEST_ASSERT(output.get_time() == INPUT_TIME);
    TEST_ASSERT(output.get_content()->is_equal(OUTPUT_CONTENT));
}

void add_grouping_test()
{
    CLEAN_AND_INSTALL();

    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(INPUT_UUID, L""));
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(OUTPUT_UUID, L""));  

    auto source_type_mockup = std::make_unique<SourceTypeMockup>(SOURCE_TYPE_UUID);
    push_download(*source_type_mockup, input_type, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));
    auto source = sources.add_source({L"source", root.get_ref(), default_role, source_type, source_type.get_default_config()});
    
    TEST_ASSERT(!collector.find_current_report(root, output_type));
    TEST_ASSERT(!collector.find_current_report(source, output_type));

    auto grouping_mockup = std::make_unique<GroupingMockup>(input_type, output_type);
    grouping_mockup->add_functor(root, {source}, [](const std::vector<std::shared_ptr<Content>>& input){
        TEST_ASSERT(input.size() == 1);
        TEST_ASSERT(input[0] == nullptr);
        return std::make_unique<PlainTextContent>(ROOT_OUTPUT_CONTENT);     
    });

    grouping_mockup->add_functor(source, {}, [](const std::vector<std::shared_ptr<Content>>& input){
        TEST_ASSERT(input.empty());
        return std::make_unique<PlainTextContent>(SOURCE_OUTPUT_CONTENT);       
    });

    Transaction t1(db);
    groupings.add_grouping(std::move(grouping_mockup), t1);
    t1.commit();

    TEST_ASSERT(collector.find_current_report(root, output_type));
    auto root_output = collector.get_current_report(root, output_type);
    TEST_ASSERT(root_output.get_time() == 0);
    TEST_ASSERT(root_output.get_content()->is_equal(ROOT_OUTPUT_CONTENT));

    TEST_ASSERT(collector.find_current_report(source, output_type));
    auto source_output = collector.get_current_report(source, output_type);
    TEST_ASSERT(source_output.get_time() == 0);
    TEST_ASSERT(source_output.get_content()->is_equal(SOURCE_OUTPUT_CONTENT));
}

void add_grouping_input_ready_test()
{
    CLEAN_AND_INSTALL();

    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(INPUT_UUID, L""));
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(OUTPUT_UUID, L""));  

    auto source_type_mockup = std::make_unique<SourceTypeMockup>(SOURCE_TYPE_UUID);
    push_download(*source_type_mockup, input_type, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));

    auto source_1 = sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});
    auto source_2 = sources.add_source({L"source_2", root.get_ref(), default_role, source_type, source_type.get_default_config()});
    
    collector.collect_report({source_1, input_type, std::make_unique<PlainTextContent>(INPUT_CONTENT_1), INPUT_TIME});
    collector.collect_report({source_2, input_type, std::make_unique<PlainTextContent>(INPUT_CONTENT_2), MAX_TIME});
    
    TEST_ASSERT(!collector.find_current_report(root, output_type));
    TEST_ASSERT(!collector.find_current_report(source_1, output_type));
    TEST_ASSERT(!collector.find_current_report(source_2, output_type));

    auto grouping_mockup = std::make_unique<GroupingMockup>(input_type, output_type, [](const std::vector<std::shared_ptr<Content>>& input){
        TEST_ASSERT(false);
        return nullptr;
    });
    
    grouping_mockup->add_functor(root, {source_1, source_2}, [](const std::vector<std::shared_ptr<Content>>& input){
        TEST_ASSERT(input.size() == 2);
        TEST_ASSERT(input[0]->is_equal(INPUT_CONTENT_1));
        TEST_ASSERT(input[1]->is_equal(INPUT_CONTENT_2));
        return std::make_unique<PlainTextContent>(ROOT_OUTPUT_CONTENT);     
    });

    grouping_mockup->add_functor(source_1, {}, [](const std::vector<std::shared_ptr<Content>>& input){
        TEST_ASSERT(input.empty());
        return std::make_unique<PlainTextContent>(SOURCE_OUTPUT_CONTENT);       
    });

    grouping_mockup->add_functor(source_2, {}, [](const std::vector<std::shared_ptr<Content>>& input){
        TEST_ASSERT(input.empty());
        return std::make_unique<PlainTextContent>(SOURCE_OUTPUT_CONTENT);       
    });

    Transaction t1(db);
    groupings.add_grouping(std::move(grouping_mockup), t1);
    t1.commit();

    TEST_ASSERT(collector.find_current_report(root, output_type));
    auto root_output = collector.get_current_report(root, output_type);
    TEST_ASSERT(root_output.get_time() == MAX_TIME);
    TEST_ASSERT(root_output.get_content()->is_equal(ROOT_OUTPUT_CONTENT));

    TEST_ASSERT(collector.find_current_report(source_1, output_type));
    auto source_1_output = collector.get_current_report(source_1, output_type);
    TEST_ASSERT(source_1_output.get_time() == 0);
    TEST_ASSERT(source_1_output.get_content()->is_equal(SOURCE_OUTPUT_CONTENT));

    TEST_ASSERT(collector.find_current_report(source_2, output_type));
    auto source_2_output = collector.get_current_report(source_1, output_type);
    TEST_ASSERT(source_2_output.get_time() == 0);
    TEST_ASSERT(source_2_output.get_content()->is_equal(SOURCE_OUTPUT_CONTENT));
}

void collect_strict_selection_input_test()
{
    CLEAN_AND_INSTALL();

    auto& input_type_1 = report_types.add_report_type(std::make_unique<PlainTextReportType>(INPUT_UUID_1, L""));
    auto& input_type_2 = report_types.add_report_type(std::make_unique<PlainTextReportType>(INPUT_UUID_2, L""));
    auto& input_type_3 = report_types.add_report_type(std::make_unique<PlainTextReportType>(INPUT_UUID_3, L""));
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(OUTPUT_UUID, L""));  

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_download(*source_type_mockup, input_type_1, "");
    push_download(*source_type_mockup, input_type_2, "");
    push_download(*source_type_mockup, input_type_3, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));

    Transaction t1(db);
    selections.add_selection(std::make_unique<SelectionMockup>(SelectionDirection({input_type_1, input_type_2, input_type_3}, output_type), [](const std::vector<std::shared_ptr<Content>>& input){
        TEST_ASSERT(input.size() == 3);
        TEST_ASSERT(input[0]->is_equal(INPUT_CONTENT_1));
        TEST_ASSERT(input[1]->is_equal(INPUT_CONTENT_2));
        TEST_ASSERT(input[2]->is_equal(INPUT_CONTENT_3));
        return std::make_unique<PlainTextContent>(OUTPUT_CONTENT);
    }), t1);
    t1.commit();

    auto source = sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});

    TEST_ASSERT(!collector.find_current_report(source, output_type));   
    collector.collect_report({source, input_type_1, std::make_unique<PlainTextContent>(INPUT_CONTENT_1), INPUT_TIME});
    TEST_ASSERT(!collector.find_current_report(source, output_type));

    collector.collect_report({source, input_type_2, std::make_unique<PlainTextContent>(INPUT_CONTENT_2), MAX_TIME});
    TEST_ASSERT(!collector.find_current_report(source, output_type));

    collector.collect_report({source, input_type_3, std::make_unique<PlainTextContent>(INPUT_CONTENT_3), INPUT_TIME});
    TEST_ASSERT(collector.find_current_report(source, output_type));
    auto output = collector.get_current_report(source, output_type);    
    TEST_ASSERT(output.get_content()->is_equal(OUTPUT_CONTENT));
    TEST_ASSERT(output.get_time() == MAX_TIME);
}

void collect_not_strict_selection_input_test()
{
    CLEAN_AND_INSTALL();

    auto& input_type_1 = report_types.add_report_type(std::make_unique<PlainTextReportType>(INPUT_UUID_1, L""));
    auto& input_type_2 = report_types.add_report_type(std::make_unique<PlainTextReportType>(INPUT_UUID_2, L""));
    auto& input_type_3 = report_types.add_report_type(std::make_unique<PlainTextReportType>(INPUT_UUID_3, L""));
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(OUTPUT_UUID, L""));  

    auto source_type_uuid = SourceTypeUUID(stl_tools::gen_uuid());
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(source_type_uuid);
    push_download(*source_type_mockup, input_type_1, "");
    push_download(*source_type_mockup, input_type_2, "");
    push_download(*source_type_mockup, input_type_3, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));

    Transaction t1(db);

    auto selection_plan = SelectionPlan({ 
        { input_type_1, SelectionInput::Mode::OPTIONAL_MODE },
        { input_type_2, SelectionInput::Mode::OPTIONAL_MODE },
        { input_type_3, SelectionInput::Mode::OPTIONAL_MODE } },
        output_type);

    selections.add_selection(std::make_unique<SelectionMockup>(selection_plan, [](const std::vector<std::shared_ptr<Content>>& input){
        TEST_ASSERT(input.size() == 3);
        
        auto output_content = OUTPUT_CONTENT;
        if (input[0])
        {
            TEST_ASSERT(input[0]->is_equal(INPUT_CONTENT_1));
            output_content = OUTPUT_CONTENT_1;
        }

        if (input[1])
        {
            TEST_ASSERT(input[1]->is_equal(INPUT_CONTENT_2));
            output_content = OUTPUT_CONTENT_2;
        }

        if (input[2])
        {
            TEST_ASSERT(input[2]->is_equal(INPUT_CONTENT_3));
            output_content = OUTPUT_CONTENT_3;
        }
    
        return std::make_unique<PlainTextContent>(output_content);
    }), t1);
    t1.commit();

    auto source = sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});
    
    TEST_ASSERT(collector.find_current_report(source, output_type));    
    auto output = collector.get_current_report(source, output_type);
    TEST_ASSERT(output.get_content()->is_equal(OUTPUT_CONTENT));
    TEST_ASSERT(output.get_time() == 0);

    collector.collect_report({source, input_type_1, std::make_unique<PlainTextContent>(INPUT_CONTENT_1), INPUT_TIME});
    auto output_1 = collector.get_current_report(source, output_type);
    TEST_ASSERT(output_1.get_content()->is_equal(OUTPUT_CONTENT_1));
    TEST_ASSERT(output_1.get_time() == INPUT_TIME);

    collector.collect_report({source, input_type_2, std::make_unique<PlainTextContent>(INPUT_CONTENT_2), MAX_TIME});
    auto output_2 = collector.get_current_report(source, output_type);
    TEST_ASSERT(output_2.get_content()->is_equal(OUTPUT_CONTENT_2));
    TEST_ASSERT(output_2.get_time() == MAX_TIME);

    collector.collect_report({source, input_type_3, std::make_unique<PlainTextContent>(INPUT_CONTENT_3), INPUT_TIME});
    auto output_3 = collector.get_current_report(source, output_type);
    TEST_ASSERT(output_3.get_content()->is_equal(OUTPUT_CONTENT_3));
    TEST_ASSERT(output_3.get_time() == MAX_TIME);
}

void collect_grouping_input_test()
{
    CLEAN_AND_INSTALL();

    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(INPUT_UUID, L""));
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(OUTPUT_UUID, L""));  

    auto source_type_mockup = std::make_unique<SourceTypeMockup>(SOURCE_TYPE_UUID);
    push_download(*source_type_mockup, input_type, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));
    auto source_1 = sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});
    auto source_2 = sources.add_source({L"source_2", root.get_ref(), default_role, source_type, source_type.get_default_config()});
    
    Transaction t1(db);
    auto grouping_mockup = std::make_shared<GroupingMockup>(input_type, output_type, [](const std::vector<std::shared_ptr<Content>>& input){
        return nullptr;
    });

    grouping_mockup->add_functor(root, {source_1, source_2}, [](const std::vector<std::shared_ptr<Content>>& input){
        TEST_ASSERT(input.size() == 2);
        TEST_ASSERT(input[0] == nullptr);
        TEST_ASSERT(input[1] == nullptr);
        return std::make_unique<PlainTextContent>(OUTPUT_CONTENT);      
    });

    groupings.add_grouping(grouping_mockup, t1);
    t1.commit();

    TEST_ASSERT(collector.find_current_report(root, output_type));
    auto root_output = collector.get_current_report(root, output_type);
    TEST_ASSERT(root_output.get_time() == 0);
    TEST_ASSERT(root_output.get_content()->is_equal(OUTPUT_CONTENT));

    grouping_mockup->add_functor(root, {source_1, source_2}, [](const std::vector<std::shared_ptr<Content>>& input){
        TEST_ASSERT(input.size() == 2);
        TEST_ASSERT(input[0]->is_equal(INPUT_CONTENT_1));
        TEST_ASSERT(input[1] == nullptr);
        return std::make_unique<PlainTextContent>(OUTPUT_CONTENT_1);        
    });

    collector.collect_report({source_1, input_type, std::make_unique<PlainTextContent>(INPUT_CONTENT_1), MAX_TIME});
    
    auto root_output_1 = collector.get_current_report(root, output_type);
    TEST_ASSERT(root_output_1.get_time() == MAX_TIME);
    TEST_ASSERT(root_output_1.get_content()->is_equal(OUTPUT_CONTENT_1));

    grouping_mockup->add_functor(root, {source_1, source_2}, [](const std::vector<std::shared_ptr<Content>>& input){
        TEST_ASSERT(input.size() == 2);
        TEST_ASSERT(input[0]->is_equal(INPUT_CONTENT_1));
        TEST_ASSERT(input[1]->is_equal(INPUT_CONTENT_2));
        return std::make_unique<PlainTextContent>(OUTPUT_CONTENT_2);        
    });

    collector.collect_report({source_2, input_type, std::make_unique<PlainTextContent>(INPUT_CONTENT_2), INPUT_TIME});

    auto root_output_2 = collector.get_current_report(root, output_type);
    TEST_ASSERT(root_output_2.get_time() == MAX_TIME);
    TEST_ASSERT(root_output_2.get_content()->is_equal(OUTPUT_CONTENT_2));
}

void source_moving_test()
{
    CLEAN_AND_INSTALL();

    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(INPUT_UUID, L""));
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(OUTPUT_UUID, L""));  

    auto source_type_mockup = std::make_unique<SourceTypeMockup>(SOURCE_TYPE_UUID);
    push_download(*source_type_mockup, input_type, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));
    auto group = sources.add_source({L"group", root.get_ref(), default_role, group_source_type, group_source_type.get_default_config()});
    auto source = *sources.add_source({L"source", root.get_ref(), default_role, source_type, source_type.get_default_config()});
    collector.collect_report({ source, input_type, std::make_unique<PlainTextContent>(INPUT_CONTENT), INPUT_TIME });
        
    Transaction t1(db);
    auto grouping_mockup = std::make_shared<GroupingMockup>(input_type, output_type, [](const std::vector<std::shared_ptr<Content>>& input){
        return nullptr;
    });

    grouping_mockup->add_functor(root, {source}, [](const std::vector<std::shared_ptr<Content>>& input){
        TEST_ASSERT(input.size() == 1);
        return std::make_unique<PlainTextContent>(OUTPUT_CONTENT_2);        
    });

    grouping_mockup->add_functor(root, {}, [](const std::vector<std::shared_ptr<Content>>& input){
        TEST_ASSERT(input.empty());
        return std::make_unique<PlainTextContent>(OUTPUT_CONTENT_1);        
    });

    grouping_mockup->add_functor(group, {}, [](const std::vector<std::shared_ptr<Content>>& input){
        TEST_ASSERT(input.empty());
        return std::make_unique<PlainTextContent>(OUTPUT_CONTENT_1);        
    });

    grouping_mockup->add_functor(group, {source}, [](const std::vector<std::shared_ptr<Content>>& input){
        TEST_ASSERT(input.size() == 1);
        return std::make_unique<PlainTextContent>(OUTPUT_CONTENT_2);        
    });

    groupings.add_grouping(grouping_mockup, t1);
    t1.commit();

    TEST_ASSERT(collector.find_current_report(root, output_type));
    auto root_output_1 = collector.get_current_report(root, output_type);
    TEST_ASSERT(root_output_1.get_content()->is_equal(OUTPUT_CONTENT_2));

    TEST_ASSERT(collector.find_current_report(group, output_type));
    auto group_output_1 = collector.get_current_report(group, output_type);
    TEST_ASSERT(group_output_1.get_content()->is_equal(OUTPUT_CONTENT_1));

    source.set_parent(group);
    sources.update_source(source);

    auto root_output_2 = collector.get_current_report(root, output_type);
    TEST_ASSERT(root_output_2.get_content()->is_equal(OUTPUT_CONTENT_1));

    auto group_output_2 = collector.get_current_report(group, output_type);
    TEST_ASSERT(group_output_2.get_content()->is_equal(OUTPUT_CONTENT_2));
}

void skip_unsignificant_output_test()
{
    CLEAN_AND_INSTALL();

    auto& input_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(INPUT_UUID, L""));
    auto& output_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(OUTPUT_UUID, L""));  
    auto source_type_mockup = std::make_unique<SourceTypeMockup>(SOURCE_TYPE_UUID);
    push_download(*source_type_mockup, input_type, "");
    auto& source_type = source_types.add_source_type(std::move(source_type_mockup));

    Transaction t1(db);
    selections.add_selection(std::make_unique<SelectionMockup>(SelectionDirection(input_type, output_type), [](const std::vector<std::shared_ptr<Content>>& input){
        TEST_ASSERT(input.size() == 1);
        TEST_ASSERT(input[0]->is_equal(INPUT_CONTENT));
        return std::make_unique<PlainTextContent>(OUTPUT_CONTENT);
    }), t1);
    t1.commit();

    auto source = sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});
    
    Transaction t2(db);
    resource_policies.set_defer_policy(source, output_type, DeferPolicy::DEFER, t2);
    t2.commit();

    TEST_ASSERT(!source_resources.get_source_resource(source, output_type).m_significant);  
    TEST_ASSERT(!collector.find_current_report(source, output_type));

    collector.collect_report({source, input_type, std::make_unique<PlainTextContent>(INPUT_CONTENT), INPUT_TIME});
    TEST_ASSERT(!collector.find_current_report(source, output_type));

    Transaction t3(db);
    resource_policies.set_defer_policy(source, output_type, DeferPolicy::DEFAULT_POLICY, t3);
    t3.commit();

    TEST_ASSERT(source_resources.get_source_resource(source, output_type).m_significant);
    TEST_ASSERT(collector.find_current_report(source, output_type));
    auto output = collector.get_current_report(source, output_type);
    TEST_ASSERT(output.get_time() == INPUT_TIME);
    TEST_ASSERT(output.get_content()->is_equal(OUTPUT_CONTENT));
}

} //namespace {

test_assist::test_suit get_conveyor_suit()
{
    test_assist::test_suit ts(L"conveyor_suit");
    ts
        .add({ L"add_strict_selection_test", &add_strict_selection_test })
        .add({ L"add_strict_selection_input_ready_test", &add_strict_selection_input_ready_test })
        .add({ L"add_not_strict_selection_test", &add_not_strict_selection_test })
        .add({ L"add_not_strict_selection_input_ready_test", &add_not_strict_selection_input_ready_test })
        .add({ L"add_grouping_test", &add_grouping_test })
        .add({ L"add_grouping_input_ready_test", &add_grouping_input_ready_test })
        .add({ L"collect_strict_selection_input_test", &collect_strict_selection_input_test })
        .add({ L"collect_not_strict_selection_input_test", &collect_not_strict_selection_input_test })
        .add({ L"collect_grouping_input_test", &collect_grouping_input_test })
        .add({ L"source_moving_test", &source_moving_test })        
        .add({ L"skip_unsignificant_output_test", &skip_unsignificant_output_test });

    return ts;
}