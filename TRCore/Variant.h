#pragma once
#include <boost\variant.hpp>
namespace TR { namespace Core {

template<class Value>
Value create_invalid()
{
    return Value::create_invalid();
}

template<class... Args>
struct Variant;

template<class Value1, class Value2>
struct Variant<Value1, Value2>
{
    typedef boost::variant<Value1,Value2> boost_variant;

    Variant():
        m_value1(create_invalid<Value1>()),
        m_value2(create_invalid<Value2>())
    {
    }

    Variant(Value1 value1):
        m_value1(value1),
        m_value2(create_invalid<Value2>())
    {
    }

    Variant(Value2 value2):
        m_value1(create_invalid<Value1>()),
        m_value2(value2)
    {
    }

    Variant(const boost_variant& v):
        m_value1(create_invalid<Value1>()),
        m_value2(create_invalid<Value2>())
    {
        switch (v.which())
        {
        case 0:
            m_value1 = boost::get<Value1>(v);
            break;

        case 1:
            m_value2 = boost::get<Value2>(v);
            break;

        default:
            throw(std::logic_error("Invalid variant type"));
            break;
        }
    }

    boost_variant get_boost_variant() const
    {
        if (m_value1 != create_invalid<Value1>())
        {
            return m_value1;
        }
        else if (m_value2 != create_invalid<Value2>())
        {
            return m_value2;
        }
        else
        {
            throw std::logic_error("Invalid usage of not initialized Variant");
        }
    }

    operator boost_variant() const
    {
        return get_boost_variant();
    }
    
    Value1 m_value1;
    Value2 m_value2;
};

template<class Arg1, class Arg2>
bool operator == (const Variant<Arg1, Arg2>& left, const Variant<Arg1, Arg2>& right)
{
    return left.m_value1 == right.m_value1 && left.m_value2 == right.m_value2;
}


template<class Arg1, class Arg2>
bool operator < (const Variant<Arg1, Arg2>& left, const Variant<Arg1, Arg2>& right)
{
    if (left.m_value1 < right.m_value1)
    {
        return true;
    }
    
    if (right.m_value1 < left.m_value1)
    {
        return false;
    }
    return left.m_value2 < right.m_value2;
}

template<class Arg1, class Arg2>
bool operator > (const Variant<Arg1, Arg2>& left, const Variant<Arg1, Arg2>& right)
{
    return right < left;
}

}} //namespace TR { namespace Core {