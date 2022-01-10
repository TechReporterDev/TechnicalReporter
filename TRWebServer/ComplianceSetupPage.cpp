#include "stdafx.h"
#include "ComplianceSetupPage.h"
#include "ComplianceSetupView.h"
#include "PreviewPage.h"
#include "Application.h"

class PreviewComplianceContentQuery: public LazyContentQuery
{
public:
    PreviewComplianceContentQuery(TR::ComplianceSetup complianceSetup, std::shared_ptr<LazyContentQuery> checkListQuery):
        LazyContentQuery(nullptr, checkListQuery->getSourceKey(), checkListQuery->getReportTypeUUID()),
        m_complianceSetup(std::move(complianceSetup)),
        m_checkListQuery(checkListQuery)
    {
    }

    ~PreviewComplianceContentQuery()
    {
        cancel();
    }

    virtual void doRun(CompletedHandler completed_handler, FailedHandler failed_handler) override
    {
        m_checkListQuery->run(
            [this, completed_handler](std::shared_ptr<TR::Content> content){
                completed_handler(m_complianceSetup.apply_to(std::dynamic_pointer_cast<TR::CheckListContent>(content)));
            },
            failed_handler
        );
    }

    virtual void doCancel() override
    {
        m_checkListQuery->cancel();
    }

    TR::ComplianceSetup m_complianceSetup;
    std::shared_ptr<LazyContentQuery> m_checkListQuery;
};

ComplianceSetupWidget::ComplianceSetupWidget(TR::SourceInfo sourceInfo, TR::ComplianceInfo complianceInfo):
    m_sourceInfo(std::move(sourceInfo)),
    m_complianceInfo(std::move(complianceInfo))
{
    m_complianceSetup = m_client->getComplianceSetup(m_sourceInfo.m_key, m_complianceInfo.m_key);
    m_previewContent = std::make_unique<CurrentContentQuery>(m_client, m_sourceInfo.m_key, complianceInfo.m_base_report_type_uuid);

    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());   

    auto persistBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Persist");
    persistBtn->clicked().connect(this, &ComplianceSetupWidget::onPersistClick);

    auto previewBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Preview");
    previewBtn->clicked().connect(this, &ComplianceSetupWidget::onPreviewClick);

    auto closeBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Close");
    closeBtn->clicked().connect(this, &ComplianceSetupWidget::onCloseClick);

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->addWidget(std::make_unique<ComplianceSetupView>(
        *m_complianceSetup.m_setup,
        *m_complianceSetup.m_base_setup,
        nullptr), 1);
}

void ComplianceSetupWidget::onPersistClick()
{
    m_client->setComplianceSetup(m_sourceInfo.m_key, m_complianceInfo.m_key, m_complianceSetup.clone());

    auto messageBox = new Wt::WMessageBox("Compliance Setup", "Compliance settings was saved successfully.", Wt::Icon::Information, Wt::StandardButton::Ok);
    messageBox->buttonClicked().connect(std::bind([messageBox]() { delete messageBox; }));
    messageBox->setWidth(300);
    messageBox->show();
}

void ComplianceSetupWidget::onPreviewClick()
{
    Application::pushPage(std::make_unique<PreviewPage>(std::make_unique<PreviewComplianceContentQuery>(m_complianceSetup.clone(), m_previewContent)));
}

void ComplianceSetupWidget::onCloseClick()
{
    Application::popPage();
}

ComplianceSetupPage::ComplianceSetupPage(TR::SourceInfo sourceInfo, TR::ComplianceInfo complianceInfo):
    MainFramePage(L"Compliance Setup", "/compliance_setup"),
    m_widget(std::move(sourceInfo), std::move(complianceInfo))
{
}

MainFrameWidget* ComplianceSetupPage::getWidget()
{
    return &m_widget;
}