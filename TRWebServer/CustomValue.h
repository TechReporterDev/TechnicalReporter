#pragma once
#include "Value.h"
#include "StaticValue.h"

template<class T>
class CustomValue: public SingleValue<T>
{
public:
    using GetImpl = std::function<T()>;
    using SetImpl = std::function<void(T)>;

    CustomValue(GetImpl getImpl, SetImpl setImpl):
        m_getImpl(std::move(getImpl)),
        m_setImpl(std::move(setImpl))
    {
    }

    virtual std::shared_ptr<Value> clone() const override
    {
        return std::make_shared<StaticValue<T>>(get());
    }

    virtual T get() const override
    {
        return m_getImpl();
    }

    virtual void set(T value) override
    {
        m_setImpl(std::move(value));
    }

private:
    GetImpl m_getImpl;
    SetImpl m_setImpl;
};

template<class T>
std::shared_ptr<CustomValue<T>> customValue(typename CustomValue<T>::GetImpl getImpl, typename CustomValue<T>::SetImpl setImpl)
{
    return std::make_unique<CustomValue<T>>(std::move(getImpl), std::move(setImpl));
}