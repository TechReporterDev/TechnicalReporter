#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class PreviewWidget: public MainFrameWidget
{
public:
    PreviewWidget(std::unique_ptr<LazyContentQuery> previewContent);
    PreviewWidget(const PreviewWidget&) = delete;

protected:
    void onContentReady(std::shared_ptr<TR::Content> content);
    void onContentUnavailable();
    void onCloseClick();

    std::unique_ptr<LazyContentQuery> m_previewContent;
};

class PreviewPage: public MainFramePage
{
public:
    PreviewPage(std::unique_ptr<LazyContentQuery> previewContent);
    virtual MainFrameWidget* getWidget() override;

protected:
    PreviewWidget m_widget;
};