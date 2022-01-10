#include "stdafx.h"
#include "encoding.h"
#include "u8decdfa.h"
#include <codecvt>

#pragma warning(disable:4996)
namespace stl_tools {

std::string ucs_to_utf8(const std::wstring& str)
{
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(str);
}

std::wstring utf8_to_ucs(const std::string& str)
{
    //return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(str);
    return decode(str.c_str());
}

std::string ucs_to_utf8(const wchar_t* str)
{
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(str);
}

std::wstring utf8_to_ucs(const char* str)
{
    //return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(str);
    return decode(str);
}

std::wstring utf8_to_ucs(const unsigned char* str)
{
    return decode((const char*)str);
}

std::string ucs_to_ansi(const std::wstring& str)
{
    size_t size = wcstombs(nullptr, str.c_str(), 0);   
    if(size!=-1)
    {
        std::vector<char> buf(size+1);
        wcstombs(&buf[0], str.c_str(), buf.size()); 
        return &buf[0];
    }
    return "";
}

std::wstring ansi_to_ucs(const std::string& str)
{
    size_t size = mbstowcs(nullptr, str.c_str(), 0);   
    if(size!=-1)
    {
        std::vector<wchar_t> buf(size+1);
        mbstowcs(&buf[0], str.c_str(), buf.size()); 
        return &buf[0];
    }
    return L"";
}

} //namespace stl_tools {