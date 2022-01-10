#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class RoleList;
class RoleSelectionWidget: public MainFrameWidget
{
public:
    RoleSelectionWidget();
    void createToolBar();

private:
    RoleList* m_roleList;
};

class RoleSelectionPage: public MainFramePage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    RoleSelectionPage();
    virtual MainFrameWidget*    getWidget() override;

private:
    std::unique_ptr<RoleSelectionWidget> m_widget;
};