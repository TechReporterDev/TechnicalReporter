#pragma once
#include "TrivialTypes.h"
#include <string>
#include <vector>
#include <boost\optional.hpp>
namespace TR { namespace Core {

struct XmlNodeFilter
{
    enum class Action { ACCEPT = 0, REJECT };
    enum class Predicate { EQUAL = 0, MATCH, LESS, MORE };

    XmlNodeFilter():
        XmlNodeFilter(L"", "", Action::ACCEPT, Predicate::EQUAL, {})
    {
    }

    XmlNodeFilter(std::wstring name, std::string xpath, Action action, Predicate predicate, std::vector<std::wstring> patterns):
        m_name(std::move(name)),
        m_xpath(std::move(xpath)),
        m_action(action),
        m_predicate(predicate),
        m_patterns(std::move(patterns))
    {
    }

    template<class Archive>
    void serialize(Archive &archive, const unsigned int /*version*/)
    {
        archive & m_name;
        archive & m_xpath;
        archive & m_action;
        archive & m_predicate;
        archive & m_patterns;
    }

    std::wstring                m_name;
    std::string                 m_xpath;
    Action                      m_action;
    Predicate                   m_predicate;
    std::vector<std::wstring>   m_patterns;
};

void add_node_filter(XML::XmlQueryDoc& query_doc, const XmlNodeFilter& xml_node_filter);

}} //namespace TR { namespace Core {