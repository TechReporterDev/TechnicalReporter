#include "stdafx.h"
#include "ComplianceReportPage.h"
#include "ComplianceContentView.h"
#include "ComplianceSetupPage.h"
#include "Application.h"

CurrentComplianceReportWidget::CurrentComplianceReportWidget(const TR::CurrentReportInfo& currentReportInfo):
    CurrentReportWidget(currentReportInfo)
{
}

void CurrentComplianceReportWidget::buildAdvancedMenu(Wt::WPopupMenu *advancedMenu)
{
    if (!m_content)
    {
        CurrentReportWidget::buildAdvancedMenu(advancedMenu);
        return;
    }

    auto setupItem = advancedMenu->addItem("Setup...");
    setupItem->clicked().connect([&](Wt::WMouseEvent){
        onSetupClick();
    });

    auto sendItem = advancedMenu->addItem("Send to");
    auto sendMenu = std::make_unique<Wt::WPopupMenu>();
    sendMenu->addItem("File")->clicked().connect([&](Wt::WMouseEvent){
        onSendFileClick();
    });
    sendItem->setMenu(std::move(sendMenu));

    auto reloadItem = advancedMenu->addItem("Reload");
    reloadItem->clicked().connect([&](Wt::WMouseEvent){
        onReloadClick();
    });
}

void CurrentComplianceReportWidget::onSetupClick()
{
    auto reportTypeInfo = m_client->getReportTypeInfo(m_currentReportInfo.m_report_type_uuid);
    auto complianceInfo = m_client->getComplianceInfo(reportTypeInfo.m_compliance_key);
    auto sourceInfo = m_client->getSourceInfo(m_currentReportInfo.m_source_key);

    Application::pushPage(std::make_unique<ComplianceSetupPage>(sourceInfo, complianceInfo));
}

CurrentComplianceReportPage::CurrentComplianceReportPage(const TR::CurrentReportInfo& currentReportInfo, const TR::ReportTypeInfo& reportTypeInfo):
    CurrentReportPage(currentReportInfo)
{
}

CurrentReportWidget* CurrentComplianceReportPage::createWidget()
{
    return new CurrentComplianceReportWidget(m_currentReportInfo);
}