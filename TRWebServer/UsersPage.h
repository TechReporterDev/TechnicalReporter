#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "InventoryTable.h"

class UsersWidget: public MainFrameWidget
{
public:
    UsersWidget();

private:
    void refresh();
    void createToolBar();
    void onAddClick();
    void onSetupClick(int position);
    void onRemoveClick(int position);

    std::vector<TR::UserInfo> m_users;
    InventoryTable* m_inventory;
};

class UsersPage: public MainFramePage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    UsersPage();
    virtual MainFrameWidget* getWidget() override;

private:
    std::unique_ptr<UsersWidget> m_widget;
};