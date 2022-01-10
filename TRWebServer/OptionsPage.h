#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class OptionsWidget: public MainFrameWidget
{
public:
    OptionsWidget();

private:
    void onSetupClick(int index);
};

class OptionsPage: public MainFramePage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    OptionsPage();
    virtual MainFrameWidget*    getWidget() override;   

private:
    std::unique_ptr<OptionsWidget> m_widget;
};