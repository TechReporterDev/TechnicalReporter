#pragma once
#include "TRWebServer.h"
#include "Value.h"
#include "StaticValue.h"

#pragma warning(push)
#pragma warning(disable:4521)

template<class T, class X = std::conditional_t<std::is_enum<T>::value, int, T>>
class RefValue: public SingleValue<X>
{
public:
    RefValue(T& ref):
        m_ref(ref)
    {
    }

    virtual std::shared_ptr<Value> clone() const override
    {
        return staticValue(get());
    }

    virtual X get() const override
    {
        return static_cast<X>(m_ref.get());
    }

    virtual void set(X value) override
    {
        m_ref.get() = static_cast<T>(std::move(value));
    }

private:
    std::reference_wrapper<T> m_ref;
};

template<class T>
std::shared_ptr<RefValue<T>> refValue(T& ref)
{
    return std::make_shared<RefValue<T>>(ref);
}

#pragma warning(pop)