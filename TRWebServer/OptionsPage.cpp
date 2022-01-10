#include "stdafx.h"
#include "OptionsPage.h"
#include "InventoryTable.h"
#include "ReportSelectionPage.h"
#include "RoleSelectionPage.h"
#include "SourceSelectionPage.h"
#include "ScheduleSelectionPage.h"
#include "ActionSelectionPage.h"
#include "StreamSelectionPage.h"
#include "Application.h"

OptionsWidget::OptionsWidget()
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);

    auto inventory = vbox->addWidget(std::make_unique<InventoryTable>());
    inventory->addTool(L"Setup", std::bind(&OptionsWidget::onSetupClick, this, std::placeholders::_1), true);

    inventory->setItem(0, L"Sources", L"Setup source");
    inventory->setItem(1, L"Roles", L"Create or setup role");
    inventory->setItem(2, L"Reports", L"Create or setup report");
    inventory->setItem(3, L"Streams", L"Create or setup stream");
    inventory->setItem(4, L"Actions", L"Create or setup action or shortcut");
    inventory->setItem(5, L"Schedules", L"Create or modify schedule");  
}

void OptionsWidget::onSetupClick(int index)
{
    std::unique_ptr<MainFramePage> nextPage;
    switch (index)
    {
    case 0:
        nextPage = std::make_unique<SourceSelectionPage>();
        break;

    case 1:
        nextPage = std::make_unique<RoleSelectionPage>();
        break;

    case 2:
        nextPage = std::make_unique<ReportSelectionPage>();
        break;

    case 3:
        nextPage = std::make_unique<StreamSelectionPage>();
        break;

    case 4:
        nextPage = std::make_unique<ActionSelectionPage>();
        break;

    case 5:
        nextPage = std::make_unique<ScheduleSelectionPage>();
        break;

    default:
        _ASSERT(false);
    }
    Application::showPage(std::move(nextPage)); 
}

std::unique_ptr<MainFramePage> OptionsPage::createPage(const std::string& url)
{
    if (url == "/options")
    {
        return std::make_unique<OptionsPage>();
    }
    return nullptr;
}

OptionsPage::OptionsPage():
    MainFramePage(L"Options", "/options")
{
}

MainFrameWidget* OptionsPage::getWidget()
{
    if (!m_widget)
    {
        m_widget.reset(new OptionsWidget());
    }
    return m_widget.get();
}