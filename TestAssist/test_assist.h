#pragma once

#include <stdexcept>
#include <memory>
#include <functional>
#include <vector>
#include <iostream>
#include <string>
#include <boost/format.hpp>

namespace test_assist {

typedef std::function<void(void)> test_func;

struct test_case
{
    test_case(const std::wstring& name, test_func tf);

    std::wstring    m_name;
    test_func       m_tf;
};

struct test_suit
{
    typedef std::vector<test_case> container;
    typedef container::const_iterator const_iterator;

    test_suit(const std::wstring& name);

    test_suit&      add(test_case tc);
    const_iterator  begin() const;
    const_iterator  end() const;

    std::wstring    m_name;
    container       m_tcs;
};

struct test_runner
{
    test_runner(std::wostream& strm = std::wcout);
    void run(const test_suit& ts);
    void show_stats() const;

    std::wostream& m_strm;
    int m_succeeded;
    int m_failed;
};

struct test_error: std::logic_error
{
    test_error(const char* msg):
        std::logic_error(msg)
    {
    }
};

} //namespace test_assist {

#define TEST_FORMAT(msg) (boost::format("%1%: %2% - %3%") % (__FILE__) % __LINE__ % msg).str()
#define TEST_FAILED(msg) throw test_assist::test_error((TEST_FORMAT(msg)).c_str());
#define TEST_ASSERT(condition) if(!(condition)) TEST_FAILED(#condition)
#define TEST_ASSERT_MSG(condition, msg) if(!(condition)) TEST_FAILED(msg)
#define TEST_EXCEPTION(statement) try{(statement); TEST_FAILED(#statement)} catch(test_assist::test_error&){throw;} catch(std::exception&){}
#define TEST_NO_EXCEPTION(statement) (statement);