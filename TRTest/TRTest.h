#pragma once
#define TEST_FORMAT(msg) (boost::format("%1%: %2% - %3%") % (__FILE__) % __LINE__ % msg).str()
#define TEST_FAILED(msg) throw std::exception((TEST_FORMAT(msg)).c_str());
#define TEST_ASSERT(condition) if(!(condition)) TEST_FAILED(#condition)
#define TEST_ASSERT_MSG(condition, msg) if(!(condition)) TEST_FAILED(msg)
#define TEST_EXCEPTION(statement) try{(statement); TEST_FAILED(#statement)} catch(std::exception&){}

typedef std::function<void(void)> TestFunc;

struct TestCase
{
    TestCase(const std::wstring& name, TestFunc test_func) :
        m_name(name), m_test_func(test_func)
    {
    }

    std::wstring m_name;
    TestFunc m_test_func;
};

struct TestSuit
{
    typedef std::vector<TestCase> Container;
    typedef Container::const_iterator ConstIterator;

    TestSuit(const std::wstring& name) :
        m_name(name)
    {
    }

    TestSuit& add(TestCase test_case)
    {
        m_test_cases.push_back(std::move(test_case));       
        return *this;
    }

    ConstIterator begin() const
    {
        return m_test_cases.begin();
    }

    ConstIterator end() const
    {
        return m_test_cases.end();
    }

    std::wstring m_name;
    std::vector<TestCase> m_test_cases;
};