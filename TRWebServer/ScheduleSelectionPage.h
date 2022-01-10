#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class ScheduleList;
class ScheduleSelectionWidget: public MainFrameWidget
{
public:
    ScheduleSelectionWidget();
    void createToolBar();

private:
    ScheduleList* m_scheduleList;
};

class ScheduleSelectionPage: public MainFramePage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    ScheduleSelectionPage();
    virtual MainFrameWidget*    getWidget() override;

private:
    std::unique_ptr<ScheduleSelectionWidget> m_widget;
};