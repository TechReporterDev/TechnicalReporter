#include "stdafx.h"
#include "Exception.h"

namespace TR {

Exception::Exception(const std::wstring& error):
    m_error(stl_tools::ucs_to_ansi(error))
{
}

Exception::~Exception()
{
}

const char * Exception::what() const
{
    return m_error.c_str();
}

} //namespace TR {