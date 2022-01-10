#include "stdafx.h"
#include "SourceSettingsPage.h"
#include "SourcesPage.h"
#include "CommonPropertyPage.h"
#include "CommonPropertyView.h"
#include "RefValue.h"
#include "CustomValue.h"
#include "StaticValue.h"
#include "ContainerArray.h"
#include "CommonPropertyDoc.h"
#include "SourceBrowseDlg.h"
#include "TRCore\SuitsFeature.h"
#include "Application.h"

class SourceSettingsWidget::AdvancedPage: public CommonPropertyPage
{
public:
    AdvancedPage(SourceSettingsWidget* sourceSettingsWidget):
        CommonPropertyPage(L"Advanced settings", L"Setup advanced source settings"),
        m_sourceSettingsWidget(sourceSettingsWidget),
        m_sourceConfig(sourceSettingsWidget->m_sourceConfig)
    {
        if (!m_sourceConfig)
        {
            initializeConfig();
        }

        show(createProperty(*m_sourceConfig->m_config));
    }

    void initializeConfig()
    {
        auto& client = Application::instance()->getClient();
        if (auto sourceKey = m_sourceSettingsWidget->m_sourceKey)
        {
            auto settings = client.getSourceInfo(sourceKey);
            if (settings.m_source_type_uuid == m_sourceSettingsWidget->m_sourceSettings.m_source_type_uuid)
            {
                m_sourceConfig = client.getSourceConfig(sourceKey);
                return;
            }
        }

        m_sourceConfig = client.getDefaultConfig(m_sourceSettingsWidget->m_sourceSettings.m_source_type_uuid);      
    }

    SourceSettingsWidget* m_sourceSettingsWidget;
    boost::optional<TR::SourceConfig>& m_sourceConfig;
};

class SourceSettingsWidget::SecurityPage: public PropertyPageWidget
{
public:
    SecurityPage(SourceSettingsWidget* sourceSettingsWidget):
        PropertyPageWidget(L"Security", L"Source security settings", 0),
        m_sourceSettingsWidget(sourceSettingsWidget),
        m_accessRightsView(nullptr),
        m_accessInfo(sourceSettingsWidget->m_accessInfo)
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
            if (auto sourceKey = m_sourceSettingsWidget->m_sourceKey)
            {
                m_accessInfo = client.getAccessInfo(TR::SourceKey(sourceKey));
                return;
            }
            m_accessInfo = TR::SourceAccessInfo(TR::SourceKey{}, {}, {});
        }

        if (m_sourceSettingsWidget->m_sourceSettings.m_parent_key)
        {
            m_accessInfo->m_default_access_entries = client.getAccessEntries(m_sourceSettingsWidget->m_sourceSettings.m_parent_key);
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
                    CheckProperty(L"", L"Read access", staticValue(hasDefaultAccess(userKey, TR::Access::READ_ACCESS)), PropertyAttribute::READ_ONLY),
                    CheckProperty(L"", L"Write access", staticValue(hasDefaultAccess(userKey, TR::Access::WRITE_ACCESS)), PropertyAttribute::READ_ONLY),
                    CheckProperty(L"", L"Execute access", staticValue(hasDefaultAccess(userKey, TR::Access::EXEC_ACCESS)), PropertyAttribute::READ_ONLY)
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
                ),
                CheckProperty(L"", L"Execute access", customValue<bool>(
                    [this, userKey]{ return hasAccess(userKey, TR::Access::EXEC_ACCESS); },
                    [this, userKey](bool value) { setAccess(userKey, TR::Access::EXEC_ACCESS, value); })
                )
            );
        };

        CheckPropertyEx checker(L"", L"Inherit access rights", checkerExtension, customValue<bool>(
            [this, userKey] {
                return ! hasAccessEntry(userKey);
            },
            [this, userKey](bool inherit)   {
                if (inherit && hasAccessEntry(userKey))
                {
                    removeAccessEntry(userKey);
                }

                if (!inherit && !hasAccessEntry(userKey))
                {
                    cloneAccessEntry(userKey);
                }
            })
        );      

        if (m_accessRightsView)
        {
            removeWidget(m_accessRightsView);
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

    SourceSettingsWidget* m_sourceSettingsWidget;
    Wt::WContainerWidget* m_accessRightsView;
    boost::optional<TR::SourceAccessInfo>& m_accessInfo;
    std::vector<TR::UserInfo> m_users;
};

class SourceSettingsWidget::GeneralPage: public CommonPropertyPage
{
public:
    enum class SettingsType {ADVANCED_SETTINGS, SECURITY_SETTINGS, PARENT_SETTINGS};

    GeneralPage(SourceSettingsWidget* sourceSettingsWidget):
        CommonPropertyPage(L"General", L"Setup general source settings"),
        m_sourceSettingsWidget(sourceSettingsWidget),
        m_sourceSettings(sourceSettingsWidget->m_sourceSettings)
    {
        auto& client = Application::instance()->getClient();
        for(auto& sourceTypeInfo: client.getSourceTypesInfo())
        {
            if(sourceTypeInfo.m_uuid != TR::Core::SuitRootSourceType::SOURCE_TYPE_UUID && sourceTypeInfo.m_uuid != TR::Core::SuitSourceType::SOURCE_TYPE_UUID)
            {
                m_sourceTypesInfo.push_back(sourceTypeInfo);
            }
        }

        m_rolesInfo = client.getRolesInfo();

        show(CommonProperty(L"", L"", 
            StringProperty(L"Name", L"Name Description", refValue(m_sourceSettings.m_name), STRING_ATTRIBUTES(NOT_EMPTY)),
            BrowseProperty(L"Parent", L"Select parent source", L"Browse...", boost::any(SettingsType::PARENT_SETTINGS), 
                [this, &client](boost::any){
                    if (m_sourceSettings.m_parent_key == TR::INVALID_SOURCE_KEY)
                    {
                        return std::wstring(L"Root has no parent");
                    }
                    return client.getSourceInfo(m_sourceSettings.m_parent_key).m_name;
                }, m_sourceSettings.m_parent_key == TR::INVALID_SOURCE_KEY ? DEFAULT_PROPERTY_ATTRIBUTES | PropertyAttribute::READ_ONLY : DEFAULT_PROPERTY_ATTRIBUTES
            ),
            SelectorProperty(L"Role", L"Role Description", getRolesNames(), customValue<int>(
                [this](){ return getRole(); },
                [this](int index){ setRole(index); })
            ),
            SelectorPropertyEx(L"Source Type", L"Source Type", L"Settings", 
                [this](int){ return boost::any(SettingsType::ADVANCED_SETTINGS); },
                getSourceTypeNames(),
                customValue<int>(
                    [this](){ return getSourceType(); },
                    [this](int index){ setSourceType(index); }
                ),
                m_sourceSettingsWidget->m_sourceKey ? PropertyAttribute::READ_ONLY : PropertyAttribute::DEFAULT
            ),
            AnyProperty(L"Security", L"Setup user access to source data", L"Edit", staticValue<boost::any>(SettingsType::SECURITY_SETTINGS))
        ));
    }

protected:
    virtual void handleEvent(boost::any any) override
    {
        switch (boost::any_cast<SettingsType>(any))
        {
        case SettingsType::ADVANCED_SETTINGS:
            m_propertySheet->pushPage(std::make_unique<AdvancedPage>(m_sourceSettingsWidget));
            break;
        
        case SettingsType::SECURITY_SETTINGS:
            m_propertySheet->pushPage(std::make_unique<SecurityPage>(m_sourceSettingsWidget));
            break;

        case SettingsType::PARENT_SETTINGS:
        {
            auto browseDlg = new SourceBrowseDlg(m_sourceSettingsWidget->m_sourceSettings.m_parent_key, m_sourceSettingsWidget->m_sourceKey);
            browseDlg->connectApply([this](const TR::SourceInfo& parentSourceInfo){ 
                m_sourceSettingsWidget->m_sourceSettings.m_parent_key = parentSourceInfo.m_key;
                refresh(); 
            });
            browseDlg->show();
            break;
        }           

        default:
            _ASSERT(false);
        }
    }

    std::vector<std::wstring> getSourceTypeNames() const
    {
        return stl_tools::copy_vector(m_sourceTypesInfo | stl_tools::members(&TR::SourceTypeInfo::m_name));
    }

    void setSourceType(int index)
    {
        m_sourceSettings.m_source_type_uuid = m_sourceTypesInfo[index].m_uuid;
        m_sourceSettingsWidget->m_sourceConfig.reset();
    }

    int getSourceType() const
    {
        auto found = boost::find(m_sourceTypesInfo | stl_tools::members(&TR::SourceTypeInfo::m_uuid), m_sourceSettings.m_source_type_uuid);
        if(found.base() == m_sourceTypesInfo.end())
        {
            throw std::logic_error("SourceType not found");
        }

        return found.base() - m_sourceTypesInfo.begin();
    }

    std::vector<std::wstring> getRolesNames() const
    {
        return stl_tools::copy_vector(m_rolesInfo | stl_tools::members(&TR::RoleInfo::m_name));
    }

    void setRole(int index)
    {
        m_sourceSettings.m_role_key = m_rolesInfo[index].m_key;
        m_sourceSettingsWidget->m_sourceConfig.reset();
    }

    int getRole() const
    {
        auto found = boost::find(m_rolesInfo | stl_tools::members(&TR::RoleInfo::m_key), m_sourceSettings.m_role_key);
        if (found.base() == m_rolesInfo.end())
        {
            throw std::logic_error("Role not found");
        }

        return found.base() - m_rolesInfo.begin();
    }

    std::vector<TR::SourceTypeInfo> m_sourceTypesInfo;
    std::vector<TR::RoleInfo> m_rolesInfo;

    SourceSettingsWidget* m_sourceSettingsWidget;
    TR::SourceSettings& m_sourceSettings;
};

SourceSettingsWidget::SourceSettingsWidget(TR::SourceKey sourceKey, TR::SourceSettings sourceSettings):
    m_sourceKey(sourceKey),
    m_sourceSettings(sourceSettings)
{
    init();
}

void SourceSettingsWidget::init()
{
    createToolBar();

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);

    auto sourcePropertySheet = vbox->addWidget(std::make_unique<PropertySheetWidget>());
    sourcePropertySheet->pushPage(std::make_unique<GeneralPage>(this));
}

void SourceSettingsWidget::setOnApply(OnApply onApply)
{
    _ASSERT(!m_onApply);
    m_onApply = onApply;
}

void SourceSettingsWidget::createToolBar()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());
    
    auto btnOk = Wt::addButton<Wt::WPushButton>(*toolbar, "OK");    
    btnOk->clicked().connect(std::bind(&SourceSettingsWidget::onOk, this));

    auto btnCancel = Wt::addButton<Wt::WPushButton>(*toolbar, "Cancel");    
    btnCancel->clicked().connect(std::bind(&SourceSettingsWidget::onCancel, this));
}

void SourceSettingsWidget::onOk()
{
    if (!validatePropertyPages(this))
    {
        return;
    }

    auto& client = Application::instance()->getClient();
    if (!m_sourceKey)
    {
        m_sourceKey = client.addSource(m_sourceSettings);
    }
    else
    {
        client.updateSource(m_sourceKey, m_sourceSettings);
    }

    if (m_sourceConfig)
    {
        client.setSourceConfig(m_sourceKey, {xml_tools::clone_doc(*m_sourceConfig->m_config), m_sourceConfig->m_config_def});
    }

    if (m_accessInfo)
    {
        m_accessInfo->m_source_key = m_sourceKey;
        client.setAccessList(*m_accessInfo);
    }

    if (m_onApply)
    {
        m_onApply(m_sourceKey);
    }

    Application::popPage();
}

void SourceSettingsWidget::onCancel()
{
    Application::popPage();
}

SourceSettingsPage::SourceSettingsPage(TR::SourceKey sourceKey, TR::SourceSettings sourceSettings):
    MainFramePage(sourceKey ? sourceSettings.m_name : L"New Source", "/source_settings/key=" + std::to_string(sourceKey)),
    m_widget(sourceKey, std::move(sourceSettings))
{
    m_path = SourcesPage().getPath();
    m_path.push_back({m_caption, m_url});
}

SourceSettingsPage::SourceSettingsPage(TR::SourceSettings sourceSettings):
    SourceSettingsPage(TR::SourceKey(0), sourceSettings)
{
}

void SourceSettingsPage::setOnApply(OnApply onApply)
{
    m_widget.setOnApply(onApply);
}

MainFrameWidget* SourceSettingsPage::getWidget()
{
    return &m_widget;
}