#include "stdafx.h"
#include "XmlDiffSuit.h"
#include "XmlDiff.h"
using namespace TR::Core;
using namespace TR::XML;
namespace {

void xml_add_node_diff_test()
{
    std::string old_xml =
        "<?xml version=\"1.0\"?>"
        "<root>"
            "<prev>"
            "</prev>"
            "<next>"
            "</next>"   
        "</root>";
    
    std::string new_xml =
        "<?xml version=\"1.0\"?>"
        "<root>"
            "<prev>"
            "</prev>"
            "<added>"
                "<internal/>"
            "</added>"
            "<next>"
            "</next>"   
        "</root>";

    auto old_doc = xml_tools::parse(old_xml);
    auto new_doc = xml_tools::parse(new_xml);   
    auto diff_points = make_diff(*old_doc, *new_doc, "key");
    
    TEST_ASSERT(diff_points.size() == 1);
    TEST_ASSERT(diff_points[0].get_type() == XmlDiffPoint::ELEMENT);
    TEST_ASSERT(diff_points[0].m_state == XmlDiffPoint::STATE_INSERTED);
    TEST_ASSERT(diff_points[0].m_path == XmlDiffPoint::Path({{"root"}, {"added", 1}}));
}

void xml_remove_node_diff_test()
{
    std::string old_xml =
        "<?xml version=\"1.0\"?>"
        "<root>"
            "<prev>"
            "</prev>"
            "<removed>"
                "<internal/>"
            "</removed>"
            "<next>"
            "</next>"   
        "</root>";
    
    std::string new_xml =
        "<?xml version=\"1.0\"?>"
        "<root>"
            "<prev>"
            "</prev>"
            "<next>"
            "</next>"   
        "</root>";

    auto old_doc = xml_tools::parse(old_xml);
    auto new_doc = xml_tools::parse(new_xml);   
    auto diff_points = make_diff(*old_doc, *new_doc, "key");
    
    TEST_ASSERT(diff_points.size() == 1);
    TEST_ASSERT(diff_points[0].get_type() == XmlDiffPoint::ELEMENT);
    TEST_ASSERT(diff_points[0].m_state == XmlDiffPoint::STATE_REMOVED);
    TEST_ASSERT(diff_points[0].m_path == XmlDiffPoint::Path({{"root"}, {"removed", 1}}));
}

void xml_replace_node_diff_test()
{
    std::string old_xml =
        "<?xml version=\"1.0\"?>"
        "<root>"
            "<first/>"
            "<second/>"
            "<third/>"
        "</root>";
    
    std::string new_xml =
        "<?xml version=\"1.0\"?>"
        "<root>"
            "<third/>"
            "<second/>" 
            "<first/>"                  
        "</root>";

    auto old_doc = xml_tools::parse(old_xml);
    auto new_doc = xml_tools::parse(new_xml);   
    auto diff_points = make_diff(*old_doc, *new_doc, "key");    
    
    TEST_ASSERT(diff_points.size() == 0);   
}
} //namespace {

test_assist::test_suit get_xml_diff_suit()
{
    test_assist::test_suit ts(L"xml_diff_suit");
    ts
        .add({L"xml_add_node_diff_test", &xml_add_node_diff_test})
        .add({L"xml_remove_node_diff_test", &xml_remove_node_diff_test})
        .add({L"xml_replace_node_diff_test", &xml_replace_node_diff_test});

    return ts;
}