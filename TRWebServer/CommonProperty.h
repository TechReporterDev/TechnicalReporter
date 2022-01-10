#pragma once
#include "TRWebServer.h"
#include "Property.h"
#include "Value.h"
#include <boost/preprocessor/variadic/to_list.hpp>
#include <boost/preprocessor/list/cat.hpp>
#include <boost/preprocessor/list/transform.hpp>

class CheckProperty: public SingleProperty<SingleValue<bool>>
{
public:
    CheckProperty(std::wstring name, std::wstring description, std::shared_ptr<SingleValue<bool>> value, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);
    
    bool    getChecked() const;
    void    setChecked(bool checked);
};

class CheckPropertyFactory: public SinglePropertyFactory<CheckProperty>
{
public:
    CheckPropertyFactory(std::wstring name, std::wstring description, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);
    CheckProperty createProperty(std::shared_ptr<SingleValue<bool>> value) const;
};

class CheckPropertyEx: public SingleProperty<SingleValue<bool>>
{
public:
    using ExtensionFactory = std::function<boost::any(bool)>;

    CheckPropertyEx(std::wstring name, std::wstring description, ExtensionFactory extensionFactory, std::shared_ptr<SingleValue<bool>> value, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);
    
    bool        getChecked() const;
    void        setChecked(bool checked);
    boost::any  getExtension() const;

private:
    ExtensionFactory m_extensionFactory;
};

class CheckPropertyExFactory: public SinglePropertyFactory<CheckPropertyEx>
{
public:
    using ExtensionFactory = CheckPropertyEx::ExtensionFactory;

    CheckPropertyExFactory(std::wstring name, std::wstring description, ExtensionFactory extensionFactory, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);
    CheckPropertyEx createProperty(std::shared_ptr<SingleValue<bool>> value) const;

private:
    ExtensionFactory m_extensionFactory;
};

class NumberProperty: public SingleProperty<SingleValue<int>>
{
public:
    NumberProperty(std::wstring name, std::wstring description, std::shared_ptr<SingleValue<int>> value, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);

    int     getNumber() const;
    void    setNumber(int number);
};

std::unique_ptr<NumberProperty> numberProperty(std::wstring name, std::wstring description, std::shared_ptr<SingleValue<int>> value, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);

class NumberPropertyFactory: public SinglePropertyFactory<NumberProperty>
{
public:
    NumberPropertyFactory(std::wstring name, std::wstring description, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);
    NumberProperty createProperty(std::shared_ptr<SingleValue<int>> value) const;
};

class StringProperty: public SingleProperty<SingleValue<std::wstring>>
{
public:
    enum Attribute { DEFAULT = 0x0000, READ_ONLY = 0x0001, NO_PREVIEW = 0x0002, SECRET = 0x0100, NOT_EMPTY = 0x0200 };  
    using Attributes = stl_tools::flag_type<Attribute>;
    static Attributes DEFAULT_ATTRIBUTES;

    StringProperty(std::wstring name, std::wstring description, std::shared_ptr<SingleValue<std::wstring>> value, Attributes attributes = DEFAULT_ATTRIBUTES);
    StringProperty(std::wstring name, std::wstring description, std::wstring prompt, std::shared_ptr<SingleValue<std::wstring>> value, Attributes attributes = DEFAULT_ATTRIBUTES);

    std::wstring    getString() const;
    void            setString(const std::wstring& string);
    bool            isSecret() const;
    std::wstring    getPrompt() const;
    bool            NotEmpty() const;

private:
    std::wstring m_prompt;
    Attributes m_attributes;
};

#define MAKE_ATTRIBUTE(d, data, elem) | data##::##elem
#define STRING_ATTRIBUTES(...) (stl_tools::flag BOOST_PP_LIST_CAT(BOOST_PP_LIST_TRANSFORM(MAKE_ATTRIBUTE, StringProperty, BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))))

class StringPropertyFactory: public SinglePropertyFactory<StringProperty>
{
public:
    StringPropertyFactory(std::wstring name, std::wstring description, StringProperty::Attributes attributes = StringProperty::DEFAULT_ATTRIBUTES);
    StringProperty createProperty(std::shared_ptr<SingleValue<std::wstring>> value) const;

private:
    StringProperty::Attributes m_attributes;
};

class TextProperty: public SingleProperty<SingleValue<std::wstring>>
{
public:
    TextProperty(std::wstring name, std::wstring description, int lineCount, std::shared_ptr<SingleValue<std::wstring>> value, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);

    int             getLineCount() const;
    std::wstring    getText() const;
    void            setText(const std::wstring& text);

private:
    int m_lineCount;
};

class TextPropertyFactory: public SinglePropertyFactory<TextProperty>
{
public:
    TextPropertyFactory(std::wstring name, std::wstring description, int lineCount, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);
    TextProperty    createProperty(std::shared_ptr<SingleValue<std::wstring>> value) const;

private:
    int m_lineCount;
};

class AnyProperty: public SingleProperty<SingleValue<boost::any>>
{
public:
    AnyProperty(std::wstring name, std::wstring description, std::wstring caption, std::shared_ptr<SingleValue<boost::any>> value);
    std::wstring    getCaption() const;
    boost::any      getAny() const;

private:
    std::wstring m_caption;
};

class AnyPropertyFactory: public SinglePropertyFactory<AnyProperty>
{
public:
    AnyPropertyFactory(std::wstring name, std::wstring description, std::wstring caption);
    AnyProperty createProperty(std::shared_ptr<SingleValue<boost::any>> value) const;

private:
    std::wstring m_caption;
};

class SelectorProperty: public SingleProperty<SingleValue<int>>
{
public:
    SelectorProperty(std::wstring name, std::wstring description, std::vector<std::wstring> items, std::shared_ptr<SingleValue<int>> value, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);
    
    std::vector<std::wstring>       getItems() const;
    int                             getSelection() const;
    void                            setSelection(int selection);

private:
    std::vector<std::wstring> m_items;
};

class SelectorPropertyFactory: public SinglePropertyFactory<SelectorProperty>
{
public:
    SelectorPropertyFactory(std::wstring name, std::wstring description, std::vector<std::wstring> items, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);
    SelectorProperty createProperty(std::shared_ptr<SingleValue<int>> value) const;

private:
    std::vector<std::wstring> m_items;
};

class SwitchProperty: public SingleProperty<SingleValue<int>>
{
public:
    SwitchProperty(std::wstring name, std::wstring description, std::vector<std::wstring> cases, std::shared_ptr<SingleValue<int>> value, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);

    std::vector<std::wstring>   getCases() const;
    int                         getCase() const;
    void                        setCase(int case_);

private:
    std::vector<std::wstring> m_cases;
};

class SwitchPropertyFactory: public SinglePropertyFactory<SwitchProperty>
{
public:
    SwitchPropertyFactory(std::wstring name, std::wstring description, std::vector<std::wstring> cases, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);
    SwitchProperty createProperty(std::shared_ptr<SingleValue<int>> value) const;

private:
    std::vector<std::wstring> m_cases;
};

class SwitchPropertyEx: public SingleProperty<SingleValue<int>>
{
public:
    struct Case
    {
        Case(std::wstring name, std::wstring eventName, boost::any eventIde) :
            m_name(std::move(name)),
            m_eventName(std::move(eventName)),
            m_eventId(std::move(eventIde))
        {
        }

        Case(std::wstring name, boost::any eventId = boost::none):
            m_name(std::move(name)),
            m_eventName(L"Setup"),
            m_eventId(std::move(eventId))
        {
        }

        std::wstring    m_name;
        std::wstring    m_eventName;
        boost::any      m_eventId;
    };

    SwitchPropertyEx(std::wstring name, std::wstring description, std::vector<Case> cases, std::shared_ptr<SingleValue<int>> value, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);

    std::vector<Case>           getCases() const;
    int                         getCase() const;
    void                        setCase(int case_);

private:
    std::vector<Case> m_cases;
};

class SwitchCommonProperty: public SingleProperty<SingleValue<int>>
{
public:
    using Case = std::tuple<std::wstring, ComposedProperty, ComposedProperty>;
    SwitchCommonProperty(std::wstring name, std::wstring description, std::vector<Case> cases, std::shared_ptr<SingleValue<int>> value, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);

    std::vector<Case>           getCases() const;
    int                         getCase() const;
    void                        setCase(int case_);

private:
    std::vector<Case> m_cases;
};

class DynamicSelectorProperty: public SingleProperty<ValueCollection>
{
public:
    DynamicSelectorProperty(std::wstring name, std::wstring description, std::shared_ptr<ValueCollection> valueCollection, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);

    std::vector<std::wstring>       getItems() const;
    int                             getSelection() const;
    void                            setSelection(int selection);
};

class DynamicSelectorPropertyFactory: public SinglePropertyFactory<DynamicSelectorProperty>
{
public:
    DynamicSelectorPropertyFactory(std::wstring name, std::wstring description, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);
    DynamicSelectorProperty createProperty(std::shared_ptr<ValueCollection> valueCollection) const;

private:
    std::vector<std::wstring> m_items;
};

class SelectorPropertyEx: public SingleProperty<SingleValue<int>>
{
public:
    using ExpandHandler = std::function<boost::any(int value)>;

    SelectorPropertyEx(std::wstring name, std::wstring description, std::wstring caption, ExpandHandler expandHandler, std::vector<std::wstring> items, std::shared_ptr<SingleValue<int>> value, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);

    std::vector<std::wstring>       getItems() const;
    int                             getSelection() const;
    void                            setSelection(int selection);
    std::wstring                    getExpandCaption() const;
    boost::any                      expand() const;

private:
    std::vector<std::wstring> m_items;
    std::wstring m_caption;
    ExpandHandler m_expandHandler;
};

std::unique_ptr<SelectorPropertyEx> selectorPropertyEx(std::wstring name, std::wstring description, std::wstring caption, SelectorPropertyEx::ExpandHandler expandHandler, std::vector<std::wstring> items, std::shared_ptr<SingleValue<int>> value, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);

class SelectorPropertyExFactory: public SinglePropertyFactory<SelectorPropertyEx>
{
public:
    using ExpandHandler = SelectorPropertyEx::ExpandHandler;

    SelectorPropertyExFactory(std::wstring name, std::wstring description, std::wstring caption, ExpandHandler expandHandler, std::vector<std::wstring> items, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);
    SelectorPropertyEx createProperty(std::shared_ptr<SingleValue<int>> value) const;

private:
    std::vector<std::wstring> m_items;
    std::wstring m_caption;
    ExpandHandler m_expandHandler;
};

#define CHECKLIST_ATTRIBUTES(...) (stl_tools::flag BOOST_PP_LIST_CAT(BOOST_PP_LIST_TRANSFORM(MAKE_ATTRIBUTE, CheckListProperty, BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))))
class CheckListProperty: public SingleProperty<ValueArray<SingleValue<bool>>>
{
public:
    enum Attribute { DEFAULT = 0x0000, READ_ONLY = 0x0001, CHECK_REQUIRED = 0x0100 };
    using Attributes = stl_tools::flag_type<Attribute>;
    static Attributes DEFAULT_ATTRIBUTES;

    CheckListProperty(std::wstring name, std::wstring description, std::vector<std::wstring> items, std::shared_ptr<ValueArray<SingleValue<bool>>> checks, Attributes attributes = DEFAULT_ATTRIBUTES);

    bool                                isChecked(size_t position) const;
    void                                setChecked(size_t position, bool checked = true);
    const std::vector<std::wstring>&    getItems() const;
    bool                                checkRequired() const;

private:
    std::vector<std::wstring> m_items;
    Attributes m_attributes;
};

class CheckListPropertyFactory: public SinglePropertyFactory<CheckListProperty>
{
public:
    CheckListPropertyFactory(std::wstring name, std::wstring description, std::vector<std::wstring> items);
    CheckListProperty createProperty(std::shared_ptr<ValueArray<SingleValue<bool>>> checks) const;

private:
    std::vector<std::wstring> m_items;
};

#define SELECTIONLIST_ATTRIBUTES(...) (stl_tools::flag BOOST_PP_LIST_CAT(BOOST_PP_LIST_TRANSFORM(MAKE_ATTRIBUTE, SelectionListProperty, BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))))
class SelectionListProperty: public SingleProperty<SingleValue<int>>
{
public:
    enum Attribute { DEFAULT = 0x0000, READ_ONLY = 0x0001, SELECTION_REQUIRED = 0x0100 };
    using Attributes = stl_tools::flag_type<Attribute>;
    static Attributes DEFAULT_ATTRIBUTES;
    static const int INVALID_SELECTION = -1;

    SelectionListProperty(std::wstring name, std::wstring description, std::vector<std::wstring> items, std::shared_ptr<SingleValue<int>> value, Attributes attributes = DEFAULT_ATTRIBUTES);

    size_t                              getSelection() const;
    void                                select(size_t position);
    const std::vector<std::wstring>&    getItems() const;
    bool                                isSelectionRequired() const;

private:
    std::vector<std::wstring> m_items;
    Attributes m_attributes;
};

std::unique_ptr<SelectionListProperty> selectionListProperty(std::wstring name, std::wstring description, std::vector<std::wstring> items, std::shared_ptr<SingleValue<int>> value, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);

class SelectionListPropertyFactory: public SinglePropertyFactory<SelectionListProperty>
{
public:
    using Attributes = SelectionListProperty::Attributes;

    SelectionListPropertyFactory(std::wstring name, std::wstring description, std::vector<std::wstring> items, Attributes attributes = SelectionListProperty::DEFAULT_ATTRIBUTES);
    SelectionListProperty createProperty(std::shared_ptr<SingleValue<int>> value) const;

private:
    std::vector<std::wstring> m_items;
    Attributes m_attributes;
};

class DateProperty: public SingleProperty<SingleValue<std::tm>>
{
public:
    DateProperty(std::wstring name, std::wstring description, std::shared_ptr<SingleValue<std::tm>> value, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);

    std::tm     getDate() const;
    void        setDate(std::tm date);
};

class DatePropertyFactory: public SinglePropertyFactory<DateProperty>
{
public:
    DatePropertyFactory(std::wstring name, std::wstring description, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);
    DateProperty createProperty(std::shared_ptr<SingleValue<tm>> value) const;
};

class TimeProperty: public SingleProperty<SingleValue<std::tm>>
{
public:
    TimeProperty(std::wstring name, std::wstring description, std::shared_ptr<SingleValue<std::tm>> value, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);

    std::tm     getTime() const;
    void        setTime(std::tm time);
};

std::unique_ptr<TimeProperty> timeProperty(std::wstring name, std::wstring description, std::shared_ptr<SingleValue<std::tm>> value, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);

class TimePropertyFactory: public SinglePropertyFactory<TimeProperty>
{
public:
    TimePropertyFactory(std::wstring name, std::wstring description, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);
    TimeProperty createProperty(std::shared_ptr<SingleValue<tm>> value) const;
};

class BrowseProperty: public SingleProperty<SingleValue<boost::any>>
{
public:
    using FormatFunctor = std::function<std::wstring(boost::any)>;
    BrowseProperty(std::wstring name, std::wstring description, std::wstring caption, boost::any subject, FormatFunctor formatFunctor, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);

    std::wstring    getCaption() const;
    boost::any      getSubject() const;
    std::wstring    formatString() const;

private:
    std::wstring m_caption;
    FormatFunctor m_formatFunctor;
};


class OptionalProperty
{
public:
    using OptionalPredicate = std::function<bool()>;

    OptionalProperty(OptionalPredicate optionalPredicate, ComposedProperty innerProperty);

    bool                                isReady() const;
    ComposedProperty                    getInnerProperty() const;
    std::shared_ptr<ValueCollection>    shareValue() const;

private:
    OptionalPredicate m_optionalPredicate;
    ComposedProperty m_innerProperty;
};

using CommonProperty = ComposedProperty;
using CommonPropertyFactory = ComposedPropertyFactory;
using CommonPropertyArray = PropertyArray<CommonPropertyFactory>;
using CommonPropertyArrayFactory = PropertyArrayFactory<CommonPropertyArray>;


template<class Visitor>
typename Visitor::result_type visitMemberProperty(Visitor visitor, const CommonProperty& commonProperty, size_t position)
{
    if (auto checkProperty = commonProperty.getProperty<CheckProperty*>(position))
    {
        return visitor(*checkProperty);
    }
    else if (auto checkPropertyEx = commonProperty.getProperty<CheckPropertyEx*>(position))
    {
        return visitor(*checkPropertyEx);
    }
    else if (auto mumberProperty = commonProperty.getProperty<NumberProperty*>(position))
    {
        return visitor(*mumberProperty);
    }
    else if (auto stringProperty = commonProperty.getProperty<StringProperty*>(position))
    {
        return visitor(*stringProperty);
    }
    else if (auto textProperty = commonProperty.getProperty<TextProperty*>(position))
    {
        return visitor(*textProperty);
    }
    else if (auto anyProperty = commonProperty.getProperty<AnyProperty*>(position))
    {
        return visitor(*anyProperty);
    }
    else if (auto selectorProperty = commonProperty.getProperty<SelectorProperty*>(position))
    {
        return visitor(*selectorProperty);
    }
    else if (auto selectorPropertyEx = commonProperty.getProperty<SelectorPropertyEx*>(position))
    {
        return visitor(*selectorPropertyEx);
    }
    else if (auto switchProperty = commonProperty.getProperty<SwitchProperty*>(position))
    {
        return visitor(*switchProperty);
    }
    else if (auto switchPropertyEx = commonProperty.getProperty<SwitchPropertyEx*>(position))
    {
        return visitor(*switchPropertyEx);
    }
    else if (auto switchCommonProperty = commonProperty.getProperty<SwitchCommonProperty*>(position))
    {
        return visitor(*switchCommonProperty);
    }
    else if (auto dynamicSelectorProperty = commonProperty.getProperty<DynamicSelectorProperty*>(position))
    {
        return visitor(*dynamicSelectorProperty);
    }
    else if (auto checkListProperty = commonProperty.getProperty<CheckListProperty*>(position))
    {
        return visitor(*checkListProperty);
    }
    else if (auto selectionListProperty = commonProperty.getProperty<SelectionListProperty*>(position))
    {
        return visitor(*selectionListProperty);
    }
    else if (auto dateProperty = commonProperty.getProperty<DateProperty*>(position))
    {
        return visitor(*dateProperty);
    }
    else if (auto timeProperty = commonProperty.getProperty<TimeProperty*>(position))
    {
        return visitor(*timeProperty);
    }
    else if (auto browseProperty = commonProperty.getProperty<BrowseProperty*>(position))
    {
        return visitor(*browseProperty);
    }
    else if (auto optionalProperty = commonProperty.getProperty<OptionalProperty*>(position))
    {
        return visitor(*optionalProperty);
    }
    else if (auto ppt = commonProperty.getProperty<CommonProperty*>(position))
    {
        return visitor(*ppt);
    }
    else if (auto ppt = commonProperty.getProperty<PropertyArray<CommonPropertyFactory>*>(position))
    {
        return visitor(*ppt);
    }
    throw std::logic_error("Invalid property type");
}


