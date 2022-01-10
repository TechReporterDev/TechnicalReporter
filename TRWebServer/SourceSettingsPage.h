#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class SourceSettingsWidget: public MainFrameWidget
{
public:
    using OnApply = std::function<void(TR::SourceKey sourceKey)>;

    SourceSettingsWidget(TR::SourceKey sourceKey, TR::SourceSettings sourceSettings);
    void setOnApply(OnApply onApply);
        
private:
    class GeneralPage;
    class AdvancedPage;
    class SecurityPage;

    void init();
    void createToolBar();
    void onOk();
    void onCancel();

    OnApply m_onApply;

    TR::SourceKey m_sourceKey;
    TR::SourceSettings m_sourceSettings;
    boost::optional<TR::SourceConfig> m_sourceConfig;
    boost::optional<TR::SourceAccessInfo> m_accessInfo;
};

class SourceSettingsPage: public MainFramePage
{
public:
    using OnApply = SourceSettingsWidget::OnApply;
    SourceSettingsPage(TR::SourceKey sourceKey, TR::SourceSettings sourceSettings);
    SourceSettingsPage(TR::SourceSettings sourceSettings);
    
    void                        setOnApply(OnApply onApply);
    virtual MainFrameWidget*    getWidget() override;

private:
    SourceSettingsWidget m_widget;
};