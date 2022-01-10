#pragma once
#include <string>
#include <vector>
namespace TR {namespace Core {

enum DiffState { DIFF_ADD, DIFF_DELETE };

struct DiffLine
{
    DiffState       m_diff_state;
    int             m_line_pos;
    std::wstring    m_line_str;
};
bool operator == (const DiffLine& left, const DiffLine& right);
std::vector<DiffLine> make_diff(const std::wstring& old_text, const std::wstring& new_text);

}} //namespace TR { namespace Core {