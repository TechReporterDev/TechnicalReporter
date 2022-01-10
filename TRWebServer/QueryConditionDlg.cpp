#include "stdafx.h"
#include "QueryConditionDlg.h"
#include "CommonPropertyPage.h"
#include <boost/algorithm/string.hpp>
#include "Application.h"

QueryConditionDlg::QueryConditionDlg(std::unique_ptr<TR::XML::XmlConditionNode> conditionNode):
    PropertyDialog("Condition"),
    m_conditionNode(std::move(conditionNode)),
    m_predicate(m_conditionNode->get_predicate()),
    m_action(m_conditionNode->get_action())
{
    for (auto& pattern : m_conditionNode->get_patterns())
    {
        if (!m_patterns.empty())
        {
            m_patterns.append(L"\n");
        }
        m_patterns.append(std::move(pattern));
    }

    init(std::make_unique<CommonPropertyPage>(CommonProperty{L"Parameters", L"Enter condition parameters",
        SelectorProperty(L"Action", L"", {L"Accept", L"Reject"}, refValue(m_action)),
        SelectorProperty(L"Predicate", L"", {L"Equal", L"Match", L"Less", L"More"}, refValue(m_predicate)),
        TextProperty(L"Patterns", L"", 4, refValue(m_patterns))
    }, 0));
}

void QueryConditionDlg::apply()
{
    std::vector<std::wstring> patterns;
    boost::split(patterns, m_patterns, boost::is_any_of(L"\r\n"), boost::algorithm::token_compress_on);

    m_conditionNode->set_predicate(m_predicate);
    m_conditionNode->set_action(m_action);
    m_conditionNode->set_patterns(std::move(patterns));

    m_onOK(std::move(m_conditionNode));
}

void QueryConditionDlg::setOnOK(OnOK onOK)
{
    m_onOK = onOK;
}