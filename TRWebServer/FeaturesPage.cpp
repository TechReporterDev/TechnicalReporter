#include "stdafx.h"
#include "FeaturesPage.h"
#include "PropertyDialog.h"
#include "PropertyBox.h"
#include "RefValue.h"
#include "CommonPropertyPage.h"
#include "Application.h"

FeaturesWidget::FeaturesWidget():
    m_inventory(nullptr)
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);

    m_inventory = vbox->addWidget(std::make_unique<InventoryTable>());  
    m_inventory->addTool(L"Install", boost::bind(&FeaturesWidget::onInstallClick, this, _1), false);
    m_inventory->addTool(L"Uninstall", boost::bind(&FeaturesWidget::onUninstallClick, this, _1), false);
    m_inventory->addTool(L"Info", boost::bind(&FeaturesWidget::onInfoClick, this, _1), false);

    refresh();
}

void FeaturesWidget::refresh()
{
    m_features = m_client->getFeaturesInfo();
    m_inventory->resize(m_features.size());
    for (size_t position = 0; position < m_features.size(); ++position)
    {
        m_inventory->setItem(position, m_features[position].m_name, L"");
    }
}

void FeaturesWidget::onInstallClick(int position)
{
    m_client->installFeature(m_features[position].m_uuid);
    refresh();
}

void FeaturesWidget::onUninstallClick(int position)
{
    auto propertyBox = new PropertyBox(stl_tools::flag | Dialog::BTN_OK | Dialog::BTN_CANCEL, L"Remove feature", L"Do you really want to uninstall feature?", Wt::Icon::Question);
    propertyBox->show([this, position]{
        m_client->uninstallFeature(m_features[position].m_uuid);
        refresh();
    });
}

void FeaturesWidget::onInfoClick(int position)
{
    _ASSERT(false); //implement later
}

std::unique_ptr<MainFramePage> FeaturesPage::createPage(const std::string& url)
{
    if (url == "/features")
    {
        return std::make_unique<FeaturesPage>();
    }
    return nullptr;
}

FeaturesPage::FeaturesPage():
    MainFramePage(L"Features", "/features")
{
}

MainFrameWidget* FeaturesPage::getWidget()
{
    if (!m_widget)
    {
        m_widget.reset(new FeaturesWidget());
    }
    return m_widget.get();
}