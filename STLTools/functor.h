#pragma once
#include <boost/optional.hpp>
namespace stl_tools {

struct NOP
{
    template<class... Args>
    void operator ()(Args&&... args)
    {
    }
};

template<class T, T Value>
struct Always
{
    template<class... Args>
    T operator ()(Args&&... args)
    {
        return Value;
    }
};

template<class T>
struct Same
{
    Same(T value):
        m_value(std::move(value))
    {
    }

    template<class... Args>
    T operator ()(Args&&... args)
    {
        return m_value;
    }

    T m_value;
};

template<class T>
Same<typename std::decay<T>::type> same(T&& value)
{
    return Same<typename std::decay<T>::type>(std::forward<T>(value));
}

template<class Functor>
struct Proxy
{
    Proxy(Functor& functor):
        m_functor(&functor)
    {
    }

    template<class... Args>
    typename std::result_of<Functor(Args...)>::type operator ()(Args&&... args)
    {
        return (*m_functor)(std::forward<Args>(args)...);
    }

    Functor* m_functor;
};

template<class T>
Proxy<T> proxy(T& functor)
{
    return Proxy<T>(functor);
}

template<class Functor>
struct ForceMoveArgs
{
    ForceMoveArgs(Functor functor):
        m_functor(std::move(functor))
    {
    }

    template<class... Args>
    typename std::result_of<Functor(Args&&...)>::type operator ()(Args&... args)
    {
        return m_functor(std::move(args)...);
    }

    Functor m_functor;
};

template<class Functor>
ForceMoveArgs<Functor> force_move_args(Functor&& functor)
{
    return ForceMoveArgs<Functor>(std::forward<Functor>(functor));
}

template<class Functor>
struct FakeCopyable
{

    FakeCopyable(Functor&& functor):
        m_functor(std::move(functor))
    {       
    }

    FakeCopyable(const FakeCopyable&)
    {
        throw std::logic_error("Fake copy constructor called");
    }

    FakeCopyable(FakeCopyable&& fc):
        m_functor(std::move(fc.m_functor))
    {       
    }

    FakeCopyable& operator = (const FakeCopyable&)
    {
        throw std::logic_error("Fake copy operator called");
    }

    FakeCopyable& operator = (FakeCopyable&& fc)        
    {
        m_functor = std::move(fc.m_functor);
    }

    template<class... Args>
    typename std::result_of<Functor(Args&&...)>::type operator ()(Args&&... args)
    {
        return (*m_functor)(std::forward<Args>(args)...);
    }

    boost::optional<Functor> m_functor;
};

template<class Functor>
FakeCopyable<Functor> fake_copyable(Functor&& functor)
{
    static_assert(std::is_reference<Functor>::value == false, "");
    return FakeCopyable<Functor>(std::move(functor));
}

template<class Functor>
struct ForceMovable
{
    ForceMovable(Functor&& functor):
        m_functor(std::move(functor))
    {       
    }

    ForceMovable(const ForceMovable& fm):
        m_functor(std::move(const_cast<ForceMovable&>(fm).m_functor))
    {       
    }

    ForceMovable(ForceMovable&& fm):
        m_functor(std::move(fm.m_functor))
    {
    }

    ForceMovable& operator = (const ForceMovable& fm)
    {
        m_functor = std::move(const_cast<ForceMovable&>(fm).m_functor);
    }

    ForceMovable& operator = (ForceMovable&& fm)
    {
        m_functor = std::move(fc.m_functor);
    }

    template<class... Args>
    typename std::result_of<Functor(Args&&...)>::type operator ()(Args&&... args)
    {
        return m_functor(std::forward<Args>(args)...);
    }

    Functor m_functor;
};

template<class Functor>
ForceMovable<Functor> force_movable(Functor&& functor)
{
    static_assert(std::is_reference<Functor>::value == false, "");
    return ForceMovable<Functor>(std::move(functor));
}

namespace
{
    const NOP nop;
    const Always<bool, true> always_true;
    const Always<bool, true> always_false;
}

}