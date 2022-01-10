#include "stdafx.h"
#include "ValidationSettingsPage.h"
#include "ValidatorView.h"
#include "PreviewPage.h"
#include "PropertyBox.h"
#include "Application.h"

class PreviewValidationContentQuery : public LazyContentQuery
{
public:
    PreviewValidationContentQuery(TR::ValidationSettings validationSettings, std::shared_ptr<LazyContentQuery> inputQuery):
        LazyContentQuery(nullptr, inputQuery->getSourceKey(), inputQuery->getReportTypeUUID()),
        m_validationSettings(std::move(validationSettings)),
        m_inputQuery(inputQuery)
    {
    }

    ~PreviewValidationContentQuery()
    {
        cancel();
    }

    virtual void doRun(CompletedHandler completed_handler, FailedHandler failed_handler) override
    {
        m_inputQuery->run(
            [this, completed_handler](std::shared_ptr<TR::Content> content){
                completed_handler(m_validationSettings.apply_to(std::dynamic_pointer_cast<TR::RegularContent>(content)));
            },
            failed_handler
        );
    }

    virtual void doCancel() override
    {
        m_inputQuery->cancel();
    }

    TR::ValidationSettings m_validationSettings;
    std::shared_ptr<LazyContentQuery> m_inputQuery;
};

ValidationSettingsWidget::ValidationSettingsWidget()
{
}

void ValidationSettingsWidget::initialize()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());   

    {
        auto persistBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Persist");
        persistBtn->clicked().connect(this, &ValidationSettingsWidget::onPersistClick);
    }
    

    if (m_previewContent)
    {
        auto previewBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Preview");
        previewBtn->clicked().connect(this, &ValidationSettingsWidget::onPreviewClick);
    }

    {
        auto closeBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Close");
        closeBtn->clicked().connect(this, &ValidationSettingsWidget::onCloseClick);
    }

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->addWidget(std::make_unique<ValidatorView>(*m_validationSettings.m_validator, m_validationSettings.m_base_validator.get()));
}

void ValidationSettingsWidget::onPreviewClick()
{
    Application::pushPage(std::make_unique<PreviewPage>(std::make_unique<PreviewValidationContentQuery>(m_validationSettings.clone(), m_previewContent)));
}

void ValidationSettingsWidget::onCloseClick()
{
    Application::popPage();
}

ModifyValidationWidget::ModifyValidationWidget(TR::SourceInfo sourceInfo, TR::ValidationInfo validationInfo):
    m_sourceInfo(std::move(sourceInfo)),
    m_validationInfo(std::move(validationInfo))
{
    m_validationSettings = m_client->getValidationSettings(m_sourceInfo.m_key, m_validationInfo.m_key);
    m_previewContent = std::make_unique<CurrentContentQuery>(m_client, m_sourceInfo.m_key, m_validationInfo.m_base_report_type_uuid);
    initialize();
}

void ModifyValidationWidget::onPersistClick()
{
    m_client->setValidationSettings(m_sourceInfo.m_key, m_validationInfo.m_key, m_validationSettings.clone());

    auto messageBox = new Wt::WMessageBox("Validation Setup", "Validator settings was saved successfully.", Wt::Icon::Information, Wt::StandardButton::Ok);
    messageBox->buttonClicked().connect(std::bind([messageBox]() { delete messageBox; }));
    messageBox->setWidth(300);
    messageBox->show();
}

ModifyValidationPage::ModifyValidationPage(TR::SourceInfo sourceInfo, TR::ValidationInfo validationInfo):
    MainFramePage(L"Validation Settings", "/modify_validation"),
    m_widget(std::move(sourceInfo), std::move(validationInfo))
{
}

MainFrameWidget* ModifyValidationPage::getWidget()
{
    return &m_widget;
}

CreateValidationWidget::CreateValidationWidget(TR::ReportTypeInfo inputInfo, std::shared_ptr<TR::Content> previewContent):
    m_inputInfo(std::move(inputInfo))
{
    m_validationSettings = m_client->createValidationSettings(m_inputInfo.m_uuid);
    if (previewContent)
    {
        m_previewContent = std::make_unique<CurrentContentQuery>(TR::SourceKey(), TR::ReportTypeUUID(), previewContent);
    }

    initialize();
}

void CreateValidationWidget::onPersistClick()
{
    auto propertyBox = new PropertyBox(stl_tools::flag | Dialog::BTN_OK | Dialog::BTN_CANCEL, L"Save", L"<p>Do you want to register new validator?</p>", Wt::Icon::Question);
    propertyBox->addString(L"Output name", L"", L"My Validator");
    propertyBox->show([this, propertyBox](){
        m_client->addValidation(propertyBox->getValue<std::wstring>(0), m_inputInfo.m_uuid, m_validationSettings.clone());
    });
}

CreateValidationPage::CreateValidationPage(TR::ReportTypeInfo inputInfo, std::shared_ptr<TR::Content> previewContent):
    MainFramePage(L"Validation Settings", "/create_validation"),
    m_widget(std::move(inputInfo), previewContent)
{
}

MainFrameWidget* CreateValidationPage::getWidget()
{
    return &m_widget;
}