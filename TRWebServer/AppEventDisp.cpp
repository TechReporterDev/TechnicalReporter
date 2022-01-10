#include "stdafx.h"
#include "AppEventDisp.h"
#include "Application.h"

AppEventDisp::AppEventDisp(Application& app):
    m_app(app)
{
    m_timer.setInterval(std::chrono::milliseconds(2000));
    m_timer.timeout().connect(std::bind(&AppEventDisp::handleEvents, this));
    m_timer.start();
}

void AppEventDisp::dispatch(TR::Client::PackedEvent evt)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_events.push(std::move(evt));
}

void AppEventDisp::handleEvents()
{
    try
    {
        for (;;)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_events.empty())
                break;

            auto evt = std::move(m_events.front());
            m_events.pop();
            lock.unlock();

            evt();
        }
    }
    catch (std::exception&)
    {
        //show message
        m_app.quit();
    }
}