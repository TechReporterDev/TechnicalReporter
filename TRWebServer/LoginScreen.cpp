#include "stdafx.h"
#include "LoginScreen.h"
#include "Layout.h"

static const int KC_ENTER = 0x0D;

LoginScreen::LoginScreen(AppClient& client, std::function<void(void)> onConnected):
    m_client(client),
    m_onConnected(onConnected),
    m_loginEdit(nullptr),
    m_passwordEdit(nullptr)
{
    keyPressed().connect([this](Wt::WKeyEvent& evt){        
        if (evt.key() == Wt::Key::Enter)
        {
            connect();
        }
    });

    auto layout = centerLayout(this);
    
    auto container = layout->addWidget(std::make_unique<Wt::WContainerWidget>());
    container->setWidth(320);

    auto hboxLogin = container->addWidget(std::make_unique<Wt::WContainerWidget>())->setLayout(std::make_unique<Wt::WHBoxLayout>());
    hboxLogin->addStretch(1);
    auto loginLabel = hboxLogin->addWidget(std::make_unique<Wt::WLabel>(L"Login:"));
    loginLabel->setLineHeight(25);
    loginLabel->setMargin(5);
    m_loginEdit = hboxLogin->addWidget(std::make_unique<Wt::WLineEdit>());
    loginLabel->setBuddy(m_loginEdit);
    m_loginEdit->setWidth(200);
    

    auto hboxPassword = container->addWidget(std::make_unique<Wt::WContainerWidget>())->setLayout(std::make_unique<Wt::WHBoxLayout>());
    hboxPassword->addStretch(1);
    auto passwordLabel = hboxPassword->addWidget(std::make_unique<Wt::WLabel>(L"Password:"));   
    passwordLabel->setLineHeight(25);
    passwordLabel->setMargin(5);
    m_passwordEdit = hboxPassword->addWidget(std::make_unique<Wt::WLineEdit>());
    passwordLabel->setBuddy(m_passwordEdit);
    m_passwordEdit->setEnabled(false);
    m_passwordEdit->setWidth(200);

    auto buttons = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    auto hbox = buttons->setLayout(std::make_unique<Wt::WHBoxLayout>());
    hbox->addStretch(1);

    auto connectBtn = hbox->addWidget(std::make_unique<Wt::WPushButton>(L"Connect"));
    connectBtn->setDefault(true);

    connectBtn->clicked().connect([this](Wt::WMouseEvent){
        connect();
    });
}

void LoginScreen::connect()
{   
    try
    {
        std::wstring login = m_loginEdit->text();
        m_client.connect(*getServerCore(), login);
        m_onConnected();
    }
    catch (std::exception&)
    {
        Wt::WMessageBox::show(L"Connection failed", L"Invalid login or password.", Wt::StandardButton::Ok);
    }
}