#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "CurrentReportPage.h"

class CurrentValidatedReportWidget: public CurrentReportWidget
{
public:
    CurrentValidatedReportWidget(const TR::CurrentReportInfo& currentReportInfo);
    CurrentValidatedReportWidget(const CurrentValidatedReportWidget&) = delete;

protected:
    // override CurrentReportWidget
    virtual void buildAdvancedMenu(Wt::WPopupMenu *advancedMenu) override;
    
    void onSetupClick();
};

class CurrentValidatedReportPage: public CurrentReportPage
{
public:
    CurrentValidatedReportPage(const TR::CurrentReportInfo& currentReportInfo);
    virtual CurrentReportWidget* createWidget() override;
};