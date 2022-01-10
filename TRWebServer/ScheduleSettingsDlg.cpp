#include "stdafx.h"
#include "ScheduleSettingsDlg.h"
#include "RefValue.h"
#include "CustomValue.h"
#include "CommonPropertyPage.h"
#include "Application.h"

ScheduleSettingsDlg::ScheduleSettingsDlg():
    PropertyDialog(L"Schedule"),
    m_scheduleKey(0),
    m_scheduleSettings(L"", TR::ScheduleSettings::Period::Hourly, 1, 1, 0)
{
    init();
}

ScheduleSettingsDlg::ScheduleSettingsDlg(const TR::ScheduleInfo& scheduleInfo):
    PropertyDialog(L"Schedule"),
    m_scheduleKey(scheduleInfo.m_schedule_key),
    m_scheduleSettings(scheduleInfo)
{
    init();
}

void ScheduleSettingsDlg::init()
{
    auto schedulePropeties = [&](int){
        switch (m_scheduleSettings.m_period)
        {
        case TR::ScheduleSettings::Period::Never:
            return CommonProperty{L"Never period setup", L"Never period has no settings"};

        case TR::ScheduleSettings::Period::Hourly:
            return CommonProperty{L"Hourly period setup", L"Hourly period has no settings"};

        case TR::ScheduleSettings::Period::Daily:
            return CommonProperty{L"Daily period setup", L"Daily period has no settings"};

        case TR::ScheduleSettings::Period::Weekly:
            return CommonProperty{L"Weekly period setup", L"Weekly period has no settings"};

        case TR::ScheduleSettings::Period::Mounthly:
            return CommonProperty{L"Mounthly period setup", L"Mounthly period has no settings"};

        case TR::ScheduleSettings::Period::Often:
            return CommonProperty{L"Often period setup", L"Often period has no settings"};

        default:
            _ASSERT(false);
        }
        return CommonProperty{L"", L""}; //to supress warning
    };

    PropertyDialog::init(std::make_unique<CommonPropertyPage>(CommonProperty{L"", L"Setup schedule settings",
        StringProperty(L"Name", L"", refValue(m_scheduleSettings.m_name)),
        SelectorPropertyEx(L"Period", L"", L"Setup", schedulePropeties, 
            {L"Never", L"Hourly", L"Daily", L"Weekly", L"Mounthly", L"Often"},
            refValue(m_scheduleSettings.m_period))
    }, 0));
}

void ScheduleSettingsDlg::connectApply(OnApply onApply)
{
    m_onApply = onApply;
}

void ScheduleSettingsDlg::apply()
{
    auto& client = Application::instance()->getClient();
    if (!m_scheduleKey)
    {
        m_scheduleKey = client.addSchedule(m_scheduleSettings);
    }
    else
    {
        client.updateSchedule(m_scheduleKey, m_scheduleSettings);
    }

    if (m_onApply)
    {
        m_onApply();
    }
}