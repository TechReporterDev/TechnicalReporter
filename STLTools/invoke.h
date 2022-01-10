#pragma once
#include <boost\algorithm\algorithm.hpp>
namespace stl_tools {

template <class Functor, class... Args>
auto invoke(Functor&& functor, Args&&... args) -> decltype(std::forward<Functor>(functor)(std::forward<Args>(args)...))
{
    return std::forward<Functor>(functor)(std::forward<Args>(args)...);
}

template <class R, class Base, class Derived>
auto invoke(R Base::* member, Derived&& ref) -> decltype(std::forward<Derived>(ref).*member)
{
    return std::forward<Derived>(ref).*member;
}

template <class R, class Base, class Derived>
auto invoke(R (Base::*mem_fun)(), Derived&& ref) -> decltype((std::forward<Derived>(ref).*mem_fun)())
{
    return (std::forward<Derived>(ref).*mem_fun)();
}

template <class R, class Base, class Derived>
auto invoke(R (Base::*mem_fun)() const, Derived&& ref) -> decltype((std::forward<Derived>(ref).*mem_fun)())
{
    return (std::forward<Derived>(ref).*mem_fun)();
}

} //namespace stl_tools {