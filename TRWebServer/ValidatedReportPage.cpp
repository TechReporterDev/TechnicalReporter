#include "stdafx.h"
#include "ValidatedReportPage.h"
#include "ValidatedContentView.h"
#include "ValidationSettingsPage.h"
#include "Application.h"

CurrentValidatedReportWidget::CurrentValidatedReportWidget(const TR::CurrentReportInfo& currentReportInfo):
    CurrentReportWidget(currentReportInfo)
{   
}

void CurrentValidatedReportWidget::buildAdvancedMenu(Wt::WPopupMenu *advancedMenu)
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
    sendMenu->addItem("File")->clicked().connect([&](Wt::WMouseEvent) {
        onSendFileClick();
    });
    sendItem->setMenu(std::move(sendMenu)); 

    auto reloadItem = advancedMenu->addItem("Reload");
    reloadItem->clicked().connect([&](Wt::WMouseEvent){
        onReloadClick();
    });
}

void CurrentValidatedReportWidget::onSetupClick()
{
    auto reportTypeInfo = m_client->getReportTypeInfo(m_currentReportInfo.m_report_type_uuid);
    auto validationInfo = m_client->getValidationInfo(reportTypeInfo.m_validation_key);
    auto sourceInfo = m_client->getSourceInfo(m_currentReportInfo.m_source_key);    
    Application::pushPage(std::make_unique<ModifyValidationPage>(std::move(sourceInfo), std::move(validationInfo)));
}

CurrentValidatedReportPage::CurrentValidatedReportPage(const TR::CurrentReportInfo& currentReportInfo):
    CurrentReportPage(currentReportInfo)
{
}

CurrentReportWidget* CurrentValidatedReportPage::createWidget()
{
    return new CurrentValidatedReportWidget(m_currentReportInfo);
}