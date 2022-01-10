#include "stdafx.h"
#include "ActionSelectionPage.h"
#include "ActionOptionsPage.h"
#include "TabWidget.h"
#include "Application.h"
#include "SubjectActionOptionsPage.h"
#include "InventoryTable.h"

class ActionSubjectOptions: public InventoryTable
{
public:
    enum SubjectType { SOURCE_SUBJECT, ROLE_SUBJECT };

    static SubjectType getSubjectType(TR::SubjectKey subject_key)
    {
        if (boost::get<TR::SourceKey>(&subject_key))
        {
            return SOURCE_SUBJECT;
        }
        return ROLE_SUBJECT;
    }

    ActionSubjectOptions(TR::ActionInfo actionInfo, boost::optional<TR::UUID> shortcutUUID, std::map<TR::SourceKey, TR::SourceInfo>& sourcesInfo, std::map<TR::RoleKey, TR::RoleInfo>& rolesInfo, SubjectType subjectType):
        m_actionInfo(actionInfo),
        m_shortcutUUID(shortcutUUID),
        m_sourcesInfo(sourcesInfo),
        m_rolesInfo(rolesInfo),
        m_subjectType(subjectType)
    {
        addTool("Setup", [this](int index){
            onSetupClick(index);
        }, true);
    }

    void refresh(const TR::ActionTypeOptions& actionOptions)
    {
        m_subjectOptions.clear();
        for (auto& subjectOptionsInfo : actionOptions.m_subject_options)
        {
            if (m_subjectType == getSubjectType(subjectOptionsInfo.m_subject_key))
            {
                m_subjectOptions.push_back(subjectOptionsInfo);
            }
        }

        resize(m_subjectOptions.size());
        for (size_t i = 0; i < m_subjectOptions.size(); ++i)
        {
            setItem(i, getSubjectName(m_subjectOptions[i].m_subject_key), "Subject options");
        }
    }

    void onSetupClick(int index)
    {
        Application::pushPage(std::make_unique<SubjectActionOptionsPage>(m_subjectOptions[index]));
    }

private:
    Wt::WString getSubjectName(TR::SubjectKey subjectKey)
    {
        if (auto sourceKey = boost::get<TR::SourceKey>(&subjectKey))
        {
            auto& sourceInfo = m_sourcesInfo[*sourceKey];
            return sourceInfo.m_name;
        }

        auto roleKey = boost::get<TR::RoleKey>(subjectKey);
        auto& roleInfo = m_rolesInfo[roleKey];
        return roleInfo.m_name;
    }

    TR::ActionInfo m_actionInfo;
    boost::optional<TR::UUID> m_shortcutUUID;

    SubjectType m_subjectType;
    std::vector<TR::SubjectActionOptions> m_subjectOptions;

    std::map<TR::SourceKey, TR::SourceInfo>& m_sourcesInfo;
    std::map<TR::RoleKey, TR::RoleInfo>& m_rolesInfo;
};

class ModifiedActionOptions: public InventoryTable
{
public:
    ModifiedActionOptions(TR::ActionInfo actionInfo, boost::optional<TR::UUID> shortcutUUID, std::map<TR::SourceKey, TR::SourceInfo>& sourcesInfo, std::map<TR::RoleKey, TR::RoleInfo>& rolesInfo):
        m_actionInfo(actionInfo),
        m_shortcutUUID(shortcutUUID),
        m_sourcesInfo(sourcesInfo),
        m_rolesInfo(rolesInfo)
    {
        addTool("Setup", [this](int index){
            onSetupClick(index);
        }, true);
    }

    void refresh(const TR::ActionTypeOptions& actionOptions)
    {
        m_modifiedOptions = TR::get_modified_options(actionOptions);

        resize(m_modifiedOptions.size());
        for (size_t i = 0; i < m_modifiedOptions.size(); ++i)
        {
            if (auto actionOptions = boost::get<TR::SubjectActionOptions>(&m_modifiedOptions[i]))
            {
                _ASSERT(actionOptions->m_shortcut_uuid);
                setItem(i, getSubjectName(actionOptions->m_subject_key), "Subject options");
            }
            else
            {
                _ASSERT(false);
            }
        }
    }

private:
    void onSetupClick(int index)
    {
        if (auto actionOptions = boost::get<TR::SubjectActionOptions>(&m_modifiedOptions[index]))
        {
            Application::pushPage(std::make_unique<SubjectActionOptionsPage>(*actionOptions));
        }
    }

    Wt::WString getSubjectName(TR::SubjectKey subjectKey)
    {
        if (auto sourceKey = boost::get<TR::SourceKey>(&subjectKey))
        {
            auto& sourceInfo = m_sourcesInfo[*sourceKey];
            return sourceInfo.m_name;
        }

        auto roleKey = boost::get<TR::RoleKey>(subjectKey);
        auto& roleInfo = m_rolesInfo[roleKey];
        return roleInfo.m_name;
    }

    TR::ActionInfo m_actionInfo;
    boost::optional<TR::UUID> m_shortcutUUID;
    TR::ModifiedOptions m_modifiedOptions;

    std::map<TR::SourceKey, TR::SourceInfo>& m_sourcesInfo;
    std::map<TR::RoleKey, TR::RoleInfo>& m_rolesInfo;
};

ActionOptionsWidget::ActionOptionsWidget(const TR::ActionInfo& actionInfo, boost::optional<TR::UUID> shortcutUUID):
    m_actionInfo(actionInfo),
    m_shortcutUUID(shortcutUUID),
    m_sourceOptions(nullptr),
    m_roleOptions(nullptr),
    m_modifiedOptions(nullptr)
{
    createToolBar();
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);
    auto tab = vbox->addWidget(std::make_unique<TabWidget>(), 1);

    for (auto& sourceInfo : m_client->getSourcesInfo())
    {
        m_sourcesInfo[TR::SourceKey(sourceInfo.m_key)] = sourceInfo;
    }

    for (auto& roleInfo : m_client->getRolesInfo())
    {
        m_rolesInfo[TR::RoleKey(roleInfo.m_key)] = roleInfo;
    }

    // sources tab
    auto sourcesTab = tab->addTab(std::make_unique<Wt::WContainerWidget>(), "Sources");
    auto sourcesLayout = sourcesTab->setLayout(std::make_unique<Wt::WVBoxLayout>());
    sourcesLayout->setContentsMargins(0, 5, 0, 0);
    m_sourceOptions = sourcesLayout->addWidget(std::make_unique<ActionSubjectOptions>(
        m_actionInfo, m_shortcutUUID, m_sourcesInfo, m_rolesInfo, ActionSubjectOptions::SOURCE_SUBJECT));

    // roles tab
    auto rolesTab = tab->addTab(std::make_unique<Wt::WContainerWidget>(), "Roles");
    auto rolesLayout = rolesTab->setLayout(std::make_unique<Wt::WVBoxLayout>());
    rolesLayout->setContentsMargins(0, 5, 0, 0);
    m_roleOptions = rolesLayout->addWidget(std::make_unique<ActionSubjectOptions>(
        m_actionInfo, m_shortcutUUID, m_sourcesInfo, m_rolesInfo, ActionSubjectOptions::ROLE_SUBJECT));
    
    // modified tab
    auto modifiedTab = tab->addTab(std::make_unique<Wt::WContainerWidget>(), "Modified");
    auto modifiedLayout = modifiedTab->setLayout(std::make_unique<Wt::WVBoxLayout>());
    modifiedLayout->setContentsMargins(0, 5, 0, 0);
    m_modifiedOptions = modifiedLayout->addWidget(std::make_unique<ModifiedActionOptions>(
        m_actionInfo, m_shortcutUUID, m_sourcesInfo, m_rolesInfo));
}

void ActionOptionsWidget::createToolBar()
{
}

void ActionOptionsWidget::load()
{
    auto actionOptions = m_client->getActionOptions(m_actionInfo.m_uuid, m_shortcutUUID);
    m_sourceOptions->refresh(actionOptions);
    m_roleOptions->refresh(actionOptions);
    m_modifiedOptions->refresh(actionOptions);
}

std::unique_ptr<MainFramePage> ActionOptionsPage::createPage(const std::string& url)
{
    boost::regex url_reg("/options/actions/uuid=(\\S+?)(,shortcut=(\\S+?))?");
    boost::smatch match;
    if (!boost::regex_match(url, match, url_reg))
    {
        return nullptr;
    }

    TR::ActionUUID actionUUID(stl_tools::gen_uuid(match[1].str()));
    boost::optional<TR::UUID> shortcutUUID;
    if (match[3].matched)
    {
        shortcutUUID = stl_tools::gen_uuid(match[3].str());
    }

    auto& client = Application::instance()->getClient();
    return std::make_unique<ActionOptionsPage>(client.getActionInfo(actionUUID), shortcutUUID);
}

ActionOptionsPage::ActionOptionsPage(const TR::ActionInfo& actionInfo, boost::optional<TR::UUID> shortcutUUID):
    MainFramePage(actionInfo.m_name, "/options/actions/uuid=" + stl_tools::to_string(actionInfo.m_uuid)),
    m_actionInfo(actionInfo),
    m_shortcutUUID(shortcutUUID)
{
    if (shortcutUUID)
    {
        m_url += ",shortcut=" + stl_tools::to_string(*shortcutUUID);
    }
    m_path = ActionSelectionPage().getPath();
    m_path.push_back({m_caption, m_url});
}

MainFrameWidget* ActionOptionsPage::getWidget()
{
    if (!m_widget)
    {
        m_widget.reset(new ActionOptionsWidget(m_actionInfo, m_shortcutUUID));
    }
    return m_widget.get();
}