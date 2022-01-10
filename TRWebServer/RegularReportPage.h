#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "CurrentReportPage.h"

class CurrentRegularReportWidget: public CurrentReportWidget
{
public:
    CurrentRegularReportWidget(const TR::CurrentReportInfo& currentReportInfo);
    CurrentRegularReportWidget(const CurrentRegularReportWidget&) = delete;

protected:
    virtual void                            buildAdvancedMenu(Wt::WPopupMenu *advancedMenu) override;
    virtual std::unique_ptr<ContentView>    createContentView(std::shared_ptr<TR::Content> content) override;

    void onFiltrateClick();
    void onQueryClick();
    void onValidateClick();
};

class CurrentRegularReportPage: public CurrentReportPage
{
public:
    CurrentRegularReportPage(const TR::CurrentReportInfo& currentReportInfo);
    virtual CurrentReportWidget* createWidget() override;
};