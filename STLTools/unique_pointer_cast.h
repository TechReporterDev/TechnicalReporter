#pragma once
#include <memory>
namespace stl_tools {

template<typename T, typename U>
std::unique_ptr<T> static_pointer_cast(std::unique_ptr<U>&& ptr)
{
    return std::unique_ptr<T>(static_cast<T*>(ptr.release()));
}

} //namespace stl_tools {