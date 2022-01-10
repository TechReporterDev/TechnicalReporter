#pragma once
#include <utility>
#include <type_traits>
namespace stl_tools {

template<class T>
class ForceMoveHolder
{
public:
    explicit ForceMoveHolder(T&& obj):
        m_obj(std::move(obj)),
        m_valid(true)
    {
    }

    // dirty hack
    ForceMoveHolder(const ForceMoveHolder& fm):
        m_obj(std::move(const_cast<ForceMoveHolder&>(fm).m_obj)),
        m_valid(fm.m_valid)
    {
        const_cast<ForceMoveHolder&>(fm).m_valid = false;
    }

    ForceMoveHolder(ForceMoveHolder&& fm):
        m_obj(std::move(fm.m_obj)),
        m_valid(fm.m_valid)
    {
        fm.m_valid = false;
    }

    operator T ()
    {
        if (!m_valid)
        {
            throw std::logic_error("force_move object is invalid");
        }

        m_valid = false;
        return std::move(m_obj);
    }

private:
    T m_obj;
    bool m_valid;
};

template<class T>
ForceMoveHolder<std::remove_reference_t<T>> force_move(T&& arg)
{
    return ForceMoveHolder<typename std::remove_reference<T>::type>(std::move(arg));
}

} //namespace stl_tools {