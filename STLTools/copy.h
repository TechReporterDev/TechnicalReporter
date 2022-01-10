#pragma once
#include "invoke.h"
#include <boost\algorithm\algorithm.hpp>
namespace stl_tools {

template <class Range>
std::vector<typename Range::value_type> copy_vector(const Range& range)
{
    return boost::copy_range<std::vector<typename Range::value_type>>(range);
}

template <class Range, class Functor, 
    class KeyType = std::remove_reference_t<decltype(invoke(std::declval<Functor&>(), std::declval<typename Range::value_type&>()))>,
    class ValueType = typename Range::value_type>
    std::map<KeyType, ValueType> copy_map(const Range& range, Functor functor)
{
    std::map<KeyType, ValueType> result;
    for (auto& value : range)
    {
        result.insert(std::make_pair(invoke(functor,value), value));
    }
    return result;
}

} //namespace stl_tools {