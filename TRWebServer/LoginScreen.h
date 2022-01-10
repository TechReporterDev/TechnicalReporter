#pragma once
#include "Application.h"

class LoginScreen: public Wt::WContainerWidget
{
public:
    LoginScreen(AppClient& client, std::function<void(void)> onConnected);
    
private:
    void connect();

    AppClient& m_client;
    std::function<void(void)> m_onConnected;

    Wt::WLineEdit* m_loginEdit;
    Wt::WLineEdit* m_passwordEdit;  
};