#include "stdafx.h"
#include "CollectorSuit.h"
#include "CoreTestAssist.h"
namespace {

void collect_report_test()
{
    CLEAN_AND_INSTALL();

    auto report_type_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& report_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(report_type_uuid, L""));
    auto source = sources.get_root();

    int update_report_count = 0;
    int content_changed_count = 0;
    collector.connect_update_current_report([&](CurrentReportRef current_report_ref, bool content_changed, Transaction& t){
        TEST_ASSERT(current_report_ref.get_source_ref() == source);
        TEST_ASSERT(current_report_ref.get_report_type_ref() == report_type);
        ++update_report_count;
        if (content_changed)
            ++content_changed_count;
    }, 0);

    TEST_ASSERT(collector.get_current_reports(source).empty());
    TEST_ASSERT(collector.find_current_report(source, report_type) == boost::none);
    TEST_EXCEPTION(collector.get_current_report(source, report_type));

    auto time_1 = time(nullptr);
    auto content_1 = std::make_shared<PlainTextContent>("12345", nullptr);
    collector.collect_report({source, report_type, content_1, time_1});
        
    TEST_ASSERT(update_report_count == 1);
    TEST_ASSERT(content_changed_count == 1);
    TEST_ASSERT(collector.get_current_reports(source).size() == 1);
    TEST_ASSERT(collector.find_current_report(source, report_type));
    TEST_NO_EXCEPTION(collector.get_current_report(source, report_type));
    
    auto current_report = collector.get_current_report(source, report_type);

    TEST_ASSERT(current_report.get_source_ref() == source);
    TEST_ASSERT(current_report.get_report_type_ref() == report_type);
    TEST_ASSERT(current_report.get_time() == time_1);
    TEST_ASSERT(current_report.get_check_time() == time_1);
    TEST_ASSERT(current_report.get_diff() == nullptr);
    TEST_ASSERT(current_report.get_content()->is_equal(*content_1));
    TEST_ASSERT(current_report.get_uuid() != stl_tools::null_uuid());
}

void update_report_test()
{
    CLEAN_AND_INSTALL();

    auto report_type_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& report_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(report_type_uuid, L""));
    auto source = sources.get_root();

    auto time_1 = time(nullptr);
    auto content_1 = std::make_shared<PlainTextContent>("12345", nullptr);
    collector.collect_report({source, report_type, content_1, time_1});

    int update_report_count = 0;
    int content_changed_count = 0;
    collector.connect_update_current_report([&](CurrentReportRef current_report_ref, bool content_changed, Transaction& t){
        TEST_ASSERT(current_report_ref.get_source_ref() == source);
        TEST_ASSERT(current_report_ref.get_report_type_ref() == report_type);
        ++update_report_count;
        if (content_changed)
            ++content_changed_count;
    }, 0);

    TEST_ASSERT(collector.get_current_reports(source).size() == 1);
    TEST_ASSERT(collector.find_current_report(source, report_type));
    TEST_NO_EXCEPTION(collector.get_current_report(source, report_type));

    auto time_2 = time_1 + 1;
    auto content_2 = std::make_shared<PlainTextContent>("ABCDE", nullptr);
    auto diff = content_1->make_diff(*content_2);
    collector.collect_report({source, report_type, content_2, time_2});
        
    TEST_ASSERT(update_report_count == 1);
    TEST_ASSERT(content_changed_count == 1);
    TEST_ASSERT(collector.get_current_reports(source).size() == 1);

    auto current_report = collector.get_current_report(source, report_type);

    TEST_ASSERT(current_report.get_source_ref() == source);
    TEST_ASSERT(current_report.get_report_type_ref() == report_type);
    TEST_ASSERT(current_report.get_time() == time_2);
    TEST_ASSERT(current_report.get_check_time() == time_2);
    TEST_ASSERT(current_report.get_diff() != nullptr);
    TEST_ASSERT(current_report.get_diff()->as_blob() == diff->as_blob());
    TEST_ASSERT(current_report.get_content()->is_equal(*content_2));    
    TEST_ASSERT(current_report.get_uuid() != stl_tools::null_uuid());
}

void refresh_report_test()
{
    CLEAN_AND_INSTALL();

    auto report_type_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& report_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(report_type_uuid, L""));
    auto source = sources.get_root();

    auto time_1 = time(nullptr);
    auto content_1 = std::make_shared<PlainTextContent>("12345", nullptr);
    collector.collect_report({source, report_type, content_1, time_1});

    int update_report_count = 0;
    int content_changed_count = 0;
    collector.connect_update_current_report([&](CurrentReportRef current_report_ref, bool content_changed, Transaction& t){
        TEST_ASSERT(current_report_ref.get_source_ref() == source);
        TEST_ASSERT(current_report_ref.get_report_type_ref() == report_type);
        ++update_report_count;
        if (content_changed)
            ++content_changed_count;
    }, 0);

    TEST_ASSERT(collector.get_current_reports(source).size() == 1);
    TEST_ASSERT(collector.find_current_report(source, report_type));
    TEST_NO_EXCEPTION(collector.get_current_report(source, report_type));

    auto time_2 = time_1 + 1;
    auto content_2 = std::make_shared<PlainTextContent>("12345", nullptr);
    collector.collect_report({source, report_type, content_2, time_2});
        
    TEST_ASSERT(update_report_count == 1);
    TEST_ASSERT(content_changed_count == 0);
    TEST_ASSERT(collector.get_current_reports(source).size() == 1);

    auto current_report = collector.get_current_report(source, report_type);

    TEST_ASSERT(current_report.get_source_ref() == source);
    TEST_ASSERT(current_report.get_report_type_ref() == report_type);
    TEST_ASSERT(current_report.get_time() == time_1);
    TEST_ASSERT(current_report.get_check_time() == time_2);
    TEST_ASSERT(current_report.get_diff() == nullptr);
    TEST_ASSERT(current_report.get_content()->is_equal(*content_2));
    TEST_ASSERT(current_report.get_uuid() != stl_tools::null_uuid());
}

void remove_source_test()
{
    CLEAN_AND_INSTALL();

    auto& source_type = source_types.add_source_type(std::make_unique<SourceTypeMockup>());
    auto source = sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});

    auto report_type_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& report_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(report_type_uuid, L""));

    collector.collect_report({source, report_type, std::make_shared<PlainTextContent>("", nullptr), time(nullptr)});

    TEST_ASSERT(collector.get_current_reports(source).size() == 1);
    TEST_ASSERT(collector.find_current_report(source, report_type));

    sources.remove_source(source.get_key());

    TEST_ASSERT(collector.get_current_reports(source).empty());
    TEST_ASSERT(!collector.find_current_report(source, report_type));
}

void remove_report_type_test()
{
    CLEAN_AND_INSTALL();

    auto& source_type = source_types.add_source_type(std::make_unique<SourceTypeMockup>());
    auto source = sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});

    auto report_type_uuid = ReportTypeUUID(stl_tools::gen_uuid());
    auto& report_type = report_types.add_report_type(std::make_unique<PlainTextReportType>(report_type_uuid, L""));

    collector.collect_report({source, report_type, std::make_shared<PlainTextContent>("", nullptr), time(nullptr)});

    TEST_ASSERT(collector.get_current_reports(source).size() == 1);
    TEST_ASSERT(collector.find_current_report(source, report_type));

    report_types.remove_report_type(report_type_uuid);

    TEST_ASSERT(collector.get_current_reports(source).empty());
    TEST_ASSERT(!collector.find_current_report(source, report_type));
}

} //namespace {

test_assist::test_suit get_collector_suit()
{
    test_assist::test_suit ts(L"collector_suit");
    ts
        .add({ L"collect_report_test", &collect_report_test })
        .add({ L"update_report_test", &update_report_test })
        .add({ L"refresh_report_test", &refresh_report_test })
        .add({ L"remove_source_test", &remove_source_test })
        .add({ L"remove_report_type_test", &remove_report_type_test });

    return ts;
}