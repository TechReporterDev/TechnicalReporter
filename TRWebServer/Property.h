#pragma once
#include "TRWebServer.h"
#include "Value.h"
#include "ComposedCollection.h"
#include "SelfArray.h"

#pragma warning(push)
#pragma warning(disable:4521)

enum class PropertyAttribute { DEFAULT = 0x00, READ_ONLY = 0x01, NO_PREVIEW = 0x02 };
using PropertyAttributes = stl_tools::flag_type<PropertyAttribute>;
extern PropertyAttributes DEFAULT_PROPERTY_ATTRIBUTES;

class Property
{
public:
    Property(std::wstring name, std::wstring description, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);
    virtual ~Property() = default;

    std::wstring                getName() const;
    std::wstring                getDescription() const; 
    PropertyAttributes          getAttributes() const;
    bool                        isReadOnly() const;
    
private:
    std::wstring m_name;
    std::wstring m_description;
    PropertyAttributes m_attributes;
};

class PropertyFactory
{
public:
    PropertyFactory(std::wstring name, std::wstring description, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);
    virtual ~PropertyFactory() = default;

    std::wstring                getName() const;
    std::wstring                getDescription() const;
    PropertyAttributes          getAttributes() const;

protected:
    std::wstring m_name;
    std::wstring m_description;
    PropertyAttributes m_attributes;
};

template<class ValueType_>
class SingleProperty: public Property
{
public:
    using ValueType = ValueType_;
    
    SingleProperty(std::wstring name, std::wstring description, std::shared_ptr<ValueType> value, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);
    std::shared_ptr<ValueType> shareValue() const;

protected:
    std::shared_ptr<ValueType> m_value;
};

template<class PropertyType_, class ValueType_ = typename PropertyType_::ValueType>
class SinglePropertyFactory: public PropertyFactory
{
public:
    using PropertyType = PropertyType_;
    using ValueType = ValueType_;

    SinglePropertyFactory(std::wstring name, std::wstring description, PropertyAttributes attributes = DEFAULT_PROPERTY_ATTRIBUTES);
};

template<class ItemPropertyFactory_>
class PropertyArray: public Property
{
public:
    using ItemPropertyFactory = ItemPropertyFactory_;
    using ItemPropertyType = typename ItemPropertyFactory::PropertyType;
    using ArrayType = ValueArray<typename ItemPropertyFactory::ValueType>;
    using ValueType = ArrayType;

    PropertyArray(std::wstring name, std::wstring description, ItemPropertyFactory factory, std::shared_ptr<ArrayType> valueArray, typename ArrayType::Item defaultValue = nullptr);

    size_t                              size() const;
    ItemPropertyType                    createProperty() const;
    ItemPropertyType                    getProperty(size_t position) const;
    void                                addProperty(typename ArrayType::ValueType& propertyValue, size_t position);
    void                                removeProperty(size_t position);

    const ItemPropertyFactory&          getFactory() const;
    std::shared_ptr<ArrayType>          shareArray() const;
    std::shared_ptr<ArrayType>          shareValue() const;

private:
    ItemPropertyFactory m_factory;
    std::shared_ptr<ArrayType> m_valueArray;
    typename ArrayType::Item m_defaultValue;
};

template <class PropertyArrayType>
class PropertyArrayFactory: public PropertyFactory
{
public:
    using PropertyType = PropertyArrayType;
    using ArrayType = typename PropertyArrayType::ArrayType;

    PropertyArrayFactory(std::wstring name, std::wstring description, typename PropertyArrayType::ItemPropertyFactory factory, typename ArrayType::Item defaultValue = nullptr);
    PropertyArrayType createProperty(std::shared_ptr<ArrayType> valueArray) const;
    PropertyArrayType createProperty(std::shared_ptr<Value> value) const;

private:
    typename PropertyArrayType::ItemPropertyFactory m_factory;
    typename ArrayType::Item m_defaultValue;
};

class ComposedProperty: public Property
{
public:
    friend class ComposedPropertyFactory;
    using ValueType = ValueCollection;  

    struct MemberProperty
    {
        boost::any m_property;
        std::shared_ptr<Value> m_value;
    };

    template<class... Args>
    ComposedProperty(std::wstring name, std::wstring description, Args&&... args):
        Property(std::move(name), std::move(description))
    {
        addProperties(std::forward<Args>(args)...);
    }

    template<class Arg, class... Args>
    void addProperties(Arg&& arg, Args&&... args)
    {
        m_properties.push_back(MemberProperty{arg, arg.shareValue()});
        addProperties(std::forward<Args>(args)...);
    }

    void addProperties()
    {
    }

    template<class PropertyType, std::enable_if_t<!std::is_pointer<PropertyType>::value, int> = 0>
    const PropertyType& getProperty(size_t position) const
    {
        return boost::any_cast<PropertyType>(m_properties[position].m_property);
    }

    template<class PropertyType, std::enable_if_t<std::is_pointer<PropertyType>::value, int> = 0>
    const std::remove_pointer_t<PropertyType>* getProperty(size_t position) const
    {
        return boost::any_cast<std::remove_pointer_t<PropertyType>>(&m_properties[position].m_property);
    }

    size_t size() const
    {
        return m_properties.size();
    }

    void removeProperty(size_t position)
    {
        m_properties.erase(m_properties.begin() + position);
    }

    std::shared_ptr<ValueCollection> shareValue() const
    {
        auto collection = composedCollection({});
        for (auto& ppt : m_properties)
        {
            collection->appendMember(ppt.m_value);
        }
        return collection;
    }

private:
    std::vector<MemberProperty> m_properties;
};

class ComposedPropertyFactory: public PropertyFactory
{
public:
    using PropertyType = ComposedProperty;
    using ValueType = ValueCollection;
    
    struct MemberPropertyFactory
    {
        std::wstring m_name;
        std::wstring m_description;
        PropertyAttributes m_attributes;
        std::function<ComposedProperty::MemberProperty(std::shared_ptr<Value>)> m_factory;
    };

    template<class... Args>
    ComposedPropertyFactory(std::wstring name, std::wstring description, Args&&... args):
        PropertyFactory(std::move(name), std::move(description))
    {
        addFactories(std::forward<Args>(args)...);
    }

    template<class Arg, class... Args>
    void addFactories(Arg&& arg, Args&&... args)
    {
        m_propertyFactories.push_back(MemberPropertyFactory{arg.getName(), arg.getDescription(), arg.getAttributes(), [arg](std::shared_ptr<Value> value){
            auto propertyValue = std::dynamic_pointer_cast<typename Arg::PropertyType::ValueType>(value);
            if (!propertyValue)
            {
                throw std::bad_cast::__construct_from_string_literal("Invalid value type");
            }
            return ComposedProperty::MemberProperty{arg.createProperty(propertyValue), value};
        }});
        addFactories(std::forward<Args>(args)...);
    }

    void addFactories()
    {
    }
    
    ComposedProperty createProperty(std::shared_ptr<ValueCollection> valueCollection) const
    {
        ComposedProperty composed(m_name, m_description);
        for (size_t i = 0; i < valueCollection->size(); ++i)
        {
            auto member = valueCollection->getMember(i);
            composed.m_properties.push_back(m_propertyFactories.at(i).m_factory(member));
        }
        return composed;
    }

    ComposedProperty createProperty(std::shared_ptr<Value> value) const
    {
        auto valueCollection = std::dynamic_pointer_cast<ValueCollection>(value);
        if (!valueCollection)
        {
            throw std::bad_cast::__construct_from_string_literal("Invalid value type");
        }
        return createProperty(valueCollection);
    }

    size_t getFactoryCount() const
    {
        return m_propertyFactories.size();
    }

    const MemberPropertyFactory& getFactory(size_t index) const
    {
        return m_propertyFactories.at(index);
    }

private:    
    std::vector<MemberPropertyFactory> m_propertyFactories;
};

///////////////
// implementation
///////////////

template<class ValueType>
SingleProperty<ValueType>::SingleProperty(std::wstring name, std::wstring description, std::shared_ptr<ValueType> value, PropertyAttributes attributes):
    Property(std::move(name), std::move(description), attributes),
    m_value(value)
{
}

template<class ValueType>
std::shared_ptr<ValueType> SingleProperty<ValueType>::shareValue() const
{
    return m_value;
}

template<class SinglePropertyType, class ValueType>
SinglePropertyFactory<SinglePropertyType, ValueType>::SinglePropertyFactory(std::wstring name, std::wstring description, PropertyAttributes attributes):
    PropertyFactory(std::move(name), std::move(description), attributes)
{
}

template<class ItemPropertyFactory>
PropertyArray<ItemPropertyFactory>::PropertyArray(std::wstring name, std::wstring description, ItemPropertyFactory factory, std::shared_ptr<ArrayType> valueArray, typename ArrayType::Item defaultValue):
    Property(std::move(name), std::move(description)),
    m_factory(std::move(factory)),
    m_valueArray(valueArray),
    m_defaultValue(defaultValue)
{
}

template<class ItemPropertyFactory>
size_t PropertyArray<ItemPropertyFactory>::size() const
{
    return m_valueArray->size();
}

template<class ItemPropertyFactory>
typename PropertyArray<ItemPropertyFactory>::ItemPropertyType PropertyArray<ItemPropertyFactory>::createProperty() const
{
    if (!m_defaultValue)
    {
        throw std::logic_error("Invalid default value");
    }
    return m_factory.createProperty(std::dynamic_pointer_cast<ArrayType::ValueType>(m_defaultValue->clone()));
}

template<class ItemPropertyFactory>
typename PropertyArray<ItemPropertyFactory>::ItemPropertyType PropertyArray<ItemPropertyFactory>::getProperty(size_t position) const
{
    return m_factory.createProperty(m_valueArray->getItem(position));
}

template<class ItemPropertyFactory>
void PropertyArray<ItemPropertyFactory>::addProperty(typename ArrayType::ValueType& propertyValue, size_t position)
{
    m_valueArray->addItem(position, propertyValue);
}

template<class ItemPropertyFactory>
void PropertyArray<ItemPropertyFactory>::removeProperty(size_t position)
{
    m_valueArray->removeItem(position);
}

template<class ItemPropertyFactory>
const ItemPropertyFactory& PropertyArray<ItemPropertyFactory>::getFactory() const
{
    return m_factory;
}

template<class ItemPropertyFactory>
std::shared_ptr<typename PropertyArray<ItemPropertyFactory>::ArrayType> PropertyArray<ItemPropertyFactory>::shareArray() const
{
    return m_valueArray;
}

template<class ItemPropertyFactory>
std::shared_ptr<typename PropertyArray<ItemPropertyFactory>::ArrayType> PropertyArray<ItemPropertyFactory>::shareValue() const
{
    return m_valueArray;
}

template <class PropertyArrayType>
PropertyArrayFactory<PropertyArrayType>::PropertyArrayFactory(std::wstring name, std::wstring description, typename PropertyArrayType::ItemPropertyFactory factory, typename PropertyArrayType::ArrayType::Item defaultValue):
    PropertyFactory(std::move(name), std::move(description)),
    m_factory(factory),
    m_defaultValue(defaultValue)
{
}

template <class PropertyArrayType>
PropertyArrayType PropertyArrayFactory<PropertyArrayType>::createProperty(std::shared_ptr<typename PropertyArrayType::ArrayType> valueArray) const
{
    return PropertyArrayType(m_name, m_description, m_factory, valueArray, m_defaultValue);
}

template <class PropertyArrayType>
PropertyArrayType PropertyArrayFactory<PropertyArrayType>::createProperty(std::shared_ptr<Value> value) const
{
    auto arrayValue = std::dynamic_pointer_cast<typename PropertyArrayType::ArrayType>(value);
    if (!arrayValue)
    {
        throw std::bad_cast("Invalid value type");
    }

    return createProperty(arrayValue);
}

#pragma warning(pop)