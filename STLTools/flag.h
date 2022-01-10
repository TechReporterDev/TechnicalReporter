#pragma once
#include <type_traits>
namespace stl_tools {

template<class T>
class flag_type
{
    static_assert(std::is_enum<T>::value, "type is not enum");
    
public:
    typedef T value_type;   
    typedef typename std::underlying_type<T>::type integral_type;
    
    flag_type():
        m_value(0)
    {
    }

    flag_type(T value): 
        m_value(static_cast<integral_type>(value))  
    {
    }

    bool contains(T value) const
    {
        return (m_value & static_cast<integral_type>(value)) != 0;
    }

    bool contains(flag_type fg) const
    {
        return (m_value & fg.m_value) == fg.m_value;
    }

    integral_type get_value() const
    {
        return m_value;
    }

private:
    integral_type m_value;
};

template<class T>
flag_type<T> make_flag(T value)
{
    return flag_type<T>(value);
}

template<class T>
flag_type<T> operator & (flag_type<T> left, flag_type<T> right)
{
    return flag_type<T>(static_cast<T>(left.get_value() & right.get_value()));
}

template<class T>
flag_type<T> operator & (flag_type<T> left, T right)
{
    typedef typename flag_type<T>::integral_type integral_type;
    return flag_type<T>(static_cast<T>(left.get_value() & static_cast<integral_type>(right)));
}

template<class T>
flag_type<T> operator | (flag_type<T> left, flag_type<T> right)
{
    return flag_type<T>(static_cast<T>(left.get_value() | right.get_value()));
}

template<class T>
flag_type<T> operator | (flag_type<T> left, T right)
{
    typedef typename flag_type<T>::integral_type integral_type;
    return flag_type<T>(static_cast<T>(left.get_value() | static_cast<integral_type>(right)));
}

template<class T>
flag_type<T> operator % (flag_type<T> left, T right)
{
    typedef typename flag_type<T>::integral_type integral_type;
    return flag_type<T>(static_cast<T>(left.get_value() & ~static_cast<integral_type>(right)));
}

template<class T>
bool operator == (flag_type<T> left, flag_type<T> right)
{
    return left.get_value() == right.get_value();
}

template<class T>
bool operator == (flag_type<T> left, T right)
{
    typedef typename flag_type<T>::integral_type integral_type;
    return left.get_value() == static_cast<integral_type>(right);
}

struct null_flag
{
};

const null_flag flag;

template<class T>
flag_type<T> operator | (null_flag /*flag*/, T value)
{
    return flag_type<T>(value);
}

} //namespace stl_tools {