#include "stdafx.h"
#include "PropertyNodeFactory.h"
#include "Application.h"

CheckPropertyFactory createPropertyFactory(const TR::XML::XmlCheckDef& checkDef)
{
    return CheckPropertyFactory(checkDef.get_caption(), checkDef.get_description());
}

NumberPropertyFactory createPropertyFactory(const TR::XML::XmlNumberDef& numberDef)
{
    return NumberPropertyFactory(numberDef.get_caption(), numberDef.get_description());
}

StringPropertyFactory createPropertyFactory(const TR::XML::XmlStringLineDef& stringLineDef)
{
    auto attributes = StringProperty::DEFAULT_ATTRIBUTES;
    if (stringLineDef.is_secret())
    {
        attributes = attributes | STRING_ATTRIBUTES(SECRET);
    }
    return StringPropertyFactory(stringLineDef.get_caption(), stringLineDef.get_description(), attributes);
}

TextPropertyFactory createPropertyFactory(const TR::XML::XmlTextAreaDef& textAreaDef)
{
    return TextPropertyFactory(textAreaDef.get_caption(), textAreaDef.get_description(), textAreaDef.get_line_count());
}

SelectorPropertyFactory createPropertyFactory(const TR::XML::XmlSelectorDef& selectorDef)
{
    std::vector<std::wstring> items;
    for (auto& item_node : selectorDef.get_items())
    {
        items.push_back(item_node.get_name());
    }

    return SelectorPropertyFactory(selectorDef.get_caption(), selectorDef.get_description(), std::move(items));
}

struct AppendMemberFactory: boost::static_visitor<void>
{
    AppendMemberFactory(CommonPropertyFactory& collectionFactory):
        m_collectionFactory(collectionFactory)
    {
    }

    template<class XmlPropertyDef>
    void operator()(const XmlPropertyDef& property_def)
    {
        m_collectionFactory.addFactories(createPropertyFactory(property_def));
    }

    CommonPropertyFactory& m_collectionFactory;
};

CommonPropertyFactory createPropertyFactory(const TR::XML::XmlPropertyCollectionDef& collectionDef)
{
    CommonPropertyFactory commonFactory(collectionDef.get_caption(), collectionDef.get_description());
    for (auto& memberDef : collectionDef.get_members_def())
    {
        visit_property_def(AppendMemberFactory(commonFactory), memberDef);
    };
    return commonFactory;
}

std::shared_ptr<ValueCollection> createDefaultValue(const TR::XML::XmlPropertyCollectionDef& collectionDef);
CommonPropertyArrayFactory createPropertyFactory(const TR::XML::XmlPropertyListDef& listDef)
{
    auto& itemDef = listDef.get_item_def();

    return CommonPropertyArrayFactory(listDef.get_caption(), listDef.get_description(),
        createPropertyFactory(itemDef),
        std::dynamic_pointer_cast<ValueCollection>(createDefaultValue(itemDef)));
}

std::shared_ptr<SingleValue<bool>> createDefaultValue(const TR::XML::XmlCheckDef& checkDef)
{
    return staticValue(false);
}

std::shared_ptr<SingleValue<int>> createDefaultValue(const TR::XML::XmlNumberDef& numberDef)
{
    return staticValue(0);
}

std::shared_ptr<SingleValue<std::wstring>> createDefaultValue(const TR::XML::XmlStringLineDef& stringLineDef)
{
    return staticValue(std::wstring(L""));
}

std::shared_ptr<SingleValue<std::wstring>> createDefaultValue(const TR::XML::XmlTextAreaDef& textAreaDef)
{
    return staticValue(std::wstring(L""));
}

std::shared_ptr<SingleValue<int>> createDefaultValue(const TR::XML::XmlSelectorDef& selectorDef)
{
    return staticValue(0);
}

struct CreateMemberDefaultValue: boost::static_visitor<std::shared_ptr<Value>>
{
    template<class XmlPropertyDef>
    std::shared_ptr<Value> operator()(const XmlPropertyDef& property_def)
    {
        return createDefaultValue(property_def);
    }
};

std::shared_ptr<ValueCollection> createDefaultValue(const TR::XML::XmlPropertyCollectionDef& collectionDef)
{
    auto valueCollection = composedCollection({});
    for (auto& memberDef : collectionDef.get_members_def())
    {
        valueCollection->appendMember(visit_property_def(CreateMemberDefaultValue(), memberDef));
    }
    return valueCollection;
}

std::shared_ptr<ValueArray<ValueCollection>> createDefaultValue(const TR::XML::XmlPropertyListDef& listDef)
{
    return selfArray<ValueCollection>();
}