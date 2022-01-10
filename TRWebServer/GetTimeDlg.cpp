#include "stdafx.h"
#include "GetTimeDlg.h"
#include "RefValue.h"
#include "CustomValue.h"
#include "CommonPropertyPage.h"
#include "Application.h"

GetTimeDlg::GetTimeDlg(const Wt::WString& caption, Wt::WObject* parent):
    PropertyDialog(caption, parent)
{
    auto now = time(nullptr);
    localtime_s(&m_date, &now);
    m_time = m_date;

    init(std::make_unique<CommonPropertyPage>(CommonProperty{L"", L"Setup day time",
        DateProperty(L"", L"", refValue(m_date)),
        TimeProperty(L"", L"", refValue(m_time))
    }, 0));
}

void GetTimeDlg::show(OnOK onOK)
{
    m_onOK = onOK;
    PropertyDialog::show();
}

void GetTimeDlg::apply()
{
    std::tm tm = m_date;
    tm.tm_hour = m_time.tm_hour;
    tm.tm_min = m_time.tm_min;
    tm.tm_sec = m_time.tm_sec;

    _ASSERT(m_onOK);
    m_onOK(mktime(&tm));
}