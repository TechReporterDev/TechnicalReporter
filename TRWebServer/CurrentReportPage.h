#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "ContentView.h"

class CurrentReportWidget: public MainFrameWidget
{
public:
    CurrentReportWidget(const TR::CurrentReportInfo& currentReportInfo);

protected:      
    void                        showTitleBar();
    void                        showContent(std::shared_ptr<TR::Content> content);
    
    void                        setContentView(std::unique_ptr<ContentView> contentWidget);
    ContentView*                getContentView();
    bool                        emptyContentView();

    void                        onCompareArchivedClick();
    void                        onComparePatternClick();
    void                        onSendPatternClick();
    void                        onSendFileClick();
    void                        onReloadClick();

    void                        setCurrentReportInfo(const TR::CurrentReportInfo& currentReportInfo);
    void                        queryContent();


    // AppEventHandler override 
    virtual void                onUpdateCurrentReport(const TR::CurrentReportInfo& current_report_info, bool content_changed) override;

    // customization
    virtual std::unique_ptr<Wt::WToolBar>       getControlBar();
    virtual std::unique_ptr<Wt::WPushButton>    getAdvancedButton();
    virtual void                                buildAdvancedMenu(Wt::WPopupMenu *advancedMenu);
    virtual std::unique_ptr<ContentView>        createContentView(std::shared_ptr<TR::Content> content);
    virtual void                                updateContent(std::shared_ptr<TR::Content> content);

    TR::CurrentReportInfo m_currentReportInfo;  
    TR::Client::Job m_contentQuery;
    std::shared_ptr<TR::Content> m_content;
};

class CurrentReportPage: public MainFramePage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    CurrentReportPage(const TR::CurrentReportInfo& currentReportInfo);
    virtual MainFrameWidget*        getWidget() override;
    virtual CurrentReportWidget*    createWidget();

protected:
    TR::CurrentReportInfo m_currentReportInfo;
    std::unique_ptr<CurrentReportWidget> m_widget;
};

std::unique_ptr<CurrentReportPage> createCurrentReportPage(const TR::CurrentReportInfo& currentReportInfo);