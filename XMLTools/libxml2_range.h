#pragma once
#include "libxml2_iterator.h"
#include "STLTools\flag.h"
#include <libxml\tree.h>
#include <boost\range.hpp>
#include <boost\iterator\filter_iterator.hpp>

namespace xml_tools {

inline boost::iterator_range<sibling_node_iterator> get_child_nodes(XmlNode& parent_node)
{
    return boost::make_iterator_range(sibling_node_iterator(begin_child(parent_node)), sibling_node_iterator());
}

inline boost::iterator_range<const_sibling_node_iterator> get_child_nodes(const XmlNode& parent_node)
{
    return boost::make_iterator_range(const_sibling_node_iterator(begin_child(parent_node)), const_sibling_node_iterator());
}

template <class Predicate, class Iterator> 
using filter_iterator_range = boost::iterator_range<boost::filter_iterator<Predicate, Iterator>>;

template <class Predicate> 
inline filter_iterator_range<Predicate, sibling_node_iterator> get_child_nodes(XmlNode& parent_node, Predicate pred)
{
    return boost::make_iterator_range(
        boost::make_filter_iterator(pred, sibling_node_iterator(begin_child(parent_node))),
        boost::make_filter_iterator(pred, sibling_node_iterator()));
}

template <class Predicate> 
inline filter_iterator_range<Predicate, const_sibling_node_iterator> get_child_nodes(const XmlNode& parent_node, Predicate pred)
{
    return boost::make_iterator_range(
        boost::make_filter_iterator(pred, const_sibling_node_iterator(begin_child(parent_node))),
        boost::make_filter_iterator(pred, const_sibling_node_iterator()));
}

struct node_type_equal 
{
    node_type_equal(stl_tools::flag_type<XmlNodeType> type):
        m_type(type)
    {
    }

    bool operator()(const XmlNode& node) const
    {
        return m_type.contains(get_node_type(node));
    }

    stl_tools::flag_type<XmlNodeType> m_type;
};

filter_iterator_range<node_type_equal, sibling_node_iterator>
inline get_child_nodes(XmlNode& parent_node, stl_tools::flag_type<XmlNodeType> type)
{
    return get_child_nodes(parent_node, node_type_equal(type));
}

filter_iterator_range<node_type_equal, const_sibling_node_iterator>
inline get_child_nodes(const XmlNode& parent_node, stl_tools::flag_type<XmlNodeType> type)
{
    return get_child_nodes(parent_node, node_type_equal(type));
}

filter_iterator_range<node_type_equal, sibling_node_iterator>
inline get_child_nodes(XmlNode& parent_node, XmlNodeType type)
{
    return get_child_nodes(parent_node, node_type_equal(type));
}

filter_iterator_range<node_type_equal, const_sibling_node_iterator>
inline get_child_nodes(const XmlNode& parent_node, XmlNodeType type)
{
    return get_child_nodes(parent_node, node_type_equal(type));
}

} //namespace xml_tools {