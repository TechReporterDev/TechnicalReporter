#include "stdafx.h"
#include "CommonProperty.h"

CheckProperty::CheckProperty(std::wstring name, std::wstring description, std::shared_ptr<SingleValue<bool>> value, PropertyAttributes attributes):
    SingleProperty<SingleValue<bool>>(std::move(name), std::move(description), value, attributes)
{
}

bool CheckProperty::getChecked() const
{
    return m_value->get();
}

void CheckProperty::setChecked(bool checked)
{
    m_value->set(checked);
}

std::unique_ptr<CheckProperty> checkProperty(std::wstring name, std::wstring description, std::shared_ptr<SingleValue<bool>> value, PropertyAttributes attributes)
{
    return std::make_unique<CheckProperty>(std::move(name), std::move(description), value, attributes);
}

CheckPropertyFactory::CheckPropertyFactory(std::wstring name, std::wstring description, PropertyAttributes attributes):
    SinglePropertyFactory<CheckProperty>(std::move(name), std::move(description), attributes)
{
}

std::unique_ptr<CheckPropertyFactory> checkPropertyFactory(std::wstring name, std::wstring description, PropertyAttributes attributes)
{
    return std::make_unique<CheckPropertyFactory>(std::move(name), std::move(description), attributes);
}

CheckProperty CheckPropertyFactory::createProperty(std::shared_ptr<SingleValue<bool>> value) const
{
    return CheckProperty(m_name, m_description, value, m_attributes);
}

CheckPropertyEx::CheckPropertyEx(std::wstring name, std::wstring description, ExtensionFactory extensionFactory, std::shared_ptr<SingleValue<bool>> value, PropertyAttributes attributes):
    SingleProperty<SingleValue<bool>>(std::move(name), std::move(description), value, attributes),
    m_extensionFactory(extensionFactory)
{
}

bool CheckPropertyEx::getChecked() const
{
    return m_value->get();
}

void CheckPropertyEx::setChecked(bool checked)
{
    m_value->set(checked);
}

boost::any CheckPropertyEx::getExtension() const
{
    return m_extensionFactory(m_value->get());
}

CheckPropertyExFactory::CheckPropertyExFactory(std::wstring name, std::wstring description, ExtensionFactory extensionFactory, PropertyAttributes attributes):
    SinglePropertyFactory<CheckPropertyEx>(std::move(name), std::move(description), attributes),
    m_extensionFactory(std::move(extensionFactory))
{
}

CheckPropertyEx CheckPropertyExFactory::createProperty(std::shared_ptr<SingleValue<bool>> value) const
{
    return CheckPropertyEx(m_name, m_description, m_extensionFactory, value, m_attributes);
}

NumberProperty::NumberProperty(std::wstring name, std::wstring description, std::shared_ptr<SingleValue<int>> value, PropertyAttributes attributes):
    SingleProperty<SingleValue<int>>(std::move(name), std::move(description), value, attributes)
{
};

int NumberProperty::getNumber() const
{
    return m_value->get();
}

void NumberProperty::setNumber(int number)
{
    m_value->set(number);
}

NumberPropertyFactory::NumberPropertyFactory(std::wstring name, std::wstring description, PropertyAttributes attributes):
    SinglePropertyFactory<NumberProperty>(std::move(name), std::move(description), attributes)
{
}

NumberProperty NumberPropertyFactory::createProperty(std::shared_ptr<SingleValue<int>> value) const
{
    return NumberProperty(m_name, m_description, value, m_attributes);
}

StringProperty::Attributes StringProperty::DEFAULT_ATTRIBUTES = STRING_ATTRIBUTES(DEFAULT);

StringProperty::StringProperty(std::wstring name, std::wstring description, std::shared_ptr<SingleValue<std::wstring>> value, Attributes attributes):
    SingleProperty<SingleValue<std::wstring>>(std::move(name), std::move(description), value, PropertyAttribute(attributes.get_value() & 0xFF)),
    m_attributes(attributes)
{
}

StringProperty::StringProperty(std::wstring name, std::wstring description, std::wstring prompt, std::shared_ptr<SingleValue<std::wstring>> value, Attributes attributes):
    SingleProperty<SingleValue<std::wstring>>(std::move(name), std::move(description), value, PropertyAttribute(attributes.get_value() & 0xFF)),
    m_prompt(std::move(prompt)),
    m_attributes(attributes)
{
}

std::wstring StringProperty::getString() const
{
    return m_value->get();
}

void StringProperty::setString(const std::wstring& string)
{
    m_value->set(string);
}

bool StringProperty::isSecret() const
{
    return m_attributes.contains(SECRET);
}

std::wstring StringProperty::getPrompt() const
{
    return m_prompt;
}

bool StringProperty::NotEmpty() const
{
    return m_attributes.contains(NOT_EMPTY);
}

StringPropertyFactory::StringPropertyFactory(std::wstring name, std::wstring description, StringProperty::Attributes attributes):
    SinglePropertyFactory<StringProperty>(std::move(name), std::move(description), PropertyAttribute(attributes.get_value() & 0xFF)),
    m_attributes(attributes)
{
}

StringProperty StringPropertyFactory::createProperty(std::shared_ptr<SingleValue<std::wstring>> value) const
{
    return StringProperty(m_name, m_description, value, m_attributes);
}

TextProperty::TextProperty(std::wstring name, std::wstring description, int lineCount, std::shared_ptr<SingleValue<std::wstring>> value, PropertyAttributes attributes):
    SingleProperty<SingleValue<std::wstring>>(std::move(name), std::move(description), value, attributes),
    m_lineCount(lineCount)
{
}

std::wstring TextProperty::getText() const
{
    return m_value->get();
}

void TextProperty::setText(const std::wstring& text)
{
    m_value->set(text);
}

int TextProperty::getLineCount() const
{
    return m_lineCount;
}

TextPropertyFactory::TextPropertyFactory(std::wstring name, std::wstring description, int lineCount, PropertyAttributes attributes):
    SinglePropertyFactory<TextProperty>(std::move(name), std::move(description), attributes),
    m_lineCount(lineCount)
{
}

TextProperty TextPropertyFactory::createProperty(std::shared_ptr<SingleValue<std::wstring>> value) const
{
    return TextProperty(m_name, m_description, m_lineCount, value, m_attributes);
}

AnyProperty::AnyProperty(std::wstring name, std::wstring description, std::wstring caption, std::shared_ptr<SingleValue<boost::any>> value):
    SingleProperty<SingleValue<boost::any>>(std::move(name), std::move(description), value),
    m_caption(std::move(caption))
{
}

std::wstring AnyProperty::getCaption() const
{
    return m_caption;
}

boost::any AnyProperty::getAny() const
{
    return m_value->get();
}

AnyPropertyFactory::AnyPropertyFactory(std::wstring name, std::wstring description, std::wstring caption):
    SinglePropertyFactory<AnyProperty>(std::move(name), std::move(description)),
    m_caption(std::move(caption))
{
}

AnyProperty AnyPropertyFactory::createProperty(std::shared_ptr<SingleValue<boost::any>> value) const
{
    return AnyProperty(m_name, m_description, m_caption, value);
}

SelectorProperty::SelectorProperty(std::wstring name, std::wstring description, std::vector<std::wstring> items, std::shared_ptr<SingleValue<int>> value, PropertyAttributes attributes):
    SingleProperty<SingleValue<int>>(std::move(name), std::move(description), value, attributes),
    m_items(std::move(items))
{
}

std::vector<std::wstring> SelectorProperty::getItems() const
{
    return m_items;
}

int SelectorProperty::getSelection() const
{
    return m_value->get();
}

void SelectorProperty::setSelection(int selection)
{
    m_value->set(selection);
}

SelectorPropertyFactory::SelectorPropertyFactory(std::wstring name, std::wstring description, std::vector<std::wstring> items, PropertyAttributes attributes):
    SinglePropertyFactory<SelectorProperty>(std::move(name), std::move(description), attributes),
    m_items(std::move(items))
{
}

SelectorProperty SelectorPropertyFactory::createProperty(std::shared_ptr<SingleValue<int>> value) const
{
    return SelectorProperty(m_name, m_description, m_items, value, m_attributes);
}

SwitchProperty::SwitchProperty(std::wstring name, std::wstring description, std::vector<std::wstring> cases, std::shared_ptr<SingleValue<int>> value, PropertyAttributes attributes):
    SingleProperty<SingleValue<int>>(std::move(name), std::move(description), value, attributes),
    m_cases(std::move(cases))
{
}

std::vector<std::wstring> SwitchProperty::getCases() const
{
    return m_cases;
}

int SwitchProperty::getCase() const
{
    return m_value->get();
}

void SwitchProperty::setCase(int case_)
{
    m_value->set(case_);
}

SwitchPropertyFactory::SwitchPropertyFactory(std::wstring name, std::wstring description, std::vector<std::wstring> cases, PropertyAttributes attributes):
    SinglePropertyFactory<SwitchProperty>(std::move(name), std::move(description), attributes),
    m_cases(std::move(cases))
{
}

SwitchProperty SwitchPropertyFactory::createProperty(std::shared_ptr<SingleValue<int>> value) const
{
    return SwitchProperty(m_name, m_description, m_cases, value, m_attributes);
}

SwitchPropertyEx::SwitchPropertyEx(std::wstring name, std::wstring description, std::vector<Case> cases, std::shared_ptr<SingleValue<int>> value, PropertyAttributes attributes):
    SingleProperty<SingleValue<int>>(std::move(name), std::move(description), value, attributes),
    m_cases(std::move(cases))
{
}

std::vector<SwitchPropertyEx::Case> SwitchPropertyEx::getCases() const
{
    return m_cases;
}

int SwitchPropertyEx::getCase() const
{
    return m_value->get();
}

void SwitchPropertyEx::setCase(int case_)
{
    m_value->set(case_);
}

SwitchCommonProperty::SwitchCommonProperty(std::wstring name, std::wstring description, std::vector<Case> cases, std::shared_ptr<SingleValue<int>> value, PropertyAttributes attributes):
    SingleProperty<SingleValue<int>>(std::move(name), std::move(description), value, attributes),
    m_cases(std::move(cases))
{
}

std::vector<SwitchCommonProperty::Case> SwitchCommonProperty::getCases() const
{
    return m_cases;
}

int SwitchCommonProperty::getCase() const
{
    return m_value->get();
}

void SwitchCommonProperty::setCase(int case_)
{
    m_value->set(case_);
}

DynamicSelectorProperty::DynamicSelectorProperty(std::wstring name, std::wstring description, std::shared_ptr<ValueCollection> value, PropertyAttributes attributes):
    SingleProperty<ValueCollection>(std::move(name), std::move(description), std::move(value), attributes)
{
}

std::vector<std::wstring> DynamicSelectorProperty::getItems() const
{
    auto itemsArray = std::dynamic_pointer_cast<ValueArray<SingleValue<std::wstring>>>(m_value->getMember(1));

    std::vector<std::wstring> items;
    for (size_t position = 0; position < itemsArray->size(); ++position)
    {
        items.push_back(itemsArray->getItem(position)->get());
    }
    return items;
}

int DynamicSelectorProperty::getSelection() const
{
    auto indexValue = std::dynamic_pointer_cast<SingleValue<int>>(m_value->getMember(0));
    return indexValue->get();
}

void DynamicSelectorProperty::setSelection(int selection)
{
    auto indexValue = std::dynamic_pointer_cast<SingleValue<int>>(m_value->getMember(0));
    indexValue->set(selection);
}

DynamicSelectorPropertyFactory::DynamicSelectorPropertyFactory(std::wstring name, std::wstring description, PropertyAttributes attributes):
    SinglePropertyFactory<DynamicSelectorProperty>(std::move(name), std::move(description), attributes)
{
}

DynamicSelectorProperty DynamicSelectorPropertyFactory::createProperty(std::shared_ptr<ValueCollection> valueCollection) const
{
    return DynamicSelectorProperty(m_name, m_description, valueCollection, m_attributes);
}

SelectorPropertyEx::SelectorPropertyEx(std::wstring name, std::wstring description, std::wstring caption, ExpandHandler handler, std::vector<std::wstring> items, std::shared_ptr<SingleValue<int>> value, PropertyAttributes attributes):
    SingleProperty<SingleValue<int>>(std::move(name), std::move(description), std::move(value), attributes),
    m_caption(std::move(caption)),
    m_expandHandler(std::move(handler)),
    m_items(std::move(items))
{
}

std::vector<std::wstring> SelectorPropertyEx::getItems() const
{
    return m_items;
}

int SelectorPropertyEx::getSelection() const
{
    return m_value->get();
}

void SelectorPropertyEx::setSelection(int selection)
{
    m_value->set(selection);
}

std::wstring SelectorPropertyEx::getExpandCaption() const
{
    return m_caption;
}

boost::any SelectorPropertyEx::expand() const
{
    _ASSERT(m_expandHandler);
    return m_expandHandler(m_value->get());
}

SelectorPropertyExFactory::SelectorPropertyExFactory(std::wstring name, std::wstring description, std::wstring caption, ExpandHandler handler, std::vector<std::wstring> items, PropertyAttributes attributes):
    SinglePropertyFactory<SelectorPropertyEx>(std::move(name), std::move(description), attributes),
    m_caption(std::move(caption)),
    m_expandHandler(std::move(handler)),
    m_items(std::move(items))
{
}

SelectorPropertyEx SelectorPropertyExFactory::createProperty(std::shared_ptr<SingleValue<int>> value) const
{
    return SelectorPropertyEx(m_name, m_description, m_caption, m_expandHandler, m_items, value, m_attributes);
}

std::unique_ptr<SelectorPropertyExFactory> selectorPropertyExFactory(std::wstring name, std::wstring description, std::wstring caption, SelectorPropertyExFactory::ExpandHandler expandHandler, std::vector<std::wstring> items, PropertyAttributes attributes)
{
    return std::make_unique<SelectorPropertyExFactory>(std::move(name), std::move(description), std::move(caption), std::move(expandHandler), std::move(items), attributes);
}

CheckListProperty::Attributes CheckListProperty::DEFAULT_ATTRIBUTES = CHECKLIST_ATTRIBUTES(DEFAULT);

CheckListProperty::CheckListProperty(std::wstring name, std::wstring description, std::vector<std::wstring> items, std::shared_ptr<ValueArray<SingleValue<bool>>> checks, Attributes attributes):
    SingleProperty<ValueArray<SingleValue<bool>>>(std::move(name), std::move(description), checks, PropertyAttribute(attributes.get_value() & 0xFF)),
    m_items(std::move(items)),
    m_attributes(attributes)
{
}

bool CheckListProperty::isChecked(size_t position) const
{
    return m_value->getItem(position)->get();
}

void CheckListProperty::setChecked(size_t position, bool checked)
{
    m_value->getItem(position)->set(checked);
}

const std::vector<std::wstring>& CheckListProperty::getItems() const
{
    return m_items;
}

bool CheckListProperty::checkRequired() const
{
    return m_attributes.contains(CHECK_REQUIRED);
}

CheckListPropertyFactory::CheckListPropertyFactory(std::wstring name, std::wstring description, std::vector<std::wstring> items):
    SinglePropertyFactory<CheckListProperty>(std::move(name), std::move(description)),
    m_items(std::move(items))
{
}

CheckListProperty CheckListPropertyFactory::createProperty(std::shared_ptr<ValueArray<SingleValue<bool>>> checks) const
{
    return CheckListProperty(m_name, m_description, m_items, checks);
}

SelectionListProperty::Attributes SelectionListProperty::DEFAULT_ATTRIBUTES = SELECTIONLIST_ATTRIBUTES(DEFAULT);

SelectionListProperty::SelectionListProperty(std::wstring name, std::wstring description, std::vector<std::wstring> items, std::shared_ptr<SingleValue<int>> value, Attributes attributes):
    SingleProperty<SingleValue<int>>(std::move(name), std::move(description), value, PropertyAttribute(attributes.get_value() & 0xFF)),
    m_items(std::move(items)),
    m_attributes(attributes)
{
}

size_t SelectionListProperty::getSelection() const
{
    return size_t(m_value->get());
}

void SelectionListProperty::select(size_t position)
{
    m_value->set(int(position));
}

const std::vector<std::wstring>& SelectionListProperty::getItems() const
{
    return m_items;
}

bool SelectionListProperty::isSelectionRequired() const
{
    return m_attributes.contains(SELECTION_REQUIRED);
}

SelectionListPropertyFactory::SelectionListPropertyFactory(std::wstring name, std::wstring description, std::vector<std::wstring> items, Attributes attributes):
    SinglePropertyFactory<SelectionListProperty>(std::move(name), std::move(description), PropertyAttribute(attributes.get_value() & 0xFF)),
    m_items(std::move(items)),
    m_attributes(attributes)
{
}

SelectionListProperty SelectionListPropertyFactory::createProperty(std::shared_ptr<SingleValue<int>> value) const
{
    return SelectionListProperty(m_name, m_description, m_items, value, m_attributes);
}

DateProperty::DateProperty(std::wstring name, std::wstring description, std::shared_ptr<SingleValue<std::tm>> value, PropertyAttributes attributes):
    SingleProperty<SingleValue<std::tm>>(std::move(name), std::move(description), value, attributes)
{
}

std::tm DateProperty::getDate() const
{
    return m_value->get();
}

void DateProperty::setDate(std::tm date)
{
    m_value->set(date);
}

DatePropertyFactory::DatePropertyFactory(std::wstring name, std::wstring description, PropertyAttributes attributes):
    SinglePropertyFactory<DateProperty>(std::move(name), std::move(description), attributes)
{
}

DateProperty DatePropertyFactory::createProperty(std::shared_ptr<SingleValue<tm>> value) const
{
    return DateProperty(m_name, m_description, value, m_attributes);
}

TimeProperty::TimeProperty(std::wstring name, std::wstring description, std::shared_ptr<SingleValue<std::tm>> value, PropertyAttributes attributes):
    SingleProperty<SingleValue<std::tm>>(std::move(name), std::move(description), value, attributes)
{
}

std::tm TimeProperty::getTime() const
{
    return m_value->get();
}

void TimeProperty::setTime(std::tm time)
{
    return m_value->set(time);
}

TimePropertyFactory::TimePropertyFactory(std::wstring name, std::wstring description, PropertyAttributes attributes):
    SinglePropertyFactory<TimeProperty>(std::move(name), std::move(description), attributes)
{
}

TimeProperty TimePropertyFactory::createProperty(std::shared_ptr<SingleValue<tm>> value) const
{
    return TimeProperty(m_name, m_description, value, m_attributes);
}

BrowseProperty::BrowseProperty(std::wstring name, std::wstring description, std::wstring caption, boost::any subject, FormatFunctor formatFunctor, PropertyAttributes attributes):
    SingleProperty<SingleValue<boost::any>>(std::move(name), std::move(description), staticValue(subject), attributes),
    m_caption(std::move(caption)),
    m_formatFunctor(std::move(formatFunctor))
{
}

std::wstring BrowseProperty::getCaption() const
{
    return m_caption;
}

boost::any BrowseProperty::getSubject() const
{
    return m_value->get();
}

std::wstring BrowseProperty::formatString() const
{
    return m_formatFunctor(m_value->get());
}

OptionalProperty::OptionalProperty(OptionalPredicate optionalPredicate, ComposedProperty innerProperty):
    m_optionalPredicate(optionalPredicate),
    m_innerProperty(std::move(innerProperty))
{
}

bool OptionalProperty::isReady() const
{
    return m_optionalPredicate();
}

ComposedProperty OptionalProperty::getInnerProperty() const
{
    return m_innerProperty;
}

std::shared_ptr<ValueCollection> OptionalProperty::shareValue() const
{
    //_ASSERT(false);
    return nullptr;
}