#include "stdafx.h"
#include "CreateQueryPage.h"
#include "QueryDocView.h"
#include "PreviewPage.h"
#include "PropertyBox.h"
#include "Application.h"

CreateQueryWidget::CreateQueryWidget(std::vector<TR::ReportTypeInfo> inputsInfo, std::shared_ptr<TR::RegularContent> previewContent):
    m_inputsInfo(std::move(inputsInfo)),
    m_previewContent(previewContent)
{
    m_queryDefinition = m_client->createCustomQuery(stl_tools::copy_vector(m_inputsInfo | stl_tools::members(&TR::ReportTypeInfo::m_uuid)));

    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());   

    auto persistBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Persist");  
    persistBtn->clicked().connect(this, &CreateQueryWidget::onPersistClick);

    if (m_previewContent)
    {
        auto previewBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Preview");
        previewBtn->clicked().connect(this, &CreateQueryWidget::onPreviewClick);
    }

    auto closeBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Close");
    closeBtn->clicked().connect(this, &CreateQueryWidget::onCloseClick);

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->addWidget(std::make_unique<QueryDocView>(*m_queryDefinition.m_query_doc));
}

void CreateQueryWidget::onPersistClick()
{
    auto propertyBox = new PropertyBox(stl_tools::flag | Dialog::BTN_OK | Dialog::BTN_CANCEL, L"Save", L"<p>Do you really want to persist query?</p>", Wt::Icon::Question);
    propertyBox->addString(L"Output name:", L"", L"", L"Enter output report name");
    propertyBox->addCheck(L"", L"Create 'view' instead of standalone report");
    propertyBox->show([this, propertyBox](){
        auto key = m_client->addCustomQuery(propertyBox->getValue<std::wstring>(0), m_queryDefinition);
        auto customQueryInfo = m_client->getCustomQueryInfo(key);
        auto reportOptions = TR::ReportOptions(customQueryInfo.m_output_uuid);
        reportOptions.m_report_generating.m_enable_report_policy = TR::EnableReportPolicy::ENABLE_REPORT;
        if (propertyBox->getValue<bool>(1))
        {
            reportOptions.m_report_generating.m_defer_report_policy = TR::DeferReportPolicy::DEFER_REPORT;
        }
        m_client->setOptions(TR::DEFAULT_ROLE_KEY, reportOptions);
    });
}

void CreateQueryWidget::onPreviewClick()
{
    Application::pushPage(std::make_unique<PreviewPage>(
        std::make_unique<CurrentContentQuery>(TR::SourceKey(), TR::ReportTypeUUID(), m_queryDefinition.apply_to(m_previewContent))));
}

void CreateQueryWidget::onCloseClick()
{
    Application::popPage();
}

CreateQueryPage::CreateQueryPage(std::vector<TR::ReportTypeInfo> inputsInfo, std::shared_ptr<TR::RegularContent> prewiewContent):
    MainFramePage(L"Create Query", "/custom_query"),
    m_widget(std::move(inputsInfo), prewiewContent)
{
}

CreateQueryPage::CreateQueryPage(TR::ReportTypeInfo inputInfo, std::shared_ptr<TR::RegularContent> prewiewContent):
    CreateQueryPage(std::vector<TR::ReportTypeInfo>{inputInfo}, prewiewContent)
{
}

MainFrameWidget* CreateQueryPage::getWidget()
{
    return &m_widget;
}