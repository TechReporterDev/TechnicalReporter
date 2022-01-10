#include "stdafx.h"
#include "libxml2_wrap_suit.h"
#include "xml_tools.h"
using namespace xml_tools;

namespace {

void building_test()
{   
    auto doc = create_doc();
    TEST_ASSERT(doc);

    auto& root = set_root(*doc, "root");
    TEST_ASSERT(&root == get_root(*doc));

    auto& element = add_element(root, "element");
    TEST_ASSERT(&element == begin_child(root));

    add_text_node(element, "text line 1");
    add_text_node(element, " ");
    add_text_node(element, "text line 2");
    TEST_ASSERT(node_text_equal(element, "text line 1 text line 2"));
}

void navigating_test()
{
    std::string test_xml(
        "<?xml version=\"1.0\"?>"
        "<root>"
            "<element>"
                "grandchild text node "
            "</element>"        
            "text node 1"
            "<element/>"
            "text node 2"
        "</root>");

    auto doc = parse(test_xml);
    TEST_ASSERT(doc);

    // root test
    auto root = get_root(*doc);
    TEST_ASSERT(root);

    // hirizontal enumeration
    auto element1 = begin_child(*root);
    TEST_ASSERT(element1);
    TEST_ASSERT(get_node_type(*element1) == XmlNodeType::ELEMENT);

    auto text1 = next_child(*element1);
    TEST_ASSERT(text1);
    TEST_ASSERT(get_node_type(*text1) == XmlNodeType::TEXT);

    auto element2 = next_child(*text1);
    TEST_ASSERT(element2);
    TEST_ASSERT(get_node_type(*element2) == XmlNodeType::ELEMENT);

    auto text2 = next_child(*element2);
    TEST_ASSERT(text2);
    TEST_ASSERT(get_node_type(*text2) == XmlNodeType::TEXT);

    auto null_node = next_child(*text2);
    TEST_ASSERT(! null_node);

    // vertical navigation
    auto child = begin_child(*root);
    TEST_ASSERT(child);
    TEST_ASSERT(get_parent(*child) == root);

    auto grandchild = begin_child(*child); 
    TEST_ASSERT(grandchild);
    TEST_ASSERT(get_parent(*grandchild) == child);
}

void text_read_test()
{
    std::string single_text_xml(
        "<?xml version=\"1.0\"?>"
        "<root>"
            "<element1>"
            "</element1>"
            "<element2>"
                "single text node"
            "</element2>"
            "<element3>"
                "first text node "
                "<splitter/>"
                "second text node"
            "</element3>"
        "</root>");

    auto doc = parse(single_text_xml);
    auto root = get_root(*doc);
    auto element1 = begin_child(*root);

    TEST_ASSERT(node_text_equal(*element1, ""));
    TEST_ASSERT(! node_text_equal(*element1, "blah blah blah"));
    TEST_ASSERT(get_node_text(*element1) == "");
    TEST_ASSERT(get_node_wtext(*element1) == L"");

    auto element2 = next_child(*element1);

    TEST_ASSERT(get_node_text(*element2) == "single text node");
    TEST_ASSERT(get_node_wtext(*element2) == L"single text node");
    TEST_ASSERT(node_text_equal(*element2, "single text node"));
    TEST_ASSERT(! node_text_equal(*element2, "blah blah blah"));    

    auto text_node = begin_child(*element2);
    TEST_ASSERT(node_content_equal(*text_node, "single text node"));
    TEST_ASSERT(! node_content_equal(*text_node, "blah blah blah"));

    auto element3 = next_child(*element2);

    TEST_ASSERT(get_node_text(*element3) == "first text node second text node");
    TEST_ASSERT(get_node_wtext(*element3) == L"first text node second text node");
    TEST_ASSERT(node_text_equal(*element3, "first text node second text node"));    
    TEST_ASSERT(! node_text_equal(*element3, "blah blah blah"));    
}

void attribute_read_test()
{
    std::string test_xml(
        "<?xml version=\"1.0\"?>"
        "<root>"
            "<element attr1=\"value1\" attr2=\"value2\">"           
            "</element>"            
        "</root>");

    auto doc = parse(test_xml);
    auto root = get_root(*doc); 
    TEST_ASSERT(! begin_attribute(*root)); //empty attribute list

    auto element = begin_child(*root);
    auto attr1 = begin_attribute(*element);
    TEST_ASSERT(attr1);
    TEST_ASSERT(get_attr_name(*attr1) == "attr1");
    TEST_ASSERT(get_attr_wname(*attr1) == L"attr1");
    TEST_ASSERT(get_attr_value(*attr1) == "value1");
    TEST_ASSERT(get_attr_wvalue(*attr1) == L"value1");
    TEST_ASSERT(attr_name_equal(*attr1, "attr1"));

    auto attr2 = next_attribute(*attr1);
    TEST_ASSERT(attr2);
    TEST_ASSERT(get_attr_name(*attr2) == "attr2");
    TEST_ASSERT(get_attr_wname(*attr2) == L"attr2");
    TEST_ASSERT(get_attr_value(*attr2) == "value2");
    TEST_ASSERT(get_attr_wvalue(*attr2) == L"value2");
    TEST_ASSERT(attr_name_equal(*attr2, "attr2"));

    auto attr3 = next_attribute(*attr2);
    TEST_ASSERT(! attr3);

    //find attribute
    auto found_attr1 = find_attribute(*element, "attr1");
    TEST_ASSERT(found_attr1 == attr1);

    auto found_attr2 = find_attribute(*element, "attr2");
    TEST_ASSERT(found_attr2 == attr2);

    auto found_attr3 = find_attribute(*element, "attr3");
    TEST_ASSERT(! found_attr3);
}

void apply_stylesheet_test()
{
    std::string xml_stylesheet =
        "<?xml version=\"1.0\"?>"
        "<xsl:stylesheet version=\"1.0\" xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\" xmlns:exsl=\"http://exslt.org/common\">"
        "<xsl:output method=\"xml\"/>"
            "<xsl:template match=\"*\">"
                "<xsl:copy>"
                    "<xsl:value-of select=\"name(..)\"/>"
                "</xsl:copy>"
            "</xsl:template>"
        "</xsl:stylesheet>";

    auto stylesheet = xslt_parse_stylesheet_doc(parse(xml_stylesheet));

    std::string xml_doc = 
        "<?xml version=\"1.0\"?>"
        "<root>"
            "<element>"
            "</element>"
            "<element2>"
            "</element2>"
        "</root>";

    auto doc = parse(xml_doc);
    //auto res1 = xslt_apply_stylesheet(*stylesheet, *doc);
    auto res2 = xslt_apply_stylesheet(*stylesheet, *(XmlDoc*)(begin_child(*get_root(*doc))));
    auto res3 = xslt_apply_stylesheet(*stylesheet, *(XmlDoc*)(next_child(begin_child(*get_root(*doc)))));
}

} //namespace {

test_assist::test_suit get_libxml2_wrap_suit()
{
    test_assist::test_suit ts(L"libxml2_wrap_suit");
    ts
        .add({L"building_test", &building_test})
        .add({L"navigating_test", &navigating_test})
        .add({L"text_read_test", &text_read_test})
        .add({L"attribute_read_test", &attribute_read_test})
        .add({ L"apply_stylesheet_test", &apply_stylesheet_test});
        
    return ts;
}