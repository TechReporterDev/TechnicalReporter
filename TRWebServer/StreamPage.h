#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
class StreamView;

class StreamWidget: public MainFrameWidget
{
public:
    static const time_t DELAY = 5;

    StreamWidget(const TR::StreamInfo& streamInfo, time_t start, time_t stop, bool autoRefresh);
    StreamWidget(const StreamWidget&) = delete;

protected:
    void showTitleBar();
    void showMessage(const Wt::WString& message);
    void showStream(const TR::StreamSegment& streamSegment);
    void queryStream(time_t start, time_t stop);
    void onFiltrateClick();
    void refresh() override;

    TR::StreamInfo m_streamInfo;
    time_t m_start;
    time_t m_stop;
    bool m_autoRefresh;

    std::shared_ptr<const TR::XmlDefDoc> m_messageDef;
    TR::Client::Job m_streamQuery;
    StreamView* m_streamView;
};

class StreamPage: public MainFramePage
{
public:
    StreamPage(const TR::StreamInfo& streamInfo, time_t start, time_t stop, bool autoRefresh = false);
    virtual MainFrameWidget* getWidget() override;

protected:
    StreamWidget m_widget;
};