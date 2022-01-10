//////////////////
//copy-paste from https://functionalcpp.wordpress.com/2013/08/05/function-traits/
//////////////////

#pragma once
//#include <functional>
#include <tuple>

namespace stl_tools {

template<class F>
struct function_traits;

    // function pointer
template<class R, class... Args>
struct function_traits<R(*)(Args...)>: public function_traits<R(Args...)>
{};

template<class R, class... Args>
struct function_traits<R(Args...)>
{
    using result_type = R;

    static const std::size_t arity = sizeof...(Args);

    template <std::size_t N>
    struct argument
    {
        static_assert(N < arity, "error: invalid parameter index.");
        using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
    };
};

// member function pointer
template<class C, class R, class... Args>
struct function_traits<R(C::*)(Args...)>: public function_traits<R(C&, Args...)>
{};

// const member function pointer
template<class C, class R, class... Args>
struct function_traits<R(C::*)(Args...) const>: public function_traits<R(C&, Args...)>
{};

// member object pointer
template<class C, class R>
struct function_traits<R(C::*)>: public function_traits<R(C&)>
{};

// functor
template<class F>
struct function_traits
{
private:
    using call_type = function_traits<decltype(&F::operator())>;

public:
    using result_type = typename call_type::result_type;
    static const std::size_t arity = call_type::arity - 1;

    template <std::size_t N>
    struct argument
    {
        static_assert(N < arity, "error: invalid parameter index.");
        using type = typename call_type::template argument<N + 1>::type;
    };
};

template<class F>
struct function_traits<F&>: public function_traits<F>
{};

template<class F>
struct function_traits<F&&>: public function_traits<F>
{};

} //namespace stl_tools {