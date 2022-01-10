#include "stdafx.h"
#include "XMLQuerySuit.h"
#include "XMLQuery.h"
#include "XmlTools\xml_tools.h"
using namespace TR::XML;
using namespace xml_tools;
namespace {

void query_trivial_action_test()
{
    std::string string_def(
        "<?xml version=\"1.0\"?>"
        "<struct class=\"STRUCT\">"
            "<trivial class=\"STRING\"/>"
        "</struct>");

    std::string string_doc(
        "<?xml version=\"1.0\"?>"
        "<struct>"
            "<trivial>"
                "value"
            "</trivial>"
        "</struct>");

    auto def_doc = parse_definition(string_def);
    auto reg_doc = parse_regular(string_doc, *def_doc);

    std::string reject_query_xml(
        "<?xml version=\"1.0\"?>"
        "<struct>"
            "<trivial action='REJECT'/>"        
        "</struct>");

    std::string rejected_pattern(
        "<?xml version=\"1.0\"?>"
        "<struct/>");


    auto reject_query_doc = parse_query(reject_query_xml, *def_doc);
    auto rejected_pattern_doc = parse(rejected_pattern);
    auto rejected_doc = do_query(*reject_query_doc, *reg_doc);
    TEST_ASSERT(as_string(*rejected_doc) == as_string(*rejected_pattern_doc));

    std::string accept_query_xml(
        "<?xml version=\"1.0\"?>"
        "<struct>"
            "<trivial action='ACCEPT'/>"
        "</struct>");

    std::string accepted_pattern(
        "<?xml version=\"1.0\"?>"
        "<struct>"
            "<trivial>"
                "value"
            "</trivial>"
        "</struct>");

    auto accept_query_doc = parse_query(accept_query_xml, *def_doc);
    auto accepted_pattern_doc = parse(accepted_pattern);
    auto accepted_doc = do_query(*accept_query_doc, *reg_doc);
    TEST_ASSERT(as_string(*accepted_doc) == as_string(*accepted_pattern_doc));
}

void query_trivial_condition_test()
{
    std::string string_def(
        "<?xml version=\"1.0\"?>"
        "<struct class=\"STRUCT\">"
            "<trivial class=\"STRING\"/>"
        "</struct>");

    std::string string_doc(
        "<?xml version=\"1.0\"?>"
        "<struct>"
            "<trivial>"
                "value"
            "</trivial>"
        "</struct>");

    
    auto def_doc = parse_definition(string_def);
    auto reg_doc = parse_regular(string_doc, *def_doc);

    // condition not found test
    std::string query1_xml(
        "<?xml version=\"1.0\"?>"
        "<struct>"
            "<trivial>"
                "<condition type='STRING' pattern='invalid'/>"
            "</trivial>"
        "</struct>");


    auto query1_doc = parse_query(query1_xml, *def_doc);
    auto queried1_doc = do_query(*query1_doc, *reg_doc);
    TEST_ASSERT(queried1_doc == nullptr);
    
    // condition accept test
    std::string query2_xml(
        "<?xml version=\"1.0\"?>"
        "<struct>"
            "<trivial>"
                "<condition action='ACCEPT'>"
                    "<pattern>"
                        "value"
                    "</pattern>"
                "</condition>"
            "</trivial>"
        "</struct>");

    std::string pattern2_xml(
        "<?xml version=\"1.0\"?>"
        "<struct>"
            "<trivial>"
                "value"
            "</trivial>"
        "</struct>");

    auto query2_doc = parse_query(query2_xml, *def_doc);
    auto pattern2_doc = parse(pattern2_xml);
    auto queried2_doc = do_query(*query2_doc, *reg_doc);
    TEST_ASSERT(as_string(*queried2_doc) == as_string(*pattern2_doc));

    // condition reject test
    std::string query3_xml(
        "<?xml version=\"1.0\"?>"
        "<struct>"
            "<trivial>"
                "<condition action='REJECT'>"
                    "<pattern>"
                        "value"
                    "</pattern>"
                "</condition>"
            "</trivial>"
        "</struct>");


    auto query3_doc = parse_query(query3_xml, *def_doc);
    auto queried3_doc = do_query(*query3_doc, *reg_doc);
    TEST_ASSERT(queried3_doc == nullptr);
}

void query_trivial_list_test()
{
    std::string string_def(
        "<?xml version=\"1.0\"?>"
        "<list class=\"LIST\">"
            "<trivial class=\"STRING\"/>"
        "</list>");

    std::string string_doc(
        "<?xml version=\"1.0\"?>"
        "<list>"
            "<trivial>"
                "value1"
            "</trivial>"
            "<trivial>"
                "value2"
            "</trivial>"
            "<trivial>"
                "value3"
            "</trivial>"
        "</list>");


    auto def_doc = parse_definition(string_def);
    auto reg_doc = parse_regular(string_doc, *def_doc);

    std::string query_xml(
        "<?xml version=\"1.0\"?>"
        "<list>"
            "<trivial>"
                "<condition>"
                    "<pattern>"
                        "value1"
                    "</pattern>"
                "</condition>"
                "<condition>"
                    "<pattern>"
                        "value3"
                    "</pattern>"
                "</condition>"
            "</trivial>"
        "</list>");

    std::string pattern_xml(
        "<?xml version=\"1.0\"?>"
        "<list>"
            "<trivial>"
                "value1"
            "</trivial>"
            "<trivial>"
                "value3"
            "</trivial>"
        "</list>");

    auto query_doc = parse_query(query_xml, *def_doc);
    auto pattern_doc = parse(pattern_xml);
    auto queried_doc = do_query(*query_doc, *reg_doc);
    TEST_ASSERT(as_string(*queried_doc) == as_string(*pattern_doc));
}

} //namespace {

test_assist::test_suit get_xml_query_suit()
{
    test_assist::test_suit ts(L"get_xml_query_suit");
    ts
        .add({L"query_trivial_action_test", &query_trivial_action_test})
        .add({L"query_trivial_condition_test", &query_trivial_condition_test})
        .add({L"query_trivial_list_test", &query_trivial_list_test});

    return ts;
}