#pragma once
#include "..\STLTools\uuid.h"
#include <vector>
namespace TR { namespace Core {

template <class T>
struct DefaultRestrictionValue
{
    static T get_value()
    {
        return T{};
    }
};

template <class T>
struct Restriction
{
    Restriction():
        m_value(DefaultRestrictionValue<T>::get_value())
    {
    }

    explicit Restriction(T value):
        m_value(std::move(value))
    {
    }

    Restriction& operator = (T value)
    {
        m_value = std::move(value);
    }

    operator const T&() const
    {
        return m_value;
    }

    T m_value;
};

template <class T>
inline bool operator == (const Restriction<T>& left, const Restriction<T>& right)
{
    return left.m_value == right.m_value;
}

template <class T>
inline bool operator != (const Restriction<T>& left, const Restriction<T>& right)
{
    return left.m_value != right.m_value;
}

template <class T>
inline bool operator < (const Restriction<T>& left, const Restriction<T>& right)
{
    return left.m_value < right.m_value;
}

typedef Restriction<std::string> StringRestriction;

}} //namespace TR { namespace Core {

