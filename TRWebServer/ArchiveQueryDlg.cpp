#include "stdafx.h"
#include "ArchiveQueryDlg.h"
#include "RefValue.h"
#include "CustomValue.h"
#include "CommonPropertyPage.h"
#include "QueriedReportPage.h"
#include "Application.h"

ArchiveQueryDlg::ArchiveQueryDlg(const TR::SourceInfo& sourceInfo):
    PropertyDialog("Archived report"),
    m_sourceInfo(sourceInfo),
    m_reportTypePosition(0)
{
    auto now = time(nullptr);
    localtime_s(&m_date, &now);
    m_time = m_date;

    auto& client = Application::instance()->getClient();
    for (auto& reportTypeInfo : client.getReportTypesInfo())
    {
        m_reportTypes.push_back(reportTypeInfo);
    }

    init(std::make_unique<CommonPropertyPage>(CommonProperty{L"", L"Select report and time to show?",
        SelectorProperty(L"", L"",
            stl_tools::copy_vector(m_reportTypes | stl_tools::members(&TR::ReportTypeInfo::m_name)),
            refValue(m_reportTypePosition)),
        DateProperty(L"", L"", refValue(m_date)),
        TimeProperty(L"", L"", refValue(m_time))
    }, 0));     
}

void ArchiveQueryDlg::apply()
{
    std::tm tm = m_date;
    tm.tm_hour = m_time.tm_hour;
    tm.tm_min = m_time.tm_min;
    tm.tm_sec = m_time.tm_sec;  

    Application::showPage(std::make_unique<QueriedReportPage>(m_sourceInfo, m_reportTypes[m_reportTypePosition], mktime(&tm)));
}