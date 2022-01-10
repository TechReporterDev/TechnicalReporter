#include "stdafx.h"
#include "XmlNodeFilter.h"
namespace TR { namespace Core {
using namespace xml_tools;

void add_node_filter(XmlQueryDoc& query_doc, const XmlNodeFilter& xml_node_filter)
{
    auto conditional_node = static_cast<XML::XmlConditionalQueryNode*>(find_xpath_node(query_doc, xml_node_filter.m_xpath.c_str()));
    if (!conditional_node)
    {
        throw std::logic_error("Conditional node not found");
    }

    conditional_node->add_condition(create_condition(
        XML::XmlConditionNode::Action(xml_node_filter.m_action),
        XML::XmlConditionNode::Predicate(xml_node_filter.m_predicate),
        xml_node_filter.m_patterns));
}

}} //namespace TR { namespace Core {