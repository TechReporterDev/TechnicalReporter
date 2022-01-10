#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class ComplianceSetupWidget: public MainFrameWidget
{
public:
    ComplianceSetupWidget(TR::SourceInfo sourceInfo, TR::ComplianceInfo complianceInfo);
    ComplianceSetupWidget(const ComplianceSetupWidget&) = delete;
    ComplianceSetupWidget& operator = (const ComplianceSetupWidget&) = delete;
    
protected:
    void            onPersistClick();
    void            onPreviewClick();
    void            onCloseClick();

    TR::SourceInfo m_sourceInfo;
    TR::ComplianceInfo m_complianceInfo;
    TR::ComplianceSetup m_complianceSetup; 
    std::shared_ptr<LazyContentQuery> m_previewContent;
};

class ComplianceSetupPage: public MainFramePage
{
public:
    ComplianceSetupPage(TR::SourceInfo sourceInfo, TR::ComplianceInfo complianceInfo);
    virtual MainFrameWidget* getWidget() override;

private:
    ComplianceSetupWidget m_widget;
};