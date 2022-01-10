#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "PropertyDialog.h"

class ScheduleSettingsDlg: public PropertyDialog
{
public:
    typedef std::function<void()> OnApply;
    
    ScheduleSettingsDlg();
    ScheduleSettingsDlg(const TR::ScheduleInfo& scheduleInfo);
    void connectApply(OnApply onApply);

protected:
    // PropertyDialog override
    virtual void apply() override;

private:
    void init();

    TR::Key m_scheduleKey;
    TR::ScheduleSettings m_scheduleSettings;
    OnApply m_onApply;
};
