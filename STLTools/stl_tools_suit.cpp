#include "stdafx.h"
#include "stl_tools_suit.h"
#include "stl_tools.h"
#include "storage.h"
#include "make_unique.h"
using namespace stl_tools;
namespace {

void make_unique_test()
{
    auto str1 = make_unique<std::wstring>();
    TEST_ASSERT(str1);
    TEST_ASSERT(str1->empty());

    auto str2 = make_unique<std::wstring>(L"123");
    TEST_ASSERT(str2);
    TEST_ASSERT(*str2 == L"123");
}

struct S
{
    int m_int;
};

S* incr(S* p)
{
    return ++p;
}

S* decr(S* p)
{
    return --p;
}

S& ref(S* p)
{
    return *p;
}

S* ptr(S* p)
{
    return p;
}

void forward_iterator_adapter_test()
{
    S m[] = { { 1 }, { 2 }, { 3 }, { 4 }, { 5 }, { 6 }, { 7 }, { 8 }, { 9 } };
    
    typedef iterator_adapter<nontrivial_forward_static_iterator_traits<S*, incr, S*, ptr, S&, ref>> nfs_iterator;
    typedef iterator_adapter<trivial_forward_static_iterator_traits<S*, incr, S, ptr>> tfs_iterator;
    typedef iterator_adapter<primitive_forward_static_iterator_traits<S, incr>> pfs_iterator;

    int i_1 = 0;
    nfs_iterator nfsi_1(m);
    nfs_iterator nfsi_2(m + _countof(m));
    while (nfsi_1 != nfsi_2)
    {
        TEST_ASSERT(&m[i_1] == &*nfsi_1);
        TEST_ASSERT(&nfsi_1->m_int == &(*nfsi_1).m_int);
        ++i_1;
        ++nfsi_1;
    }
    TEST_ASSERT(i_1 == _countof(m));

    int i_2 = 0;
    tfs_iterator tfsi_1(m);
    tfs_iterator tfsi_2(m + _countof(m));
    while (tfsi_1 != tfsi_2)
    {
        TEST_ASSERT(&m[i_2] == &*tfsi_1);
        TEST_ASSERT(&tfsi_1->m_int == &(*tfsi_1).m_int);
        ++i_2;
        ++tfsi_1;       
    }
    TEST_ASSERT(i_2 == _countof(m));

    int i_3 = 0;
    pfs_iterator pfsi_1(m);
    pfs_iterator pfsi_2(m + _countof(m));
    while (pfsi_1 != pfsi_2)
    {
        TEST_ASSERT(&m[i_3] == &*pfsi_1);
        TEST_ASSERT(&pfsi_1->m_int == &(*pfsi_1).m_int);
        ++i_3;
        ++pfsi_1;
    }
    TEST_ASSERT(i_3 == _countof(m));
}

void ordered_signal_args_test()
{   
    ordered_signal<void(void)> signal_0;
    int call_count_0 = 0;
    signal_0.connect([&]{
        ++call_count_0;
    }, 1);
    signal_0();
    signal_0();
    signal_0();
    TEST_ASSERT(call_count_0 == 3);

    ordered_signal<void(int)> signal_1;
    int sum = 0;
    signal_1.connect([&](int val){
        sum += val;
    }, 0);
    signal_1(1);
    signal_1(2);
    signal_1(3);
    TEST_ASSERT(sum == 1 + 2 + 3);

    ordered_signal<void(std::string, std::string)> signal_2;
    std::string concat;
    signal_2.connect([&](std::string val1, std::string val2){
        concat += val1;
        concat += val2;
    }, 0);
    signal_2(std::string("1"), std::string("2"));
    signal_2(std::string("3"), std::string("4"));
    signal_2(std::string("5"), std::string("6"));
    TEST_ASSERT(concat == "123456");
}

void ordered_signal_priority_test()
{
    enum Priority {P1, P2, P3};

    std::vector<std::pair<Priority, int>> calls;    
    int call_number = 1;
    ordered_signal<void(int)> signal_1; 
    ordered_signal<void(int)> signal_2;

    signal_1.connect([&](int number){
        calls.push_back({P1, number});
    }, P1);

    signal_2.connect([&](int number){
        calls.push_back({P1, number});
    }, P1);
    
    signal_1.connect([&](int number){
        calls.push_back({P2, number});
        signal_2(call_number++);
    }, P2);

    signal_2.connect([&](int number){
        calls.push_back({P2, number});
    }, P2);

    signal_1.connect([&](int number){
        calls.push_back({P3, number});
    }, P3);

    signal_2.connect([&](int number){
        calls.push_back({P3, number});
    }, P3);

    signal_1(call_number++);

    std::vector<std::pair<Priority, int>> prediction({{P1, 1}, {P2, 1}, {P1, 2}, {P2, 2}, {P3, 1}, {P3, 2}});
    TEST_ASSERT(calls == prediction);
}

void member_iterator_test()
{
    struct  S
    {
        int m_i;
        std::wstring m_s;
    };

    {
        std::vector<S> container = { { 0, L"AAA" }, { 1, L"BBB" } };
        for (auto i = make_member_iterator(container.begin(), &S::m_i); i != make_member_iterator(container.end(), &S::m_i); ++i)
        {
            ++(*i);
        }
        _ASSERT(container[0].m_i == 1);
        _ASSERT(container[1].m_i == 2);
    }

    {
        std::vector<S> container = { { 0, L"AAA" }, { 1, L"BBB" } };

        for (int& i : make_member_range(container.begin(), container.end(), &S::m_i))
        {
            ++i;
        }
        _ASSERT(container[0].m_i == 1);
        _ASSERT(container[1].m_i == 2);
    }

    {
        std::vector<S> container = { { 0, L"AAA" }, { 1, L"BBB" } };
        for (int& i : container | members(&S::m_i))
        {
            ++i;
        }
        _ASSERT(container[0].m_i == 1);
        _ASSERT(container[1].m_i == 2);
    }
}

struct string_copy: unique_storage_index<std::string>
{
    static std::string get_key(const std::string& s)
    {
        return s;
    }
};

struct string_size: single_storage_index<int>
{
    static int get_key(const std::string& s)
    {
        return s.size();
    }
};

struct string_alpha: multi_storage_index<char>
{
    template<class F>
    static void enum_keys(const std::string& s, F f)
    {
        for (auto& alpha : s)
        {
            f(alpha);
        }
    }
};

using empty_string = memfun_index_traits<bool, std::string, &std::string::empty> ;

void storage_test()
{
    storage<std::string, string_copy> string_storage;
    storage<std::string, string_copy>::root_index_type index_node;
    storage<std::string, string_copy>::root_index_type::enum_keys("123", [](const std::string&) {});


    string_storage.insert("");

    /*storage<std::string, string_copy, string_size, empty_string, string_alpha> string_storage;
    string_storage.insert("");
    string_storage.insert("1");
    string_storage.insert("12");
    string_storage.insert("123");
    string_storage.insert("1234");
    string_storage.insert("a");
    string_storage.insert("ab");
    string_storage.insert("abc");
    string_storage.insert("abcde");

    auto found = string_storage.find("123");
    TEST_ASSERT(found != string_storage.end() && *found == "123");
    
    auto range1 = string_storage.find_range<string_size>(3);
    TEST_ASSERT(boost::equal(range1, std::vector<std::string>{"123", "abc"}));

    auto range2 = string_storage.find_range<string_size>(5);
    TEST_ASSERT(boost::equal(range2, std::vector<std::string>{"abcde"}));

    auto range3 = string_storage.find_range<empty_string>(true);
    TEST_ASSERT(boost::equal(range3, std::vector<std::string>{""}));

    auto range4 = string_storage.find_range<empty_string>(false);
    TEST_ASSERT(boost::distance(range4) == 8);

    auto range5 = string_storage.find_range<string_alpha>('b');
    TEST_ASSERT(boost::equal(range5, std::vector<std::string>{"ab", "abc", "abcde"}));*/
}

void copy_test()
{
    std::map<std::size_t, std::string> pattern = {{1, "1"}, {2, "12"}, {3, "123"}};
    std::vector<std::string> v{"1", "12", "123"};
    
    auto m1 = copy_map(v, [](const std::string& s){
        return s.size();
    });
    TEST_ASSERT(boost::equal(m1, pattern));

    auto m2 = copy_map(v, &std::string::size);
    TEST_ASSERT(boost::equal(m2, pattern));
}

void is_one_of_test()
{
    static_assert(is_one_of<int, char, double, std::string>::value == false, "");
    static_assert(is_one_of<std::string, char, double, std::string>::value == true, "");
}

} //namespace {

test_assist::test_suit get_stl_tools_suit()
{
    test_assist::test_suit test_suit(L"stl_tools_suit");
    test_suit
        .add({L"make_unique_test", &make_unique_test})
        .add({L"forward_iterator_adapter_test", &forward_iterator_adapter_test})
        .add({L"ordered_signal_args_test", &ordered_signal_args_test})
        .add({L"ordered_signal_priority_test", &ordered_signal_priority_test})
        .add({ L"member_iterator_test", &member_iterator_test })
        .add({ L"storage_test", &storage_test });

    return test_suit;
}