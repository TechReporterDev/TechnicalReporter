#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "CurrentReportsTable.h"

class CurrentReportsWidget: public MainFrameWidget
{
public:
    CurrentReportsWidget(const TR::SourceInfo& sourceInfo);

protected:
    // AppEventHandle override
    virtual void onUpdateSource(const TR::SourceInfo& sourceInfo) override;
    
    void reset();
    void createToolBar();
    CurrentReportsTable* m_currentReportsTable;
    TR::SourceInfo m_sourceInfo;    
};

class CurrentReportsPage: public MainFramePage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    CurrentReportsPage(const TR::SourceInfo& sourceInfo);
    virtual MainFrameWidget* getWidget() override;

protected:
    TR::SourceInfo m_sourceInfo;
    std::unique_ptr<CurrentReportsWidget> m_widget;
};