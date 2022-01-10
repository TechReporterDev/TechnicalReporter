#include "stdafx.h"
#include "TabWidget.h"
#include "WidgetHolder.h"

TabWidget::TabWidget():
    m_headerTab(nullptr),
    m_body(nullptr)
{
    auto container = setImplementation(std::make_unique<Wt::WContainerWidget>());
    container->setPadding(0);
    container->setMargin(0);

    // layout
    auto vbox = container->setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);

    // header
    m_headerTab = vbox->addWidget(std::make_unique<Wt::WTabWidget>());
    m_headerTab->setStyleClass("tabwidget");

    m_headerTab->currentChanged().connect(std::bind([this](int index){
        auto vbox = Wt::clear(m_body);
        vbox->addWidget(std::make_unique<WidgetHolder>(m_tabWidgets[index].get()));
    }, std::placeholders::_1));
    
    m_body = vbox->addWidget(std::make_unique<Wt::WContainerWidget>(), 1);
    m_body->setMargin(0);
    m_body->setPadding(0);

    auto bodyLayout = m_body->setLayout(std::make_unique<Wt::WVBoxLayout>());
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);
}

TabWidget::~TabWidget()
{
    Wt::clear(m_body);
}

Wt::WWidget* TabWidget::addTab(std::unique_ptr<Wt::WWidget> widget, const Wt::WString& label)
{
    auto empty = std::make_unique<Wt::WContainerWidget>();
    empty->setMargin(0);
    empty->setPadding(0);
    m_headerTab->addTab(std::move(empty), label);
    
    if (m_headerTab->count() == 1)
    {
        auto vbox = Wt::clear(m_body);
        vbox->addWidget(std::make_unique<WidgetHolder>(widget.get()));
    }

    m_tabWidgets.push_back(std::move(widget));
    return m_tabWidgets.back().get();
}