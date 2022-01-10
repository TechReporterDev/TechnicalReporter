#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
class StreamView;

class StreamPreviewWidget: public MainFrameWidget
{
public:
    StreamPreviewWidget(std::unique_ptr<LazyStreamQuery> streamQuery);
    StreamPreviewWidget(const StreamPreviewWidget&) = delete;

protected:
    void showTitleBar();
    void showMessage(const Wt::WString& message);
    void showStream(const std::vector<TR::StreamMessage>& streamMessages);
    void onCloseClick();

    std::unique_ptr<LazyStreamQuery> m_streamQuery;
};

class StreamPreviewPage: public MainFramePage
{
public:
    StreamPreviewPage(std::unique_ptr<LazyStreamQuery> streamQuery);
    virtual MainFrameWidget* getWidget() override;

protected:
    StreamPreviewWidget m_widget;
};