#pragma once
#include "InventoryTable.h"
#include "TRWebServer.h"
#include "MainFrame.h"

class ActionSelectionWidget: public MainFrameWidget
{
public:
    ActionSelectionWidget();
    virtual void refresh() override;

private:
    //override Wt::WWidget
    virtual void load() override;   

    void createToolBar();
    void onCreateClick();

    std::vector<TR::ActionInfo> m_actionsInfo;
    InventoryTable* m_inventory;    
};

class ActionSelectionPage: public MainFramePage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    ActionSelectionPage();
    virtual MainFrameWidget* getWidget() override;

private:
    std::unique_ptr<ActionSelectionWidget> m_widget;
};