#include "stdafx.h"
#include "CurrentReportsPage.h"
#include "CurrentReportPage.h"
#include "ArchivePage.h"
#include "SourcePage.h"
#include "Application.h"

CurrentReportsWidget::CurrentReportsWidget(const TR::SourceInfo& sourceInfo):
    m_sourceInfo(sourceInfo)
{
    createToolBar();

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);
    vbox->setSpacing(0);
    
    m_currentReportsTable = vbox->addWidget(std::make_unique<CurrentReportsTable>());
    reset();
}

void CurrentReportsWidget::onUpdateSource(const TR::SourceInfo& sourceInfo)
{
    if (m_sourceInfo.m_key == sourceInfo.m_key)
    {
        m_sourceInfo = sourceInfo;
        reset();
    }
}

void CurrentReportsWidget::reset()
{
    m_currentReportsTable->clear();
    for (auto& currentReportInfo : m_sourceInfo.m_current_reports)
    {
        m_currentReportsTable->addRow(currentReportInfo);
    }

    for (auto& streamInfo : m_sourceInfo.m_streams)
    {
        m_currentReportsTable->addRow(streamInfo);
    }
}

void CurrentReportsWidget::createToolBar()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());

    auto archiveBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Archive");
    archiveBtn->clicked().connect([this](Wt::WMouseEvent){
        Application::showPage(std::make_unique<ArchivePage>(m_sourceInfo));
    });
}

std::unique_ptr<MainFramePage> CurrentReportsPage::createPage(const std::string& url)
{
    boost::regex url_reg("/source/current_reports/key=(\\d+)");
    boost::smatch match;
    if (!boost::regex_match(url, match, url_reg))
    {
        return nullptr;
    }

    TR::SourceKey sourceKey(boost::lexical_cast<int>(match[1].str()));
    auto& client = Application::instance()->getClient();
    return std::make_unique<CurrentReportsPage>(client.getSourceInfo(sourceKey));
}

CurrentReportsPage::CurrentReportsPage(const TR::SourceInfo& sourceInfo):
    MainFramePage(L"Reports", "/source/current_reports/key=" + std::to_string(sourceInfo.m_key)),
    m_sourceInfo(sourceInfo)
{
    m_path = SourcePage(sourceInfo).getPath();
    m_path.push_back({m_caption, m_url});
}

MainFrameWidget* CurrentReportsPage::getWidget()
{
    if (!m_widget)
    {
        m_widget.reset(new CurrentReportsWidget(m_sourceInfo));
    }
    return m_widget.get();
}