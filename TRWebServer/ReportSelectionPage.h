#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class ReportTypeList;
class ReportSelectionWidget: public MainFrameWidget
{
public:
    ReportSelectionWidget();    
    void refresh();

private:
    void createToolBar();
    void onCreateClick();   

    ReportTypeList* m_reportTypeList;
};

class ReportSelectionPage: public MainFramePage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    ReportSelectionPage();
    virtual MainFrameWidget* getWidget() override;  

private:
    std::unique_ptr<ReportSelectionWidget> m_widget;
};