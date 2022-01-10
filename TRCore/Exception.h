#pragma once
#include <exception>
namespace TR {

class Exception: public std::exception
{
public:
    Exception(const std::wstring& error);
    ~Exception();
    virtual const char * what() const;

private:
    std::string m_error;
};

} //namespace TR { 