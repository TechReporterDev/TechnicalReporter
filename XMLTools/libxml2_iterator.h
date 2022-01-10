#pragma once
#include "libxml2_wrap.h"
#include "STLTools\iterator_adapter.h"
namespace xml_tools {

template <class Node, Node* Next(Node*)>
using node_iterator = stl_tools::iterator_adapter<stl_tools::primitive_forward_static_iterator_traits<Node, Next>>;
using sibling_node_iterator = node_iterator<XmlNode, next_child>;
using const_sibling_node_iterator = node_iterator<const XmlNode, next_child>;

} //namespace xml_tools {