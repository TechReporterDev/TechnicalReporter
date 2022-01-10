#include "stdafx.h"
#include "QueriedReportPage.h"
#include "ArchivePage.h"
#include "ContentView.h"
#include "SourcePage.h"
#include "CompareContentPage.h"
#include "GetTimeDlg.h"
#include "Application.h"

QueriedReportWidget::QueriedReportWidget(const TR::SourceInfo& sourceInfo, const TR::ReportTypeInfo& reportTypeInfo, time_t time):
    m_sourceInfo(sourceInfo),
    m_reportTypeInfo(reportTypeInfo),
    m_time(time),
    m_contentView(nullptr)
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);

    setContentView(std::make_unique<EmptyContentView>(L"Loading..."));
    showTitleBar();
    queryContent();
}

void QueriedReportWidget::setContentView(std::unique_ptr<ContentView> contentView)
{   
    auto vbox = Wt::clear(this);
    m_contentView = contentView.get();
    vbox->addWidget(std::move(contentView));    
}

void QueriedReportWidget::showTitleBar()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());
    
    toolbar->addWidget(std::make_unique<Wt::WText>("Date: "));
    toolbar->addWidget(std::make_unique<Wt::WText>(Application::formatTime(m_time)));   

    if (auto viewBar = m_contentView->getViewBar())
    {
        toolbar->addSeparator();
        toolbar->addWidget(std::move(viewBar));
    }

    toolbar->addSeparator();

    auto compareBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Compare...");

    auto popup = std::make_unique<Wt::WPopupMenu>();
    popup->setAutoHide(true, 100);
    popup->setHideOnSelect(true);

    popup->addItem("Current")->clicked().connect([&](Wt::WMouseEvent){
        onCompareCurrentClick();
    });

    popup->addItem("Archived")->clicked().connect([&](Wt::WMouseEvent){
        onCompareArchivedClick();
    });

    popup->addItem("Pattern")->clicked().connect([&](Wt::WMouseEvent){
        onComparePatternClick();
    });

    compareBtn->setMenu(std::move(popup));
}

void QueriedReportWidget::queryContent()
{
    m_contentQuery = m_client->queryArchivedContent(m_sourceInfo.m_key, m_reportTypeInfo.m_uuid, m_time, 
        [this](std::shared_ptr<TR::Content> content){
            setContentView(createContentView(content));
        },
        [this](const std::string&){
            setContentView(std::make_unique<EmptyContentView>(L"Report is not available"));
        });
}

void QueriedReportWidget::onCompareCurrentClick()
{
    Application::showPage(std::make_unique<CompareContentPage>(
        m_reportTypeInfo,
        std::make_unique<ArchivedContentQuery>(m_client, m_sourceInfo.m_key, m_reportTypeInfo.m_uuid, m_time),
        std::make_unique<CurrentContentQuery>(m_client, m_sourceInfo.m_key, m_reportTypeInfo.m_uuid)));
}

void QueriedReportWidget::onCompareArchivedClick()
{
    auto getTimeDlg = new GetTimeDlg("Archived time", this);
    getTimeDlg->show([this](time_t time){
        Application::showPage(std::make_unique<CompareContentPage>(
            m_reportTypeInfo,
            std::make_unique<ArchivedContentQuery>(m_client, m_sourceInfo.m_key, m_reportTypeInfo.m_uuid, m_time),
            std::make_unique<ArchivedContentQuery>(m_client, m_sourceInfo.m_key, m_reportTypeInfo.m_uuid, time)));
    });
}

void QueriedReportWidget::onComparePatternClick()
{
    Application::showPage(std::make_unique<CompareContentPage>(
        m_reportTypeInfo,
        std::make_unique<ArchivedContentQuery>(m_client, m_sourceInfo.m_key, m_reportTypeInfo.m_uuid, m_time),
        std::make_unique<PatternQuery>(m_client, m_sourceInfo.m_key, m_reportTypeInfo.m_uuid)));
}

std::unique_ptr<MainFramePage> QueriedReportPage::createPage(const std::string& url)
{
    boost::regex url_reg("/queried_report/source_key=(\\d+)/report_type_uuid=(\\S+)/time=(\\d+)");
    boost::smatch match;
    if (!boost::regex_match(url, match, url_reg))
    {
        return nullptr;
    }

    TR::SourceKey sourceKey(boost::lexical_cast<int>(match[1].str()));
    TR::ReportTypeUUID reportTypeUUID(stl_tools::gen_uuid(match[2].str()));
    time_t time(boost::lexical_cast<time_t>(match[3].str()));

    auto& client = Application::instance()->getClient();
    return std::make_unique<QueriedReportPage>(client.getSourceInfo(sourceKey), client.getReportTypeInfo(reportTypeUUID), time);
}

static std::string getPageURL(const TR::SourceInfo& sourceInfo, const TR::ReportTypeInfo& reportTypeInfo, time_t time)
{
    return (boost::format("/queried_report/source_key=%1%/report_type_uuid=%2%/time=%3%")
        % sourceInfo.m_key
        % stl_tools::to_string(reportTypeInfo.m_uuid)
        % time).str();
}

QueriedReportPage::QueriedReportPage(const TR::SourceInfo& sourceInfo, const TR::ReportTypeInfo& reportTypeInfo, time_t time):
    MainFramePage(reportTypeInfo.m_name, getPageURL(sourceInfo, reportTypeInfo, time)),
    m_widget(sourceInfo, reportTypeInfo, time)
{
    m_path = ArchivePage(sourceInfo).getPath();
    m_path.push_back({getCaption(), getUrl()});
}

MainFrameWidget* QueriedReportPage::getWidget()
{
    return &m_widget;
}