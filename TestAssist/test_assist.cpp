#include "stdafx.h"
#include "test_assist.h"

namespace test_assist {

test_case::test_case(const std::wstring& name, test_func tf):
    m_name(name),
    m_tf(tf)
{
}

test_suit::test_suit(const std::wstring& name):
    m_name(name)
{
}

test_suit& test_suit::add(test_case tc)
{
    m_tcs.push_back(std::move(tc));
    return *this;
}

test_suit::const_iterator test_suit::begin() const
{
    return m_tcs.begin();
}

test_suit::const_iterator test_suit::end() const
{
    return m_tcs.end();
}

test_runner::test_runner(std::wostream& strm):
        m_strm(strm),
        m_succeeded(0),
        m_failed(0)
    {
    }

void test_runner::run(const test_suit& ts)
{
    int succeeded = 0;
    int failed = 0;

    for(const auto& tc : ts)
    {
        try
        {
            tc.m_tf();
            ++succeeded;            
        }
        catch(std::exception& e)
        {
            std::wcout << tc.m_name << L" :";
            std::cout << e.what() << std::endl;
            ++failed;
//          throw;
        }
    }

    std::wcout.width(25);
    std::wcout << std::left << ts.m_name;
    std::wcout.width(20);
    std::wcout << std::right << L"succeeded: ";
    std::wcout.width(3);
    std::wcout << std::right << succeeded;
    std::wcout.width(20);
    std::wcout << std::right << L"failed: ";
    std::wcout.width(3);
    std::wcout << std::right << failed;
    std::wcout << std::endl;

    m_succeeded += succeeded;
    m_failed += failed;
}

void test_runner::show_stats() const
{
    std::wcout.width(25);
    std::wcout << std::left << "summary:";
    std::wcout.width(20);
    std::wcout << std::right << L"succeeded: ";
    std::wcout.width(3); 
    std::wcout << std::right << m_succeeded;
    std::wcout.width(20);
    std::wcout << std::right << L"failed: ";
    std::wcout.width(3);
    std::wcout << std::right << m_failed;
    std::wcout << std::endl;
}

} //namespace test_assist {
