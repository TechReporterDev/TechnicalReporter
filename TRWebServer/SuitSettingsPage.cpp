#include "stdafx.h"
#include "SuitSettingsPage.h"
#include "SourcesPage.h"
#include "CommonPropertyPage.h"
#include "CommonPropertyView.h"
#include "RefValue.h"
#include "CustomValue.h"
#include "StaticValue.h"
#include "ContainerArray.h"
#include "CommonPropertyDoc.h"
#include "SourceBrowseDlg.h"
#include "Application.h"

class SuitSettingsWidget::SecurityPage: public PropertyPageWidget
{
public:
    SecurityPage(SuitSettingsWidget* suitSettingsWidget):
        PropertyPageWidget(L"Security", L"Suit security settings", 0),
        m_suitSettingsWidget(suitSettingsWidget),
        m_accessRightsView(nullptr),
        m_accessInfo(suitSettingsWidget->m_accessInfo)
    {
        initialize();

        setPadding(5);
        setOverflow(Wt::Overflow::Auto);

        showUserList();
        showAccessRights(m_users[0].m_key);
    }

    void initialize()
    {
        auto& client = Application::instance()->getClient();
        m_users = client.getUsers();

        if (!m_accessInfo)
        {
            if (auto suitKey = m_suitSettingsWidget->m_suitKey)
            {
                m_accessInfo = client.getAccessInfo(suitKey);
                return;
            }
            m_accessInfo = TR::SourceAccessInfo(TR::SourceKey{}, {}, {});
        }
    }

    void showUserList()
    {
        SelectionListProperty usersList(L"Users", L"", stl_tools::copy_vector(m_users | stl_tools::members(&TR::UserInfo::m_name)),
            customValue<int>([]{ return 0; }, [this](int position){
                showAccessRights(m_users[position].m_key);
            })
        );

        addWidget(std::make_unique<SelectionListPropertyView>(usersList));
    }

    void showAccessRights(TR::Key userKey)
    {
        auto checkerExtension = [this, userKey](bool checked)
        {
            if (checked)
            {
                return CommonProperty(L"", L"",
                    CheckProperty(L"", L"Read access", staticValue(true), PropertyAttribute::READ_ONLY),
                    CheckProperty(L"", L"Write access", staticValue(true), PropertyAttribute::READ_ONLY)
                );
            }

            return CommonProperty(L"", L"",
                CheckProperty(L"", L"Read access", customValue<bool>(
                    [this, userKey]{ return hasAccess(userKey, TR::Access::READ_ACCESS); },
                    [this, userKey](bool value) { setAccess(userKey, TR::Access::READ_ACCESS, value); })
                ),
                CheckProperty(L"", L"Write access", customValue<bool>(
                    [this, userKey]{ return hasAccess(userKey, TR::Access::WRITE_ACCESS); },
                    [this, userKey](bool value) { setAccess(userKey, TR::Access::WRITE_ACCESS, value); })
                )           
            );
        };

        CheckPropertyEx checker(L"", L"Use default access rights", checkerExtension, customValue<bool>(
            [this, userKey] {
                return !hasAccessEntry(userKey);
            },
            [this, userKey](bool default) {
                if (default && hasAccessEntry(userKey))
                {
                    removeAccessEntry(userKey);
                }

                if (!default && !hasAccessEntry(userKey))
                {
                    cloneAccessEntry(userKey);
                }
            })
        );

        if (m_accessRightsView)
        {
            delete m_accessRightsView;
        }

        m_accessRightsView = addWidget(std::make_unique<CheckPropertyExView>(checker));
    }

    TR::AccessEntry* findAccessEntry(std::vector<TR::AccessEntry>& accessEntries, TR::Key userKey)
    {
        for (auto& accessEntry : accessEntries)
        {
            if (accessEntry.m_user_key == userKey)
            {
                return &accessEntry;
            }
        }
        return nullptr;
    }

    bool hasAccessEntry(TR::Key userKey)
    {
        return findAccessEntry(m_accessInfo->m_access_entries, userKey) != nullptr;
    }

    void cloneAccessEntry(TR::Key userKey)
    {
        _ASSERT(!hasAccessEntry(userKey));
        if (auto defaultAccessEntry = findAccessEntry(m_accessInfo->m_default_access_entries, userKey))
        {
            m_accessInfo->m_access_entries.push_back(*defaultAccessEntry);
        }
    }

    void removeAccessEntry(TR::Key userKey)
    {
        auto position = boost::find_if(m_accessInfo->m_access_entries, [&](TR::AccessEntry& accessEntry){
            return accessEntry.m_user_key == userKey;
        });

        _ASSERT(position != m_accessInfo->m_access_entries.end());
        m_accessInfo->m_access_entries.erase(position);
    }

    bool hasAccess(TR::Key userKey, TR::Access access)
    {
        auto accessEntry = findAccessEntry(m_accessInfo->m_access_entries, userKey);
        _ASSERT(accessEntry);
        return accessEntry->m_access_rights.contains(access);
    }

    void setAccess(TR::Key userKey, TR::Access access, bool value)
    {
        auto accessEntry = findAccessEntry(m_accessInfo->m_access_entries, userKey);
        _ASSERT(accessEntry);
        accessEntry->m_access_rights = value ? accessEntry->m_access_rights | access : accessEntry->m_access_rights % access;
    }

    bool hasDefaultAccess(TR::Key userKey, TR::Access access)
    {
        auto defaultAccessEntry = findAccessEntry(m_accessInfo->m_default_access_entries, userKey);
        _ASSERT(defaultAccessEntry);
        return defaultAccessEntry->m_access_rights.contains(access);
    }

    SuitSettingsWidget* m_suitSettingsWidget;
    Wt::WContainerWidget* m_accessRightsView;
    boost::optional<TR::SourceAccessInfo>& m_accessInfo;
    std::vector<TR::UserInfo> m_users;
};

class SuitSettingsWidget::GeneralPage: public CommonPropertyPage
{
public:
    enum class SettingsType { SECURITY_SETTINGS };

    GeneralPage(SuitSettingsWidget* suitSettingsWidget):
        CommonPropertyPage(L"General", L"Setup general source settings"),
        m_suitSettingsWidget(suitSettingsWidget),
        m_suitSettings(suitSettingsWidget->m_suitSettings)
    {
        show(CommonProperty(L"", L"",
            StringProperty(L"Name", L"Name Description", refValue(m_suitSettings.m_name), STRING_ATTRIBUTES(NOT_EMPTY)),            
            AnyProperty(L"Security", L"Setup user access to suit data", L"Edit", staticValue<boost::any>(SettingsType::SECURITY_SETTINGS))
        ));
    }

protected:
    virtual void handleEvent(boost::any any) override
    {
        switch (boost::any_cast<SettingsType>(any))
        {
        case SettingsType::SECURITY_SETTINGS:
            m_propertySheet->pushPage(std::make_unique<SecurityPage>(m_suitSettingsWidget));
            break;

        default:
            _ASSERT(false);
        }
    }

    SuitSettingsWidget* m_suitSettingsWidget;
    TR::SourceSettings& m_suitSettings;
};

SuitSettingsWidget::SuitSettingsWidget(TR::SourceKey suitKey, TR::SourceSettings suitSettings):
    m_suitKey(suitKey),
    m_suitSettings(suitSettings)
{
    init();
}

void SuitSettingsWidget::init()
{
    createToolBar();

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);

    auto suitPropertySheet = vbox->addWidget(std::make_unique<PropertySheetWidget>());
    suitPropertySheet->pushPage(std::make_unique<GeneralPage>(this));
}

void SuitSettingsWidget::setOnApply(OnApply onApply)
{
    _ASSERT(!m_onApply);
    m_onApply = onApply;
}

void SuitSettingsWidget::createToolBar()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());   

    auto btnOk = Wt::addButton<Wt::WPushButton>(*toolbar, "OK");
    btnOk->clicked().connect(std::bind(&SuitSettingsWidget::onOk, this));

    auto btnCancel = Wt::addButton<Wt::WPushButton>(*toolbar, "Cancel");    
    btnCancel->clicked().connect(std::bind(&SuitSettingsWidget::onCancel, this));
}

void SuitSettingsWidget::onOk()
{
    if (!validatePropertyPages(this))
    {
        return;
    }

    auto& client = Application::instance()->getClient();
    if (!m_suitKey)
    {
        m_suitKey = client.addSource(m_suitSettings);
    }
    else
    {
        client.updateSource(m_suitKey, m_suitSettings);
    }

    if (m_accessInfo)
    {
        m_accessInfo->m_source_key = m_suitKey;
        client.setAccessList(*m_accessInfo);
    }

    if (m_onApply)
    {
        m_onApply(m_suitKey);
    }

    Application::popPage();
}

void SuitSettingsWidget::onCancel()
{
    Application::popPage();
}

SuitSettingsPage::SuitSettingsPage(TR::SourceKey suitKey, TR::SourceSettings suitSettings):
    MainFramePage(suitKey ? suitSettings.m_name : L"New Suit", "/suit_settings/key=" + std::to_string(suitKey)),
    m_widget(suitKey, std::move(suitSettings))
{
    m_path = SourcesPage().getPath();
    m_path.push_back({m_caption, m_url});
}

SuitSettingsPage::SuitSettingsPage(TR::SourceSettings suitSettings):
    SuitSettingsPage(TR::SourceKey(0), suitSettings)
{
}

void SuitSettingsPage::setOnApply(OnApply onApply)
{
    m_widget.setOnApply(onApply);
}

MainFrameWidget* SuitSettingsPage::getWidget()
{
    return &m_widget;
}