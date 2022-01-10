#pragma once
#include "TRWebServer.h"
#include "PropertyDialog.h"
#include "MainFrame.h"

class RoleSettingsDlg: public PropertyDialog
{
public:
    typedef std::function<void(TR::RoleKey roleKey, const TR::RoleSettings&)> OnApply;
    
    RoleSettingsDlg(TR::RoleKey roleKey = TR::RoleKey(0));
    void connectApply(OnApply onApply);

private:
    // PropertyDialog override
    virtual void apply() override;

    int findRole(TR::Key roleKey);

    TR::RoleKey m_roleKey;
    TR::RoleSettings m_roleSettings;
    std::vector<TR::RoleInfo> m_rolesInfo;  
    OnApply m_onApply;
};
