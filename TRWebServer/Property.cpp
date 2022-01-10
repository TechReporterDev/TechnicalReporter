#include "stdafx.h"
#include "Property.h"

PropertyAttributes DEFAULT_PROPERTY_ATTRIBUTES = PropertyAttribute::DEFAULT;

Property::Property(std::wstring name, std::wstring description, PropertyAttributes attributes):
    m_name(std::move(name)),
    m_description(std::move(description)),
    m_attributes(attributes)
{
}

std::wstring Property::getName() const
{
    return m_name;
}

std::wstring Property::getDescription() const
{
    return m_description;
}

PropertyAttributes Property::getAttributes() const
{
    return m_attributes;
}

bool Property::isReadOnly() const
{
    return m_attributes.contains(PropertyAttribute::READ_ONLY);
}

PropertyFactory::PropertyFactory(std::wstring name, std::wstring description, PropertyAttributes attributes):
    m_name(std::move(name)),
    m_description(std::move(description)),
    m_attributes(attributes)
{
}

std::wstring PropertyFactory::getName() const
{
    return m_name;
}

std::wstring PropertyFactory::getDescription() const
{
    return m_description;
}

PropertyAttributes PropertyFactory::getAttributes() const
{
    return m_attributes;
}