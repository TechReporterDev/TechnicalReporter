#pragma once
#include "TRWebServer.h"
#include "Value.h"

#pragma warning(push)
#pragma warning(disable:4521)

template<class T>
class StaticValue: public SingleValue<T>
{
public:
    StaticValue(const T& val):
        m_val(val)
    {
    }

    StaticValue(T&& val):
        m_val(std::move(val))
    {
    }

    virtual std::shared_ptr<Value> clone() const override
    {
        return std::make_shared<StaticValue>(m_val);
    }

    virtual T get() const override
    {
        return m_val;
    }

    virtual void set(T value) override
    {
        m_val = std::move(value);
    }

private:
    T m_val;
};

template<class T>
std::shared_ptr<StaticValue<std::decay_t<T>>> staticValue(T&& val)
{
    return std::make_shared<StaticValue<std::decay_t<T>>>(std::forward<T>(val));
}

#pragma warning(pop)