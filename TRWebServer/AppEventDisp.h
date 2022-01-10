#pragma once
#include "TRWebServer.h"
class Application;
class AppEventDisp
{
public:
    AppEventDisp(Application& app);
    void dispatch(TR::Client::PackedEvent evt);
    void handleEvents();

private:
    Application& m_app;
    std::queue<TR::Client::PackedEvent> m_events;
    std::mutex m_mutex;
    Wt::WTimer m_timer;
};