#pragma once
#include <type_traits>
#include <boost\optional.hpp>
namespace stl_tools {

template<bool Condition, class T>
struct add_const_if
{
    typedef typename std::conditional<Condition, const T, T>::type type;
};

template<bool Condition, class T>
using add_const_if_t = typename add_const_if<Condition, T>::type;

template<class T>
using enable_void_t = std::enable_if_t<std::is_same<T, void>::value, int>;

template<class T>
using disable_void_t = std::enable_if_t<!std::is_same<T, void>::value, int>;

template<class T>
struct is_optional
{
    static const bool value = false;
};

template<class T>
struct is_optional<boost::optional<T>>
{
    static const bool value = true;
};

template<class T, class... Tn>
struct is_one_of;

template<class T, class T1, class... Tn>
struct is_one_of<T, T1, Tn...>
{
    static const bool value = std::is_same<T, T1>::value || is_one_of<T, Tn...>::value;
};

template<class T, class T1>
struct is_one_of<T, T1>
{
    static const bool value = std::is_same<T, T1>::value;
};

} //namespace stl_tools {