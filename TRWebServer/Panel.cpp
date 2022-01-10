#include "stdafx.h"
#include "Panel.h"
#include "Application.h"

Panel::Panel(const Wt::WString& title):
    AppEventHandler(&Application::instance()->getClient()),
    m_header(nullptr),
    m_body(nullptr)
{
    setStyleClass("panel panel-default");
    setPadding(0);
    //setMargin(0);

    // layout
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);

    // header   
    m_header = vbox->addWidget(std::make_unique<Wt::WContainerWidget>());   
    m_header->setStyleClass("tr-panel-heading");    
    auto hbox = m_header->setLayout(std::make_unique<Wt::WHBoxLayout>());
    hbox->addWidget(std::make_unique<Wt::WText>(title));
    hbox->addStretch(1);
    
    //body
    m_body = vbox->addWidget(std::make_unique<Wt::WContainerWidget>(), 1);
}

void Panel::addHeaderBtn(const Wt::WString& caption, OnClick onClick)
{ 
    auto hbox = static_cast<Wt::WHBoxLayout*>(m_header->layout());
    auto btn = hbox->addWidget(std::make_unique<Wt::WPushButton>(caption)); 
    btn->setStyleClass("btn-xs");
    btn->clicked().connect([onClick](Wt::WMouseEvent){
        onClick();
    });
}

Wt::WContainerWidget* Panel::getBody()
{
    return m_body;
}