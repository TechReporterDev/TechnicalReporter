#pragma once
#include "TRWebServer.h"
#include "CommonProperty.h"

class PropertyView: public Wt::WContainerWidget
{
public:
    PropertyView();

    template<class Handler>
    void connectChanged(Handler handler)
    {
        m_changed.connect(std::bind(handler));
    }

    template<class Handler>
    void connectEvent(Handler handler)
    {
        m_event.connect(std::bind(handler, std::placeholders::_1));
    }

    virtual bool validate(Wt::WString& message)
    {
        return true;
    }

protected:
    Wt::Signal<> m_changed;
    Wt::Signal<boost::any> m_event;
};

class DetailedPropertyView: public PropertyView
{
public:
    DetailedPropertyView();

protected:
    void showTitle(const Wt::WString& name, const Wt::WString& description);

    Wt::WVBoxLayout* m_layout;
    Wt::WLabel* m_nameLabel;
};

class CheckPropertyView: public DetailedPropertyView
{
public:
    CheckPropertyView(const CheckProperty& checkProperty);

private:
    CheckProperty m_checkProperty;
};

class CheckPropertyExView: public DetailedPropertyView
{
public:
    CheckPropertyExView(const CheckPropertyEx& checkPropertyEx);

private:
    void showExtension();
    CheckPropertyEx m_checkPropertyEx;
    Wt::WWidget* m_extensionView;
};

class NumberPropertyView: public DetailedPropertyView
{
public:
    NumberPropertyView(const NumberProperty& numberProperty);

private:
    NumberProperty m_numberProperty;
};

class StringPropertyView: public DetailedPropertyView
{
public:
    StringPropertyView(const StringProperty& stringProperty);

    // PropertyView override
    virtual bool validate(Wt::WString& message) override;

private:
    StringProperty m_stringProperty;
};

class TextPropertyView: public DetailedPropertyView
{
public:
    TextPropertyView(const TextProperty& textProperty);

private:
    TextProperty m_textProperty;
};

class SelectorPropertyView: public DetailedPropertyView
{
public:
    SelectorPropertyView(const SelectorProperty& selectorProperty);

private:
    SelectorProperty m_selectorProperty;
    Wt::WPushButton* m_expandButton;
};

class SelectorPropertyExView: public DetailedPropertyView
{
public:
    SelectorPropertyExView(const SelectorPropertyEx& selectorPropertyEx);

private:
    SelectorPropertyEx m_selectorPropertyEx;
    Wt::WPushButton* m_expandButton;
};

class SwitchPropertyView: public DetailedPropertyView
{
public:
    SwitchPropertyView(const SwitchProperty& switchProperty);

private:
    SwitchProperty m_switchProperty;
};

class SwitchPropertyExView: public DetailedPropertyView
{
public:
    SwitchPropertyExView(const SwitchPropertyEx& switchProperty);

private:
    SwitchPropertyEx m_switchProperty;
};

class SwitchCommonPropertyView: public DetailedPropertyView
{
public:
    SwitchCommonPropertyView(const SwitchCommonProperty& switchProperty);

private:
    void showCaseView(int position);

    SwitchCommonProperty m_switchProperty;
    std::vector<Wt::WContainerWidget*> m_caseContainers;
};

class CheckListPropertyView: public DetailedPropertyView
{
public:
    CheckListPropertyView(const CheckListProperty& checkListProperty);

    // PropertyView override
    virtual bool validate(Wt::WString& message) override;

private:
    CheckListProperty m_checkListProperty;
};

class SelectionListPropertyView: public DetailedPropertyView
{
public:
    SelectionListPropertyView(const SelectionListProperty& selectionListProperty);

    // PropertyView override
    virtual bool validate(Wt::WString& message) override;

private:
    SelectionListProperty m_selectionListProperty;
};

class DatePropertyView: public DetailedPropertyView
{
public:
    DatePropertyView(const DateProperty& dateProperty);

private:
    DateProperty m_dateProperty;
};

class TimePropertyView: public DetailedPropertyView
{
public:
    TimePropertyView(const TimeProperty& timeProperty);

private:
    TimeProperty m_timeProperty;
};

class AnyPropertyView: public DetailedPropertyView
{
public:
    AnyPropertyView(const AnyProperty& anyProperty);    

protected:
    AnyProperty m_anyProperty;
};

class BrowsePropertyView: public DetailedPropertyView
{
public:
    BrowsePropertyView(const BrowseProperty& browseProperty);
    virtual void refresh() override;

protected:  
    BrowseProperty m_browseProperty;
    Wt::WLineEdit* m_edit;
};

class OptionalPropertyView : public PropertyView
{
public:
    OptionalPropertyView(const OptionalProperty& optionalProperty);
    virtual void refresh() override;

protected:
    OptionalProperty m_optionalProperty;
    PropertyView* m_optionalView;
};

class CommonPropertyView: public PropertyView
{
public:
    enum Attribute { SHOW_HEADER = 0x01, EXPAND_SUBPROPS = 0x02 };
    using Attributes = Wt::WFlags<Attribute>;
    const static Attributes DEFAULT_ATTRIBUTES; //SHOW_HEADER | EXPAND_SUBPROPS;

    CommonPropertyView(const CommonProperty& commonProperty, Attributes attributes = DEFAULT_ATTRIBUTES);
    
    // WWidget override
    virtual void refresh() override;

    // PropertyView override
    virtual bool validate(Wt::WString& message) override;
};