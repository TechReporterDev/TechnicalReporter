#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class AdminWidget: public MainFrameWidget
{
public:
    AdminWidget();

private:
    void onSetupClick(int position);
};

class AdminPage: public MainFramePage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    AdminPage();
    virtual MainFrameWidget* getWidget() override;

private:
    std::unique_ptr<AdminWidget> m_widget;
};