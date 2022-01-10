#pragma once
#include <string>
namespace stl_tools {

std::string     ucs_to_utf8(const std::wstring& str);
std::wstring    utf8_to_ucs(const std::string& str);
std::string     ucs_to_utf8(const wchar_t* str);
std::wstring    utf8_to_ucs(const char* str);
std::wstring    utf8_to_ucs(const unsigned char* str);

std::string     ucs_to_ansi(const std::wstring& str);
std::wstring    ansi_to_ucs(const std::string& str);

} //namespace stl_tools {