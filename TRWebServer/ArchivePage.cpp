#include "stdafx.h"
#include "ArchivePage.h"
#include "SourcePage.h"
#include "ArchiveQueryDlg.h"
#include "Application.h"

ArchiveWidget::ArchiveWidget(const TR::SourceInfo& sourceInfo):
    m_sourceInfo(sourceInfo)
{
    createToolBar();

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);
    vbox->setSpacing(0);

    m_archiveTable = vbox->addWidget(std::make_unique<ArchiveTable>());
    for (auto& archivedReportInfo : m_client->getArchive(m_sourceInfo.m_key))
    {
        m_archiveTable->addArchivedReportInfo(archivedReportInfo);
    }
}

void ArchiveWidget::createToolBar()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());
    
    auto queryBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Query");
    queryBtn->clicked().connect([this](Wt::WMouseEvent){
        auto archiveQueryDlg = new ArchiveQueryDlg(m_sourceInfo);
        archiveQueryDlg->show();
    });
}

std::unique_ptr<MainFramePage> createPage(const std::string& url)
{
    boost::regex url_reg("/source/archive/key=(\\d+)");
    boost::smatch match;
    if (!boost::regex_match(url, match, url_reg))
    {
        return nullptr;
    }

    TR::SourceKey sourceKey(boost::lexical_cast<int>(match[1].str()));
    auto& client = Application::instance()->getClient();
    return std::make_unique<ArchivePage>(client.getSourceInfo(sourceKey));
}

ArchivePage::ArchivePage(const TR::SourceInfo& sourceInfo):
    MainFramePage(L"Archive", "/source/archive/key=" + std::to_string(sourceInfo.m_key)),
    m_sourceInfo(sourceInfo)
{
    m_path = SourcePage(sourceInfo).getPath();
    m_path.push_back({m_caption, m_url});
}

MainFrameWidget* ArchivePage::getWidget()
{
    if (!m_widget)
    {
        m_widget.reset(new ArchiveWidget(m_sourceInfo));
    }
    return m_widget.get();
}