#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "ContentView.h"

class ArchivedReportWidget: public MainFrameWidget
{
public:
    ArchivedReportWidget(const TR::ArchivedReportInfo& archivedReportInfo);

protected:
    void    showTitleBar();
    void    showContent();

    void    onCompareCurrentClick();
    void    onCompareArchivedClick();
    void    onComparePatternClick();

    TR::ArchivedReportInfo m_archivedReportInfo;
    TR::ReportTypeInfo m_reportTypeInfo;
    Wt::WPushButton* m_treeBtn;
    Wt::WPushButton* m_tableBtn;    
    ContentView* m_contentView;
    std::shared_ptr<TR::Content> m_content;
};

class ArchivedReportPage: public MainFramePage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    ArchivedReportPage(const TR::ArchivedReportInfo& archivedReportInfo);
    virtual MainFrameWidget* getWidget() override;

protected:
    ArchivedReportWidget m_widget;
};