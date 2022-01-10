#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class ValidationSettingsWidget: public MainFrameWidget
{
public:
    ValidationSettingsWidget();
    ValidationSettingsWidget(const ValidationSettingsWidget&) = delete;
    ValidationSettingsWidget& operator = (const ValidationSettingsWidget&) = delete;

protected:
    void            initialize();
    virtual void    onPersistClick() = 0;
    void            onPreviewClick();
    void            onCloseClick();

    TR::ValidationSettings m_validationSettings;
    std::shared_ptr<LazyContentQuery> m_previewContent;
};

class ModifyValidationWidget: public ValidationSettingsWidget
{
public:
    ModifyValidationWidget(TR::SourceInfo sourceInfo, TR::ValidationInfo validationInfo);
    ModifyValidationWidget(const ModifyValidationWidget&) = delete;
    ModifyValidationWidget& operator = (const ModifyValidationWidget&) = delete;

protected:
    // ValidationSettingsWidget override
    virtual void onPersistClick() override;

    TR::SourceInfo m_sourceInfo;
    TR::ValidationInfo m_validationInfo;
};

class ModifyValidationPage: public MainFramePage
{
public:
    ModifyValidationPage(TR::SourceInfo sourceInfo, TR::ValidationInfo validationInfo);
    virtual MainFrameWidget* getWidget() override;

private:
    ModifyValidationWidget m_widget;
};

class CreateValidationWidget: public ValidationSettingsWidget
{
public:
    CreateValidationWidget(TR::ReportTypeInfo inputInfo, std::shared_ptr<TR::Content> previewContent);
    CreateValidationWidget(const CreateValidationWidget&) = delete;
    CreateValidationWidget& operator = (const CreateValidationWidget&) = delete;

protected:
    // ValidationSettingsWidget override
    virtual void onPersistClick() override;

    TR::ReportTypeInfo m_inputInfo;
};

class CreateValidationPage: public MainFramePage
{
public:
    CreateValidationPage(TR::ReportTypeInfo inputInfo, std::shared_ptr<TR::Content> previewContent = nullptr);
    virtual MainFrameWidget* getWidget() override;

private:
    CreateValidationWidget m_widget;
};