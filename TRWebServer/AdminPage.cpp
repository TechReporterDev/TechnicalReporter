#include "stdafx.h"
#include "AdminPage.h"
#include "InventoryTable.h"
#include "UsersPage.h"
#include "FeaturesPage.h"
#include "Application.h"

AdminWidget::AdminWidget()
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);

    auto inventory = vbox->addWidget(std::make_unique<InventoryTable>());
    inventory->addTool(L"Setup", std::bind(&AdminWidget::onSetupClick, this, std::placeholders::_1), true);
    inventory->setItem(0, "Features", "Install or uninstall feature on server");
    inventory->setItem(1, "Users", "Add, remove or modify user");
    inventory->setItem(2, "Audit", "Show server log");
}

void AdminWidget::onSetupClick(int position)
{
    std::unique_ptr<MainFramePage> nextPage;
    switch (position)
    {
    case 0:
        nextPage = std::make_unique<FeaturesPage>();
        break;

    case 1:
        nextPage = std::make_unique<UsersPage>();
        break;

    case 2:
        return;

    default:
        _ASSERT(false);
    }

    Application::pushPage(std::move(nextPage));
}

std::unique_ptr<MainFramePage> AdminPage::createPage(const std::string& url)
{
    if (url == "/admin")
    {
        return std::make_unique<AdminPage>();
    }
    return nullptr;
}

AdminPage::AdminPage():
    MainFramePage(L"Administrator", "/admin")
{
}

MainFrameWidget* AdminPage::getWidget()
{
    if (!m_widget)
    {
        m_widget.reset(new AdminWidget());
    }
    return m_widget.get();
}