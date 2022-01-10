#pragma once
#include <type_traits>
#include <functional>
namespace stl_tools {

enum class loop_stmt { CONTINUE, BREAK, CONTINUE_LEFT, CONTINUE_RIGHT };

template<class F>
loop_stmt call_loop_func(F f, std::false_type)
{
    f();
    return loop_stmt::CONTINUE;
}

template<class F>
loop_stmt call_loop_func(F f, std::true_type)
{
    return f();
}

template<class C1, class C2, class F>
void associate(C1& c1, C2& c2, F f)
{
    using c1_pointer_type = decltype(c1.begin())::pointer;
    using c2_pointer_type = decltype(c2.begin())::pointer;
    using func_result_type = decltype(f(nullptr, nullptr));

    auto c1_cur = c1.begin();
    auto c1_end = c1.end();
    
    auto c2_cur = c2.begin();
    auto c2_end = c2.end();

    while (c1_cur != c1_end || c2_cur != c2_end)
    {
        auto c1_ptr = (c1_cur == c1_end) ? c1_pointer_type(nullptr) : &*c1_cur;
        auto c2_ptr = (c2_cur == c2_end) ? c2_pointer_type(nullptr) : &*c2_cur; 
        
        auto control = call_loop_func(std::bind(f, c1_ptr, c2_ptr), std::is_same<func_result_type, loop_stmt>::type());
        if (control == loop_stmt::BREAK)
        {
            break;
        }
        
        if (c1_cur != c1_end && (control == loop_stmt::CONTINUE || control == loop_stmt::CONTINUE_LEFT))
            ++c1_cur;

        if (c2_cur != c2_end && (control == loop_stmt::CONTINUE || control == loop_stmt::CONTINUE_RIGHT))
            ++c2_cur;
    }
}

} //namespace stl_tools {