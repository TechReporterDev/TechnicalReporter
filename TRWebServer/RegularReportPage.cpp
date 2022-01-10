#include "stdafx.h"
#include "RegularReportPage.h"
#include "RegularContentView.h"
#include "ValidatedContentView.h"
#include "FilterSetupPage.h"
#include "CreateQueryPage.h"
#include "ValidationSettingsPage.h"
#include "ActionDlg.h"
#include "PropertyBox.h"
#include "Application.h"

CurrentRegularReportWidget::CurrentRegularReportWidget(const TR::CurrentReportInfo& currentReportInfo):
    CurrentReportWidget(currentReportInfo)
{   
}

void CurrentRegularReportWidget::buildAdvancedMenu(Wt::WPopupMenu *advancedMenu)
{
    CurrentReportWidget::buildAdvancedMenu(advancedMenu);
    if (!m_content)
    {
        return;
    }

    auto transformItem = advancedMenu->insertItem(0, "Transform");
    auto transformMenu = std::make_unique<Wt::WPopupMenu>();    

    auto queryItem = transformMenu->addItem("Query");
    queryItem->clicked().connect(this, &CurrentRegularReportWidget::onQueryClick);

    auto filtrateItem = transformMenu->addItem("Filtrate");
    filtrateItem->clicked().connect(this, &CurrentRegularReportWidget::onFiltrateClick);

    auto validateItem = transformMenu->addItem("Validate");
    validateItem->clicked().connect(this, &CurrentRegularReportWidget::onValidateClick);

    transformItem->setMenu(std::move(transformMenu));
}

std::unique_ptr<ContentView> CurrentRegularReportWidget::createContentView(std::shared_ptr<TR::Content> content)
{
    _ASSERT(content);

    auto regularContent = std::dynamic_pointer_cast<TR::RegularContent>(content);
    return std::make_unique<RegularContentView>(regularContent, [this](const TR::XML::XmlRegularNode* contextNode, const TR::XML::META::XmlActionNode* actionNode){
        auto actionInfo = m_client->getActionInfo(TR::ActionUUID(actionNode->get_action_uuid()));
        auto sourceInfo = m_client->getSourceInfo(m_currentReportInfo.m_source_key);
        auto params = m_client->createParams(actionInfo.m_uuid, as_string(*actionNode->get_params(*contextNode)));

        auto launchDlg = new LaunchDlg(sourceInfo, actionInfo, std::move(params));
        launchDlg->show();
    });
}

void CurrentRegularReportWidget::onFiltrateClick()
{
    auto reportTypeInfo = m_client->getReportTypeInfo(m_currentReportInfo.m_report_type_uuid);
    if (reportTypeInfo.m_custom_filter_key)
    {
        auto currentFilterInfo = m_client->getCustomFilterInfo(reportTypeInfo.m_custom_filter_key);
        auto box = new PropertyBox(stl_tools::flag | PropertyDialog::BTN_OK | PropertyDialog::BTN_CANCEL, L"FilterReport", L"This report already is result of filtering. So there is options:");
        box->addSwitch(L"", L"", {L"Modify existent filter", L"Create new filter"});
        box->show([=](){
            switch (box->getValue<int>(0))
            {
            case 0:
                Application::pushPage(std::make_unique<ModifyFilterPage>(
                    currentFilterInfo,
                    std::make_unique<CurrentContentQuery>(m_client, m_currentReportInfo.m_source_key, currentFilterInfo.m_input_uuid)));
                break;

            case 1:
                Application::pushPage(std::make_unique<CreateFilterPage>(
                    m_client->getReportTypeInfo(m_currentReportInfo.m_report_type_uuid),
                    std::static_pointer_cast<TR::RegularContent>(m_content)));
                break;

            default:
                _ASSERT(false);
            }
        });
        
    }
    else
    {
        Application::pushPage(std::make_unique<CreateFilterPage>(
            m_client->getReportTypeInfo(m_currentReportInfo.m_report_type_uuid),
            std::static_pointer_cast<TR::RegularContent>(m_content)));
    }
}

void CurrentRegularReportWidget::onQueryClick()
{
    Application::pushPage(std::make_unique<CreateQueryPage>(
        m_client->getReportTypeInfo(m_currentReportInfo.m_report_type_uuid),
        std::static_pointer_cast<TR::RegularContent>(m_content)));
}

void CurrentRegularReportWidget::onValidateClick()
{
    auto inputInfo = m_client->getReportTypeInfo(m_currentReportInfo.m_report_type_uuid);
    Application::pushPage(std::make_unique<CreateValidationPage>(std::move(inputInfo), m_content));
}

CurrentRegularReportPage::CurrentRegularReportPage(const TR::CurrentReportInfo& currentReportInfo):
    CurrentReportPage(currentReportInfo)
{
}

CurrentReportWidget* CurrentRegularReportPage::createWidget()
{
    return new CurrentRegularReportWidget(m_currentReportInfo);
}