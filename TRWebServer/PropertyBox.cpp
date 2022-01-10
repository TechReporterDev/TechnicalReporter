#include "stdafx.h"
#include "PropertyBox.h"
#include "CommonPropertyPage.h"
#include "StaticValue.h"
#include "Application.h"

namespace {

bool isSingleLine(const Wt::WString& message)
{
    return message.value().find(L"<br/>") == std::wstring::npos;
}

} //namespace {

class Message: public Wt::WContainerWidget
{
public:
    Message::Message(Wt::Icon icon, const Wt::WString& message)
    {
        setStyleClass(L"tr-header");
        setMargin(10);

        const char* iconName = nullptr;
        switch (icon)
        {
        case Wt::Icon::Information:
            iconName = "info";
            break;

        case Wt::Icon::Warning:
            iconName = "warning";
            break;

        case Wt::Icon::Critical:
            iconName = "exclamation";
            break;

        case Wt::Icon::Question:
            iconName = "question";
            break;

        case Wt::Icon::None:
            break;

        default:
            _ASSERT(false);
        }

        if (iconName)
        {
            auto iconWidget = addWidget(std::make_unique<Wt::WText>());
            iconWidget->setFloatSide(Wt::Side::Left);
            iconWidget->setStyleClass(std::string("fa fa-") + iconName);
            iconWidget->decorationStyle().font().setSize(Wt::WLength(2.5, Wt::LengthUnit::FontEm));
        }

        auto messageText = addWidget(std::make_unique<Wt::WText>(message));
        messageText->setInline(false);

        if (isSingleLine(message))
        {
            messageText->setLineHeight(35);
        }

        if (iconName)
        {
            messageText->setMinimumSize(100, 35);
            messageText->setMargin(35, Wt::Side::Left);
        }
    }
};

PropertyBox::PropertyBox(stl_tools::flag_type<DialogButton> buttons, std::wstring title, std::wstring message, Wt::Icon icon):
    m_commonProperty(L"", L"")
{
    setWindowTitle(title);
    rejectWhenEscapePressed();
    setButtons(buttons);

    contents()->setPadding(10);
    footer()->setMargin(0, Wt::Side::Top);

    if (!message.empty())
    {
        auto container = contents()->addWidget(std::make_unique<Wt::WContainerWidget>());
        auto vbox = container->setLayout(std::make_unique<Wt::WVBoxLayout>());
        vbox->addWidget(std::make_unique<Message>(icon, message), 1);        
    }
}

void PropertyBox::addString(std::wstring name, std::wstring description, std::wstring value, std::wstring prompt)
{
    m_commonProperty.addProperties(StringProperty(std::move(name), std::move(description), std::move(prompt), staticValue(std::move(value))));
}

void PropertyBox::addSwitch(std::wstring name, std::wstring description, std::vector<std::wstring> cases, int value)
{
    m_commonProperty.addProperties(SwitchProperty(std::move(name), std::move(description), cases, staticValue(std::move(value))));
}

void PropertyBox::addCheck(std::wstring name, std::wstring description, bool value)
{
    m_commonProperty.addProperties(CheckProperty(std::move(name), std::move(description), staticValue(std::move(value))));
}

void PropertyBox::addText(std::wstring name, std::wstring description, std::wstring value, bool readOnly)
{
    m_commonProperty.addProperties(TextProperty(std::move(name), std::move(description), 10, staticValue(std::move(value)), PropertyAttribute::READ_ONLY));
}

void PropertyBox::show(std::function<void()> accept_handler, std::function<void()> reject_handler)
{
    m_accept_handler = accept_handler;
    m_reject_handler = reject_handler;
    
    auto propertySheet = contents()->addWidget(std::make_unique<PropertySheetWidget>(PropertySheetWidget::DEFAULT_ATTRIBUTES ^ PropertySheetWidget::SHOW_HEADER));
    propertySheet->pushPage(std::make_unique<CommonPropertyPage>(m_commonProperty));
    Dialog::show();
}

void PropertyBox::apply()
{
    if (m_accept_handler)
    {
        m_accept_handler();
    }
}

void PropertyBox::cancel()
{
    if (m_reject_handler)
    {
        m_reject_handler();
    }
}