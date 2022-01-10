#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class CreateQueryWidget: public MainFrameWidget
{
public:
    CreateQueryWidget(std::vector<TR::ReportTypeInfo> inputsInfo, std::shared_ptr<TR::RegularContent> previewContent);
    CreateQueryWidget(const CreateQueryWidget&) = delete;
    CreateQueryWidget& operator = (const CreateQueryWidget&) = delete;

protected:
    void            onPersistClick();
    void            onPreviewClick();
    void            onCloseClick();

    std::vector<TR::ReportTypeInfo> m_inputsInfo;
    TR::CustomQueryDefinition m_queryDefinition;
    std::shared_ptr<TR::RegularContent> m_previewContent;
};

class CreateQueryPage: public MainFramePage
{
public:
    CreateQueryPage(std::vector<TR::ReportTypeInfo> inputsInfo, std::shared_ptr<TR::RegularContent> previewContent = nullptr);
    CreateQueryPage(TR::ReportTypeInfo inputInfo, std::shared_ptr<TR::RegularContent> previewContent = nullptr);

    virtual MainFrameWidget* getWidget() override;

private:
    CreateQueryWidget m_widget;
};