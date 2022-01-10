#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "InventoryTable.h"

class FeaturesWidget: public MainFrameWidget
{
public:
    FeaturesWidget();

private:
    void refresh();
    void onInstallClick(int position);
    void onUninstallClick(int position);
    void onInfoClick(int position);

    std::vector<TR::FeatureInfo> m_features;
    InventoryTable* m_inventory;
};

class FeaturesPage: public MainFramePage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    FeaturesPage();
    virtual MainFrameWidget* getWidget() override;

private:
    std::unique_ptr<FeaturesWidget> m_widget;
};