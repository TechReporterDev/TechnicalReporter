#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "ContentView.h"

class QueriedReportWidget: public MainFrameWidget
{
public:
    QueriedReportWidget(const TR::SourceInfo& sourceInfo, const TR::ReportTypeInfo& reportTypeInfo, time_t time);

protected:
    void    setContentView(std::unique_ptr<ContentView> contentView);
    void    showTitleBar(); 
    void    queryContent();
    
    void    onCompareCurrentClick();
    void    onCompareArchivedClick();
    void    onComparePatternClick();

    TR::SourceInfo m_sourceInfo;
    TR::ReportTypeInfo m_reportTypeInfo;
    time_t m_time;
    ContentView* m_contentView;
    TR::Client::Job m_contentQuery;
};

class QueriedReportPage: public MainFramePage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    QueriedReportPage(const TR::SourceInfo& sourceInfo, const TR::ReportTypeInfo& reportTypeInfo, time_t time);
    virtual MainFrameWidget* getWidget() override;

protected:
    QueriedReportWidget m_widget;
};