#include "stdafx.h"
#include "RoleSettingsDlg.h"
#include "RefValue.h"
#include "CustomValue.h"
#include "CommonPropertyPage.h"
#include "Application.h"

RoleSettingsDlg::RoleSettingsDlg(TR::RoleKey roleKey):
    PropertyDialog(L"Role Settings"),
    m_roleKey(roleKey),
    m_roleSettings(L"", TR::DEFAULT_ROLE_KEY)
{
    auto& client = Application::instance()->getClient();
    if (m_roleKey)
    {
        m_roleSettings = client.getRoleInfo(m_roleKey);
    }
    m_rolesInfo = client.getRolesInfo();    

    init(std::make_unique<CommonPropertyPage>(CommonProperty{L"", L"Enter role settings", 
        StringProperty(L"Name", L"", refValue(m_roleSettings.m_name)),
        SelectorProperty(L"Parent", L"", stl_tools::copy_vector(m_rolesInfo | stl_tools::members(&TR::RoleInfo::m_name)), 
            customValue<int>(
                [&]{ return findRole(m_roleSettings.m_parent_key); },
                [&](int index){ m_roleSettings.m_parent_key = m_rolesInfo[index].m_key; }
            ))
    }, 0));
}

void RoleSettingsDlg::connectApply(OnApply onApply)
{
    m_onApply = onApply;
}

void RoleSettingsDlg::apply()
{
    auto& client = Application::instance()->getClient();
    if (!m_roleKey)
    {
        m_roleKey = client.addRole(m_roleSettings);
    }
    else
    {
        client.updateRole(m_roleKey, m_roleSettings);
    }

    if (m_onApply)
    {
        m_onApply(m_roleKey, m_roleSettings);
    }
}

int RoleSettingsDlg::findRole(TR::Key roleKey)
{
    for (int i = 0; i < (int)m_rolesInfo.size(); ++i)
    {
        if (m_rolesInfo[i].m_key == roleKey)
        {
            return i;
        }
    }
    throw std::logic_error("RoleSettingsDlg: parent role node found");
}