#include "stdafx.h"
#include "OpenStreamDlg.h"
#include "RefValue.h"
#include "CustomValue.h"
#include "CommonPropertyPage.h"
#include "StreamPage.h"
#include "Application.h"

OpenStreamDlg::OpenStreamDlg(TR::StreamInfo streamInfo):
    PropertyDialog("Open stream"),
    m_streamInfo(std::move(streamInfo)),
    m_period(LAST_DAY)
{
    setWidth(300);
    setHeight(380);

    auto now = time(nullptr);
    localtime_s(&m_startDate, &now);
    m_startDate.tm_hour = 0;
    m_startDate.tm_min = 0;
    m_startDate.tm_sec = 0;

    m_stopDate = m_startDate;
    m_stopDate.tm_hour = 23;
    m_startDate.tm_min = 59;
    m_startDate.tm_sec = 59;

    init(std::make_unique<CommonPropertyPage>(CommonProperty{L"", L"Select period of time for stream.",
        SwitchPropertyEx(L"", L"", {
            {L"Last day", boost::none},
            {L"Last week", boost::none},
            {L"Custom", CommonProperty(L"", L"Setup custom period", 
                DateProperty(L"Start day", L"", refValue(m_startDate)), 
                DateProperty(L"Stop day", L"", refValue(m_stopDate)))
            }
        }, refValue(m_period))
    }, 0));     
}

void OpenStreamDlg::apply()
{
    time_t now = time(nullptr);
    time_t day = 24 * 60 * 60;
    time_t week = 7 * day;
    time_t start = 0;
    time_t stop = 0;

    switch (m_period)
    {
    case LAST_DAY:
        start = now - day;
        stop = now - StreamWidget::DELAY;
        break;

    case LAST_WEEK:
        start = now - day;
        stop = now - StreamWidget::DELAY;
        break;

    case CUSTOM_PERIOD:
        start = mktime(&m_startDate);
        stop = mktime(&m_stopDate);
        break;

    default:
        _ASSERT(false);
    }

    Application::showPage(std::make_unique<StreamPage>(m_streamInfo, start, stop, m_period != CUSTOM_PERIOD));
}