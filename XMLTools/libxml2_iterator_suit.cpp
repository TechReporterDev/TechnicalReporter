#include "stdafx.h"
#include "libxml2_iterator_suit.h"
#include "xml_tools.h"
using namespace xml_tools;
namespace {

void sibling_iterator_test()
{
    std::string single_text_xml(
        "<?xml version=\"1.0\"?>"
        "<root>"
            "<element1>"
                "text1"
            "</element1>"
            "<element2>"
                "text2"
            "</element2>"
            "<element3>"
                "text3"
            "</element3>"
        "</root>");

    auto doc = parse(single_text_xml);
    auto root = get_root(*doc);
    auto element1 = begin_child(*root);

    sibling_node_iterator sni_1(element1);  
    sibling_node_iterator sni_2;
    TEST_ASSERT(sni_1 != sni_2);
    TEST_ASSERT(&*sni_1 == element1);

    auto element2 = next_child(*element1); 
    auto sni_prev = sni_1++;
    TEST_ASSERT(&*sni_prev == element1)
    TEST_ASSERT(&*sni_1 == element2);

    auto element3 = next_child(*element2);
    ++sni_1;
    TEST_ASSERT(&*sni_1 == element3);

    ++sni_1;
    TEST_ASSERT(sni_1 == sni_2);
}

} //namespace {

test_assist::test_suit get_libxml2_iterator_suit()
{
    test_assist::test_suit ts(L"libxml2_iterator_suit");
    ts
        .add({ L"sibling_iterator_test", &sibling_iterator_test });
    return ts;
}