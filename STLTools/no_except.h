#pragma once
#include <type_traits>
namespace stl_tools {

template <class Functor, class... Args>
typename std::result_of<Functor(Args&&...)>::type no_except(Functor& functor, Args&&... args)
{
    try
    {
        return functor(std::forward<Args>(args)...);
    }
    catch (...)
    {
        terminate();
    }
}

} //namespace stl_tools {