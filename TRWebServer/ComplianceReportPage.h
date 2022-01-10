#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "CurrentReportPage.h"

class CurrentComplianceReportWidget: public CurrentReportWidget
{
public:
    CurrentComplianceReportWidget(const TR::CurrentReportInfo& currentReportInfo);
    CurrentComplianceReportWidget(const CurrentComplianceReportWidget&) = delete;
    CurrentComplianceReportWidget& operator = (const CurrentComplianceReportWidget&) = delete;

protected:
    // override CurrentReportWidget
    virtual void buildAdvancedMenu(Wt::WPopupMenu *advancedMenu) override;
    
    void onSetupClick();
};

class CurrentComplianceReportPage: public CurrentReportPage
{
public:
    CurrentComplianceReportPage(const TR::CurrentReportInfo& currentReportInfo, const TR::ReportTypeInfo& reportTypeInfo);
    virtual CurrentReportWidget* createWidget() override;
};