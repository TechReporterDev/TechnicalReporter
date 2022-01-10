#include "stdafx.h"
#include "TextDiffSuit.h"
#include "TextDiff.h"
using namespace TR::Core;
using namespace TR::XML;
namespace {

void add_line_diff_test()
{
    std::wstring old_text =
        L"111\n"
        L"222\n"
        L"333";
    
    std::wstring new_text =
        L"111\n"
        L"222\n"
        L"aaa\n"
        L"333";

    auto diff_lines = make_diff(old_text, new_text);
    
    TEST_ASSERT(diff_lines.size() == 1);
    TEST_ASSERT(diff_lines == std::vector<DiffLine>({
        {DIFF_ADD, 2, L"aaa"}
    }));
}

void remove_line_diff_test()
{
    std::wstring old_text =
        L"111\n"
        L"222\n"
        L"333\n"
        L"444";
    
    std::wstring new_text =
        L"111\n"
        L"222\n"
        L"444";

    auto diff_lines = make_diff(old_text, new_text);
    
    TEST_ASSERT(diff_lines.size() == 1);
    TEST_ASSERT(diff_lines == std::vector<DiffLine>({
        {DIFF_DELETE, 2, L"333"}
    }));
}

void update_line_diff_test()
{   
    std::wstring old_text =
        L"111\n"
        L"222\n"
        L"333\n"
        L"444";
    
    std::wstring new_text =
        L"111\n"
        L"222\n"
        L"aaa\n"
        L"444";

    auto diff_lines = make_diff(old_text, new_text);
    
    TEST_ASSERT(diff_lines.size() == 2);
    TEST_ASSERT(diff_lines == std::vector<DiffLine>({
        {DIFF_DELETE, 2, L"333"},
        {DIFF_ADD, 2, L"aaa"}
    }));
}

void twist_diff_test()
{   
    std::wstring old_text =
        L"111\n"
        L"333\n"
        L"222\n"
        L"333\n"
        L"444";
    
    std::wstring new_text =
        L"111\n"
        L"222\n"
        L"333\n"
        L"444";

    auto diff_lines = make_diff(old_text, new_text);
    
    TEST_ASSERT(diff_lines.size() == 1);
    TEST_ASSERT(diff_lines == std::vector<DiffLine>({
        {DIFF_DELETE, 1, L"333"}
    }));
}
} //namespace {

test_assist::test_suit get_text_diff_suit()
{
    test_assist::test_suit ts(L"text_diff_suit");
    ts
        .add({L"xml_add_node_diff_test", &add_line_diff_test})
        .add({L"remove_line_diff_test", &remove_line_diff_test})
        .add({L"update_line_diff_test", &update_line_diff_test})
        .add({L"twist_diff_test", &twist_diff_test});

    return ts;
}