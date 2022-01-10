#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class SuitSettingsWidget: public MainFrameWidget
{
public:
    using OnApply = std::function<void(TR::SourceKey suitKey)>;

    SuitSettingsWidget(TR::SourceKey suitKey, TR::SourceSettings suitSettings);
    void setOnApply(OnApply onApply);

private:
    class GeneralPage;
    class SecurityPage;

    void init();
    void createToolBar();
    void onOk();
    void onCancel();

    OnApply m_onApply;

    TR::SourceKey m_suitKey;
    TR::SourceSettings m_suitSettings;
    boost::optional<TR::SourceAccessInfo> m_accessInfo;
};

class SuitSettingsPage: public MainFramePage
{
public:
    using OnApply = SuitSettingsWidget::OnApply;
    SuitSettingsPage(TR::SourceKey sourceKey, TR::SourceSettings suitSettings);
    SuitSettingsPage(TR::SourceSettings suitSettings);

    void                        setOnApply(OnApply onApply);
    virtual MainFrameWidget*    getWidget() override;

private:
    SuitSettingsWidget m_widget;
};