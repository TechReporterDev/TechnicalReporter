#include "stdafx.h"
#include "Application.h"
#include "MainFrame.h"
#include "LoginScreen.h"

Application* Application::instance()
{
    return static_cast<Application*>(Wt::WApplication::instance());
}

Application::Application(const Wt::WEnvironment& env):
    Wt::WApplication(env),
    m_eventDisp(*this),
    m_client(m_eventDisp)
{   
    //enableUpdates();
    
    auto bootstrapTheme = std::make_shared<Wt::WBootstrapTheme>();
    bootstrapTheme->setVersion(Wt::BootstrapVersion::v3);
    setTheme(bootstrapTheme);   

    useStyleSheet("resources/font-awesome/css/font-awesome.min.css");
    useStyleSheet("css/technical-reporter.css");

    messageResourceBundle().use("/docroot/localization");

    auto vbox = root()->setLayout(std::make_unique<Wt::WVBoxLayout>()); 
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);

    vbox->addWidget(std::make_unique<LoginScreen>(m_client, [this]{
        showMainFrame();
    }));
};

Application::~Application()
{
    root()->clear();
}

AppClient& Application::getClient()
{
    return m_client;
}

MainFrame& Application::getMainFrame()
{
    return *m_mainFrame;
}

void Application::handleEvents()
{
    m_eventDisp.handleEvents();
}

void Application::showPage(std::unique_ptr<MainFramePage> page)
{
    auto app = Application::instance();
    auto& mainFrame = app->getMainFrame();
    mainFrame.showPage(std::move(page));
}

void Application::pushPage(std::unique_ptr<MainFramePage> page)
{
    auto app = Application::instance();
    auto& mainFrame = app->getMainFrame();
    mainFrame.pushPage(std::move(page));
}

Wt::WString Application::formatTime(time_t time)
{
    struct tm tm = {0};
    localtime_s(&tm, &time);
    return boost::lexical_cast<std::wstring>(std::put_time(&tm, L"%c"));
}

void Application::notify(const Wt::WEvent& e)
{
    try
    {
        Wt::WApplication::notify(e);
    }
    catch (AppError& e)
    {
        Wt::WMessageBox::show(L"Operation failed", e.message(), Wt::StandardButton::Ok);
    }
    catch (std::exception&)
    {
        // show message ?
        quit();
    }
}

void Application::popPage()
{
    auto app = Application::instance();
    auto& mainFrame = app->getMainFrame();
    mainFrame.popPage();
}

void Application::showMainFrame()
{
    auto vbox = Wt::clear(root());  
    m_mainFrame = vbox->addWidget(std::make_unique<MainFrame>());

    internalPathChanged().connect(std::bind([this](){
        m_mainFrame->showUrl(internalPath());
    }));
}

AppError::AppError(std::string key, std::string system_message):
    m_key(std::move(key)),
    m_system_message(std::move(system_message))
{
}

Wt::WString AppError::message() const
{
    return Wt::WString::tr(m_key);
}

Wt::WString AppError::system_message() const
{
    return m_system_message;
}

const char* AppError::what() const
{
    return m_system_message.c_str();
}

std::unique_ptr<Wt::WApplication> createApplication(const Wt::WEnvironment& env)
{
    return std::make_unique<Application>(env);
}
