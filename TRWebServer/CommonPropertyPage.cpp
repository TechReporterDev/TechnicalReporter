#include "stdafx.h"
#include "CommonPropertyPage.h"
#include "CommonPropertyView.h"
#include "PropertyBox.h"
#include "Application.h"

CommonPropertyPage::CommonPropertyPage(const CommonProperty& commonProperty, int identity):
    PropertyPageWidget(commonProperty.getName(), commonProperty.getDescription(), identity)
{
    show(commonProperty);
}

CommonPropertyPage::CommonPropertyPage(Wt::WString caption, Wt::WString description, int identity):
    PropertyPageWidget(std::move(caption), std::move(description), identity)
{
    setOverflow(Wt::Overflow::Auto);
}

void CommonPropertyPage::show(const CommonProperty& commonProperty)
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);

    auto propertyView = vbox->addWidget(std::make_unique<CommonPropertyView>(
        commonProperty, CommonPropertyView::DEFAULT_ATTRIBUTES ^ CommonPropertyView::SHOW_HEADER));
    
    propertyView->connectChanged([this]{
        setDirty(true);
        refresh();
    });

    propertyView->connectEvent([this](boost::any any){
        handleEvent(any);
    });
}

void CommonPropertyPage::refresh()
{
    for (auto widget : children())
    {
        widget->refresh();
    }
}

bool CommonPropertyPage::validate()
{
    CommonPropertyView* propertyView = nullptr;
    for (auto widget : children())
    {
        if (propertyView = dynamic_cast<CommonPropertyView*>(widget))
        {
            break;          
        }
    }

    if (!propertyView)
    {
        return true;
    }

    Wt::WString message;
    if (propertyView->validate(message))
    {
        return true;
    }

    auto propertyBox = new PropertyBox(Dialog::BTN_CLOSE, L"Validation", message, Wt::Icon::Warning);
    propertyBox->show();
    return false;
}

void CommonPropertyPage::handleEvent(boost::any any)
{
    if (auto commonProperty = boost::any_cast<CommonProperty>(&any))
    {
        m_propertySheet->pushPage(std::make_unique<CommonPropertyPage>(*commonProperty));
    }
}