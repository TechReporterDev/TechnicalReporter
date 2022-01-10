#include "stdafx.h"
#include "UsersPage.h"
#include "PropertyDialog.h"
#include "PropertyBox.h"
#include "RefValue.h"
#include "CommonPropertyPage.h"
#include "Application.h"

class UserSettingsDlg: public PropertyDialog
{
public:
    UserSettingsDlg(const TR::UserSettings& userSettings, TR::Key userKey = TR::Key(0)):
        PropertyDialog("User settings"),
        m_userSettings(userSettings),
        m_userKey(userKey)
    {
        init(std::make_unique<CommonPropertyPage>(CommonProperty{
            L"", 
            L"Enter user settings",
            StringProperty(L"Name", L"", refValue(m_userSettings.m_name)),
            StringProperty(L"Email", L"", refValue(m_userSettings.m_email))
        }, 0));
    }

    void connectApply(std::function<void()> onApply)
    {
        _ASSERT(!m_onApply);
        m_onApply = onApply;
    }

private:
    virtual void apply() override
    {
        auto& client = Application::instance()->getClient();
        if (m_userKey)
        {
            client.updateUser(m_userKey, m_userSettings);
        }
        else
        {
            m_userKey = client.addUser(m_userSettings);
        }

        if (m_onApply)
        {
            m_onApply();
        }
    }

    std::function<void()> m_onApply;
    TR::UserSettings m_userSettings; 
    TR::Key m_userKey;
};

UsersWidget::UsersWidget():
    m_inventory(nullptr)
{
    createToolBar();

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);
    
    m_inventory = vbox->addWidget(std::make_unique<InventoryTable>());
    m_inventory->addTool(L"Setup", boost::bind(&UsersWidget::onSetupClick, this, _1), true);
    m_inventory->addTool(L"Remove", boost::bind(&UsersWidget::onRemoveClick, this, _1), false); 
    refresh();  
}

void UsersWidget::refresh()
{
    m_users = m_client->getUsers();
    m_inventory->resize(m_users.size());
    for (size_t position = 0; position < m_users.size(); ++position)
    {
        m_inventory->setItem(position, m_users[position].m_name, L"");
    }
}

void UsersWidget::createToolBar()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());

    auto addBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Add");  
    addBtn->clicked().connect(std::bind(&UsersWidget::onAddClick, this));
}

void UsersWidget::onAddClick()
{
    auto settingsDlg = new UserSettingsDlg(TR::UserSettings());
    settingsDlg->connectApply([this](){
        refresh();
    });
    settingsDlg->show();
}

void UsersWidget::onSetupClick(int position)
{
    auto settingsDlg = new UserSettingsDlg(m_users[position], m_users[position].m_key);
    settingsDlg->connectApply([this](){
        refresh();
    });
    settingsDlg->show();
}

void UsersWidget::onRemoveClick(int position)
{
    auto propertyBox = new PropertyBox(stl_tools::flag | Dialog::BTN_OK | Dialog::BTN_CANCEL, L"Remove user", L"Do you really want to remove user?", Wt::Icon::Question);
    propertyBox->show([this, position]{
        m_client->removeUser(m_users[position].m_key);
        refresh();
    });
}

std::unique_ptr<MainFramePage> UsersPage::createPage(const std::string& url)
{
    if (url == "/users")
    {
        return std::make_unique<UsersPage>();
    }
    return nullptr;
}

UsersPage::UsersPage():
    MainFramePage(L"Users", "/users")
{
}

MainFrameWidget* UsersPage::getWidget()
{
    if (!m_widget)
    {
        m_widget.reset(new UsersWidget());
    }
    return m_widget.get();
}