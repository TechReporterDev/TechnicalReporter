#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "AppEventDisp.h"
#include "AppClient.h"

class Application: public Wt::WApplication
{
public:
    static Application* instance();
    Application(const Wt::WEnvironment& env);
    ~Application();
    
    AppClient&          getClient();
    MainFrame&          getMainFrame(); 
    void                handleEvents();

    static void         popPage();
    static void         showPage(std::unique_ptr<MainFramePage> page);
    static void         pushPage(std::unique_ptr<MainFramePage> page);

    static Wt::WString  formatTime(time_t time);

    template<class F>
    void post(F func)
    {
        Wt::WServer::instance()->post(sessionId(), bind(func));
    }
    
private:
    virtual void notify(const Wt::WEvent& e);
    void showMainFrame();

    AppEventDisp m_eventDisp;
    AppClient m_client;
    MainFrame* m_mainFrame = nullptr;
};

class AppError: public std::exception
{
public:
    AppError(std::string key, std::string system_message);

    Wt::WString     message() const;
    Wt::WString     system_message() const;

    //override std::exception
    virtual const char * what() const;

private:
    std::string m_key;
    std::string m_system_message;
};

class LoadingIndicatorLock
{
public:
    LoadingIndicatorLock()
    {
        Application::instance()->loadingIndicator()->show();
    }

    ~LoadingIndicatorLock()
    {
        Application::instance()->loadingIndicator()->hide();
    }
};

// Application factory
std::unique_ptr<Wt::WApplication> createApplication(const Wt::WEnvironment& env);
