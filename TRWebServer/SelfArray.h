#pragma once
#include "TRWebServer.h"
#include "Value.h"
#include "StaticValue.h"
#include "ComposedCollection.h"

template<class ValueType_>
class SelfArray: public ValueArray<ValueType_>
{
public:
    virtual std::shared_ptr<Value> clone() const override
    {
        auto selfArray = std::make_shared<SelfArray>();
        selfArray->assign(*this);
        return selfArray;
    }

    virtual size_t size() const override
    {
        return m_values.size();
    }

    virtual Item getItem(size_t position) const override
    {
        return m_values[position];
    }

    virtual void addItem(size_t position, const ValueType& value) override
    {
        m_values.insert(std::next(m_values.begin(), position), std::dynamic_pointer_cast<ValueType>(value.clone()));
    }

    virtual void removeItem(size_t position)
    {
        m_values.erase(m_values.begin() + position);
    }

private:
    std::vector<Item> m_values;
};

template<class ValueType>
std::shared_ptr<SelfArray<ValueType>> selfArray()
{
    return std::make_shared<SelfArray<ValueType>>();
}

template<class T>
std::shared_ptr<SelfArray<SingleValue<T>>> selfArray(std::initializer_list<T> values)
{
    auto sa = std::make_shared<SelfArray<SingleValue<T>>>();
    for (auto& value : values)
    {
        sa->addItem(sa->size(), *staticValue(value));
    }
    return sa;
}