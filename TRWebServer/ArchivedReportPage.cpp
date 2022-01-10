#include "stdafx.h"
#include "ArchivedReportPage.h"
#include "ContentView.h"
#include "SourcePage.h"
#include "CompareContentPage.h"
#include "GetTimeDlg.h"
#include "Application.h"

ArchivedReportWidget::ArchivedReportWidget(const TR::ArchivedReportInfo& archivedReportInfo):
    m_archivedReportInfo(archivedReportInfo),
    m_treeBtn(nullptr),
    m_tableBtn(nullptr),
    m_contentView(nullptr)
{
    auto app = Application::instance();
    auto& client = app->getClient();
    m_reportTypeInfo = client.getReportTypeInfo(m_archivedReportInfo.m_report_type_uuid);

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);

    showContent();
    showTitleBar(); 
}

void ArchivedReportWidget::showTitleBar()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());   

    toolbar->addWidget(std::make_unique<Wt::WText>("Date: "));
    toolbar->addWidget(std::make_unique<Wt::WText>(Application::formatTime(m_archivedReportInfo.m_time)));

    if(auto viewBar = m_contentView->getViewBar())
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

void ArchivedReportWidget::showContent()
{
    auto& client = Application::instance()->getClient();
    m_content = client.getArchivedContent(m_archivedReportInfo.m_key);
    
    auto vbox = dynamic_cast<Wt::WVBoxLayout*>(layout());
    m_contentView = vbox->addWidget(createContentView(m_content));  
}

void ArchivedReportWidget::onCompareCurrentClick()
{
    Application::showPage(std::make_unique<CompareContentPage>(
        m_reportTypeInfo,
        std::make_unique<ArchivedContentQuery>(m_archivedReportInfo.m_source_key, m_archivedReportInfo.m_report_type_uuid, m_archivedReportInfo.m_time, m_content),
        std::make_unique<CurrentContentQuery>(m_client, m_archivedReportInfo.m_source_key, m_archivedReportInfo.m_report_type_uuid)));
}

void ArchivedReportWidget::onCompareArchivedClick()
{
    auto getTimeDlg = new GetTimeDlg("Archived time", this);
    getTimeDlg->show([this](time_t time){
        Application::showPage(std::make_unique<CompareContentPage>(
            m_reportTypeInfo,
            std::make_unique<ArchivedContentQuery>(m_archivedReportInfo.m_source_key, m_archivedReportInfo.m_report_type_uuid, m_archivedReportInfo.m_time, m_content),
            std::make_unique<ArchivedContentQuery>(m_client, m_archivedReportInfo.m_source_key, m_archivedReportInfo.m_report_type_uuid, time)));
    });
}

void ArchivedReportWidget::onComparePatternClick()
{
    Application::showPage(std::make_unique<CompareContentPage>(
        m_reportTypeInfo,
        std::make_unique<ArchivedContentQuery>(m_archivedReportInfo.m_source_key, m_archivedReportInfo.m_report_type_uuid, m_archivedReportInfo.m_time, m_content),
        std::make_unique<PatternQuery>(m_client, m_archivedReportInfo.m_source_key, m_archivedReportInfo.m_report_type_uuid)));
}

std::unique_ptr<MainFramePage> ArchivedReportPage::createPage(const std::string& url)
{
    boost::regex url_reg("/archived_report/key=(\\d+)");
    boost::smatch match;
    if (!boost::regex_match(url, match, url_reg))
    {
        return nullptr;
    }

    auto archivedReportKey = TR::ArchivedReportKey(boost::lexical_cast<int>(match[1].str()));
    auto& client = Application::instance()->getClient();
    return std::make_unique<ArchivedReportPage>(client.getArchivedReport(archivedReportKey));
}

ArchivedReportPage::ArchivedReportPage(const TR::ArchivedReportInfo& archivedReportInfo):
    MainFramePage(L"", "/archived_report/key=" + std::to_string(archivedReportInfo.m_key)),
    m_widget(archivedReportInfo)
{
    auto& client = Application::instance()->getClient();
    auto reportTypeInfo = client.getReportTypeInfo(archivedReportInfo.m_report_type_uuid);
    m_caption = reportTypeInfo.m_name;

    auto sourceInfo = client.getSourceInfo(archivedReportInfo.m_source_key);
    m_path = SourcePage(sourceInfo).getPath();
    m_path.push_back({m_caption, m_url});
}

MainFrameWidget* ArchivedReportPage::getWidget()
{
    return &m_widget;
}