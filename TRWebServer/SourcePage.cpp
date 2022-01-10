#include "stdafx.h"
#include "SourcePage.h"
#include "SourcePanel.h"
#include "CurrentReportPage.h"
#include "RegularReportPage.h"
#include "PatternComparePage.h"
#include "SourcesPage.h"
#include "ActionDlg.h"
#include "ShortcutDlg.h"
#include "PropertyBox.h"
#include "ReloadDlg.h"
#include "SourceSettingsPage.h"
#include "Application.h"

SourceWidget::SourceWidget(TR::SourceKey sourceKey):
    m_infoPanel(nullptr),
    m_alertsPanel(nullptr),
    m_currentReportPanel(nullptr),
    m_sourceInfo(m_client->getSourceInfo(sourceKey))
{
    createToolBar();

    auto hbox = setLayout(std::make_unique<Wt::WHBoxLayout>());
    hbox->setContentsMargins(9, 0, 9, 9);
    hbox->setSpacing(8);

    auto vbox = hbox->addLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(8);    

    // panels
    m_infoPanel = vbox->addWidget(std::make_unique<SourceInfoPanel>(m_sourceInfo));
    m_alertsPanel = vbox->addWidget(std::make_unique<AlertsPanel>(m_sourceInfo));

    m_currentReportPanel = hbox->addWidget(std::make_unique<CurrentReportPanel>(m_sourceInfo)); 
    m_currentReportPanel->setWidth({ 50, Wt::LengthUnit::Percentage });
}

void SourceWidget::createToolBar()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());
    
    auto actionBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Action");
    actionBtn->setMenu(std::make_unique<Wt::WPopupMenu>());
    auto actionMenu = actionBtn->menu();
    actionMenu->setAutoHide(true, 100);
    actionMenu->setHideOnSelect(true);
    createActionMenu(actionMenu);
}

void SourceWidget::createActionMenu(Wt::WPopupMenu* actionPopup)
{
    if (!actionPopup->items().empty())
    {
        return;
    }

    auto sourceTypeInfo = m_client->getSourceTypeInfo(m_sourceInfo.m_source_type_uuid);

    auto reloadMenu = std::make_unique<Wt::WPopupMenu>();   
    reloadMenu->addItem("All...")->triggered().connect(this, &SourceWidget::onReloadClick);
    reloadMenu->addSeparator();

    auto reportTypes = stl_tools::copy_map(m_client->getReportTypesInfo(), &TR::ReportTypeInfo::m_uuid);
    for (auto& download_uuid : sourceTypeInfo.m_download_uuids)
    {
        auto reportTypeInfo = reportTypes[download_uuid];
        reloadMenu->addItem(reportTypeInfo.m_name)->clicked().connect(std::bind(&SourceWidget::onReloadReportClick, this, reportTypeInfo));
    }

    actionPopup->addMenu("Reload", std::move(reloadMenu));

    if (!sourceTypeInfo.m_action_uuids.empty())
    {
        actionPopup->addSeparator();

        auto actions = stl_tools::copy_map(m_client->getActionsInfo(), &TR::ActionInfo::m_uuid);
        for (auto& action_uuid : sourceTypeInfo.m_action_uuids)
        {
            auto actionInfo = actions[TR::ActionUUID(action_uuid)];

            auto actionMenu = std::make_unique<Wt::WPopupMenu>();
            actionMenu->addItem("Run...")->clicked().connect(std::bind(&SourceWidget::onActionClick, this, actionInfo));
            if (!actionInfo.m_shortcuts.empty())
            {
                actionMenu->addSeparator();
                for (auto& shortcutInfo : actionInfo.m_shortcuts)
                {
                    actionMenu->addItem(shortcutInfo.m_name)->clicked().connect(std::bind(&SourceWidget::onShortcutClick, this, shortcutInfo));
                }
            }

            actionPopup->addMenu(actionInfo.m_name, std::move(actionMenu));
        }
    }
}

void SourceWidget::load()
{
    //createToolBar();
}

void SourceWidget::onReloadClick()
{
    auto reloadQuestion = new PropertyBox(stl_tools::flag | Dialog::BTN_OK | Dialog::BTN_CANCEL, L"Reload all reports", L"Do you really want to reload reports?", Wt::Icon::Question);
    reloadQuestion->show([this]{
        auto reloadAllDlg = new ReloadAllDlg(m_sourceInfo);
        reloadAllDlg->show();
    });
}

void SourceWidget::onReloadReportClick(TR::ReportTypeInfo reportTypeInfo)
{
    auto reloadQuestion = new PropertyBox(stl_tools::flag | Dialog::BTN_OK | Dialog::BTN_CANCEL, L"Reload", L"Do you want to reload " + reportTypeInfo.m_name + L"?", Wt::Icon::Question);
    reloadQuestion->show([this, reportTypeInfo]{
        auto reloadDlg = new ReloadDlg(m_sourceInfo, reportTypeInfo);
        reloadDlg->show();
    });
}

void SourceWidget::onActionClick(TR::ActionInfo actionInfo)
{
    auto launchDlg = new LaunchDlg(m_sourceInfo, actionInfo, m_client->getDefaultParams(actionInfo.m_uuid));
    launchDlg->show();
}

void SourceWidget::onShortcutClick(TR::ActionShortcutInfo shortcutInfo)
{
    auto launchQuestion = new PropertyBox(stl_tools::flag | Dialog::BTN_OK | Dialog::BTN_CANCEL, L"Launch shortcut", L"Do you want to launch shortcut " + shortcutInfo.m_name + L"?", Wt::Icon::Question);
    launchQuestion->show([this, shortcutInfo]{
        auto shortcutDlg = new ShortcutDlg(m_sourceInfo, shortcutInfo);
        shortcutDlg->show();
    });
}

std::unique_ptr<MainFramePage> SourcePage::createPage(const std::string& url)
{
    boost::regex url_reg("/sources/key=(\\d+)" );
    boost::smatch match;
    if ( ! boost::regex_match(url, match, url_reg))
    {
        return nullptr;
    }

    TR::SourceKey sourceKey(boost::lexical_cast<int>(match[1].str()));
    auto& client = Application::instance()->getClient();
    return std::make_unique<SourcePage>(client.getSourceInfo(sourceKey));   
}

SourcePage::SourcePage(TR::SourceInfo sourceInfo):
    MainFramePage(sourceInfo.m_name, "/sources/key=" + std::to_string(sourceInfo.m_key)),
    m_sourceInfo(sourceInfo)
{
    auto& client = Application::instance()->getClient();
    if (sourceInfo.m_parent_key)
    {       
        auto parentInfo = client.getSourceInfo(sourceInfo.m_parent_key);
        m_path = SourcePage(parentInfo).getPath();
    }
    else
    {
        m_path = SourcesPage().getPath();
    }
    m_path.push_back({m_caption, m_url});
}

MainFrameWidget* SourcePage::getWidget()
{
    if (!m_widget)
    {
        m_widget.reset(new SourceWidget(m_sourceInfo.m_key));
    }
    return m_widget.get();
}