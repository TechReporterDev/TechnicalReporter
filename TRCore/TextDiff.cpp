#include "stdafx.h"
#include "TextDiff.h"
#include "boost/algorithm/string.hpp"
#include "dtl/dtl.hpp"

namespace TR { namespace Core {

bool operator == (const DiffLine& left, const DiffLine& right)
{
    if (left.m_diff_state != right.m_diff_state)
        return false;

    if (left.m_line_pos != right.m_line_pos)
        return false;

    if (left.m_line_str != right.m_line_str)
        return false;

    return true;
}

std::vector<std::wstring> split(const std::wstring& text)
{
    std::vector<std::wstring> result;
    std::wstring line;
    std::wistringstream stream(text);
    while (std::getline(stream, line))
    {
        result.push_back(line);
    }
    return result;
}

std::vector<DiffLine> make_diff(const std::wstring& old_text, const std::wstring& new_text)
{
    using Line = boost::iterator_range<std::wstring::const_iterator>;
    std::vector<Line> old_text_lines;
    boost::split(old_text_lines, old_text, boost::is_any_of(L"\n"));

    std::vector<Line> new_text_lines;
    boost::split(new_text_lines, new_text, boost::is_any_of(L"\n"));

    dtl::Diff<Line> d(old_text_lines, new_text_lines);
    d.compose();

    std::vector<DiffLine> result;
    int old_line_num = 0;
    int new_line_num = 0;

    for (auto& item : d.getSes().getSequence())
    {
        switch (item.second.type)
        {
        case dtl::SES_ADD:
            result.push_back({DIFF_ADD, new_line_num++, {item.first.begin(), item.first.end()}});
            break;

        case dtl::SES_DELETE:
            result.push_back({DIFF_DELETE, old_line_num++, {item.first.begin(), item.first.end()}});
            break;

        case dtl::SES_COMMON:
            ++new_line_num;
            ++old_line_num;
            break;

        default:
            _ASSERT(false);
        }
    }
    return result;
}

}} //namespace TR { namespace Core {