#pragma once
#include <boost\iterator\transform_iterator.hpp>
#include <boost\range\adaptor\transformed.hpp>
#include <boost\bind.hpp>
namespace stl_tools {

template<class T, class I>
auto make_member_iterator(I iterator, T typename I::value_type::* member)->decltype(boost::make_transform_iterator(iterator, std::mem_fn(member)))
{
    return boost::make_transform_iterator(iterator, std::mem_fn(member));
}

template<class T, class I>
auto make_member_range(I begin, I end, T typename I::value_type::* member) -> decltype(boost::make_iterator_range(begin, end) | boost::adaptors::transformed(boost::bind<T&>(member, _1)))
{
    return boost::make_iterator_range(begin, end) | boost::adaptors::transformed(boost::bind<T&>(member, _1));
}

template<class T, class S>
auto members(T S::* member) -> decltype(boost::adaptors::transformed(boost::mem_fn(member)))
{
    return boost::adaptors::transformed(boost::mem_fn(member));
}

template<class F>
auto transformed(F f) -> decltype(boost::adaptors::transformed(boost::bind(f, _1)))
{
    return boost::adaptors::transformed(boost::bind(f, _1));
}

} //namespace stl_tools {