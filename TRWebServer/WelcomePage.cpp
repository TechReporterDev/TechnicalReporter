#include "stdafx.h"
#include "WelcomePage.h"
#include "WelcomePanels.h"
#include "ExportSummaryDlg.h"
#include "Application.h"

WelcomeWidget::WelcomeWidget():
    m_summaryPanel(nullptr),
    m_messagePanel(nullptr),
    m_changesPanel(nullptr)
{
    for (auto& sourceInfo : m_client->getSourcesInfo())
    {
        m_sourcesInfo[sourceInfo.m_key] = sourceInfo;
    }

    for (auto& reportTypeInfo : m_client->getReportTypesInfo())
    {
        m_reportTypesInfo[reportTypeInfo.m_uuid] = reportTypeInfo;
    }

    for (auto& streamTypeInfo : m_client->getStreamTypesInfo())
    {
        m_streamTypesInfo[streamTypeInfo.m_uuid] = streamTypeInfo;
    }

    for (auto& actionInfo : m_client->getActionsInfo())
    {
        m_actionsInfo[actionInfo.m_uuid] = actionInfo;
        for (auto& shortcutInfo : actionInfo.m_shortcuts)
        {
            m_shortcutsInfo[shortcutInfo.m_uuid] = shortcutInfo;
        }
    }

    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());

    auto summaryBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Summary PDF");
    summaryBtn->clicked().connect([&](Wt::WMouseEvent) {
        auto exportSummaryDlg = new ExportSummaryDlg();
        exportSummaryDlg->show();
    });

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>()); 
    vbox->setContentsMargins(9, 0, 9, 9);
    vbox->setSpacing(8);
    
    /*auto top = vbox->addWidget(std::make_unique<Wt::WContainerWidget>());
    top->setHeight(100);*/

    auto hbox = vbox->addLayout(std::make_unique<Wt::WHBoxLayout>());
    hbox->setContentsMargins(0, 0, 0, 0);
    hbox->setSpacing(8);
    // panels   
    m_summaryPanel = hbox->addWidget(std::make_unique<SummaryPanel>());

    m_messagePanel = hbox->addWidget(std::make_unique<MessagePanel>(m_sourcesInfo, m_reportTypesInfo, m_actionsInfo, m_shortcutsInfo), 1);

    m_changesPanel = vbox->addWidget(std::make_unique<ChangesPanel>(m_sourcesInfo, m_reportTypesInfo, m_streamTypesInfo));
    m_changesPanel->setHeight(Wt::WLength(200, Wt::LengthUnit::Pixel));
}

void WelcomeWidget::onAddSource(const TR::SourceInfo& sourceInfo)
{
    m_sourcesInfo[sourceInfo.m_key] = sourceInfo;
}

void WelcomeWidget::onUpdateSource(const TR::SourceInfo& sourceInfo)
{
    m_sourcesInfo[sourceInfo.m_key] = sourceInfo;
}

void WelcomeWidget::onRemoveSource(TR::Key sourceKey, const std::vector<TR::SourceKey>& removedSources)
{
    for (auto sourceKey : removedSources)
    {
        m_sourcesInfo.erase(sourceKey);
    }
}

std::unique_ptr<MainFramePage> WelcomePage::createPage(const std::string& url)
{
    if (url == "/welcome")
    {
        return std::make_unique<WelcomePage>();
    }
    return nullptr;
}

WelcomePage::WelcomePage():
    MainFramePage(L"Welcome", "/welcome")
{
}

MainFrameWidget* WelcomePage::getWidget()
{
    return &m_widget;
}