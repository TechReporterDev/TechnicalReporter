#pragma once
#include <memory>
#include <mutex>
#include <boost/optional.hpp>
#include <boost/thread/once.hpp>
namespace stl_tools {
#pragma warning(push)
#pragma warning(disable:4521)

template <class T, class Functor>
class lazy_unique_ptr;

template <class T, class Functor = std::function<T()>>
class lazy
{
public:
    template <class X, class XF>
    friend class lazy;

    template <class T1, class T2, class Functor1, class Functor2>
    friend lazy_unique_ptr<T1, Functor1> static_pointer_cast(lazy_unique_ptr<T2, Functor2> ptr2);

    lazy(T obj):
        m_obj(std::move(obj)),
        m_once_flag(BOOST_ONCE_INIT)
    {
    }

    explicit lazy(Functor functor):
        m_functor(std::move(functor)),
        m_once_flag(BOOST_ONCE_INIT)
    {
    }

    lazy(const lazy&) = delete;
    lazy& operator = (const lazy&) = delete;

    lazy(lazy&& lz):
        m_functor(std::move(lz.m_functor)),
        m_obj(std::move(lz.m_obj)),
        m_once_flag(BOOST_ONCE_INIT)
    {
    }

    template<class X, class FX>
    lazy(lazy<X, FX>&& lz):
        m_functor(std::move(lz.m_functor)),
        m_obj(std::move(lz.m_obj)),
        m_once_flag(BOOST_ONCE_INIT)
    {
    }

    lazy& operator = (lazy&& lz) = delete;  

    //'assign' equal to 'get() = x', but more efficient
    template<class X>
    void assign(X&& x) 
    {
        if (!init(std::forward<X>(x)))
        {
            _ASSERT(m_obj);
            m_obj = std::forward<X>(x);
        }
    }

    template<class X>
    bool init(X&& x)
    {
        bool done = false;
        boost::call_once(m_once_flag, [this, &done](X&& x){
            m_obj = std::forward<X>(x);
            done = true;
        }, std::forward<X>(x));
        return done;
    }

    T& get()
    {
        boost::call_once(m_once_flag, [this](){
            if (!m_obj)
            {
                m_obj = m_functor();
            }
        });
        return *m_obj;
    }

    const T& get() const
    {
        return const_cast<lazy&>(*this).get();
    }

    operator T& ()
    {
        return get();
    }

    operator T& () const
    {
        return get();
    }

private:
    Functor m_functor;
    mutable boost::optional<T> m_obj;
    mutable boost::once_flag m_once_flag;
};

template <class T, class Functor>
class lazy_shared_ptr;

template <class T, class Functor = std::function<std::unique_ptr<T>()>>
class lazy_unique_ptr
{
public:
    template <class X, class XF> 
    friend class lazy_shared_ptr;

    template <class X, class XF>
    friend class lazy_unique_ptr;

    template <class T1, class T2, class Functor1, class Functor2>
    friend lazy_unique_ptr<T1, Functor1> static_pointer_cast(lazy_unique_ptr<T2, Functor2> ptr2);

    using holder_type = lazy<std::unique_ptr<T>, Functor>;

    lazy_unique_ptr(const lazy_unique_ptr&) = delete;
    lazy_unique_ptr& operator = (const lazy_unique_ptr&) = delete;

    lazy_unique_ptr(lazy_unique_ptr&& lzp):
        m_ptr(std::move(lzp.m_ptr))
    {
    }

    lazy_unique_ptr& operator = (lazy_unique_ptr&& lzp)
    {
        m_ptr = std::move(lzp.m_ptr);
    }

    lazy_unique_ptr():
        m_ptr(new holder_type(std::unique_ptr<T>(nullptr)))
    {
    }

    lazy_unique_ptr(nullptr_t):
        lazy_unique_ptr()
    {
    }

    explicit lazy_unique_ptr(T* ptr):
        m_ptr(new holder_type(std::unique_ptr<T>(ptr)))
    {
    }

    template <class X>
    lazy_unique_ptr(std::unique_ptr<X> ptr) :
        m_ptr(new holder_type(std::unique_ptr<T>(std::move(ptr))))
    {
    }

    template <class F, class R = decltype(_STD declval<F>()())>
    lazy_unique_ptr(F&& f) :
        m_ptr(new holder_type(Functor(std::forward<F>(f))))
    {
    }

    template <class X, class FX>
    lazy_unique_ptr(lazy_unique_ptr<X, FX> ptr):
        m_ptr(new holder_type(std::move(*ptr.m_ptr)))
    {
    }

    T* get() const
    {       
        return (*m_ptr).get().get();
    }

    T* operator -> () const
    {
        return get();
    }

    T& operator * () const
    {
        return *get();
    }

    explicit operator bool() const
    {
        return static_cast<bool>((*m_ptr).get());
    }

    std::unique_ptr<T> release()
    {
        return std::move((*m_ptr).get());
    }

    void reset(T* ptr)
    {
        m_ptr.reset(new holder_type(std::unique_ptr<T>(ptr)));
    }

    template <class X>
    void reset(std::unique_ptr<X> ptr)
    {
        m_ptr.reset(new holder_type(std::unique_ptr<T>(std::move(ptr))));
    }

    template <class F, class R = decltype(_STD declval<F>()())>
    void reset(F&& f)
    {
        m_ptr.reset(new holder_type(Functor(std::forward<F>(f))));
    }

private:    
    std::unique_ptr<holder_type> m_ptr;
};

template <class T1, class T2, class Functor1 = std::function<std::unique_ptr<T1>()>, class Functor2 = std::function<std::unique_ptr<T1>()>>
lazy_unique_ptr<T1, Functor1> static_pointer_cast(lazy_unique_ptr<T2, Functor2> ptr2)
{
    auto& holder = *ptr2.m_ptr;
    if (holder.m_obj)
    {
        return lazy_unique_ptr<T1, Functor1>(static_pointer_cast<T1>(std::move(*holder.m_obj)));
    }
    auto functor = std::move(holder.m_functor);
    return lazy_unique_ptr<T1, Functor1>([functor]{return static_pointer_cast<T1>(functor()); });
}

template <class T, class Functor = std::function<std::shared_ptr<T>()>>
class lazy_shared_ptr
{
public:
    using holder_type = lazy<std::shared_ptr<T>, Functor>;

    lazy_shared_ptr():
        m_ptr(new holder_type(std::shared_ptr<T>(nullptr)))
    {
    }

    lazy_shared_ptr(const lazy_shared_ptr& ptr) = default;
    lazy_shared_ptr& operator = (const lazy_shared_ptr&) = default;

    lazy_shared_ptr(nullptr_t):
        lazy_shared_ptr()
    {
    }
    
    explicit lazy_shared_ptr(T* ptr):
        m_ptr(new holder_type(std::shared_ptr<T>(ptr)))
    {
    }

    template <class X>
    lazy_shared_ptr(std::unique_ptr<X> ptr):
        m_ptr(new holder_type(std::shared_ptr<T>(std::move(ptr))))
    {       
    }

    template <class X, class FX>
    lazy_shared_ptr(lazy_unique_ptr<X, FX> ptr):
        m_ptr(new holder_type(std::move(*ptr.m_ptr)))
    {       
    }

    template <class X>
    lazy_shared_ptr(std::shared_ptr<X> ptr):
        m_ptr(new holder_type(std::shared_ptr<T>(ptr)))
    {       
    }

    template <class F, class R = decltype(_STD declval<F>()())>
    lazy_shared_ptr(F&& f) :
        m_ptr(new holder_type(Functor(f)))
    {
    }

    T* get() const
    {
        return (*m_ptr).get().get();
    }

    T* operator -> () const
    {
        return get();
    }

    T& operator * () const
    {
        return *get();
    }

    explicit operator bool() const
    {
        return (*m_ptr).get() != nullptr;
    }

    bool equal(const lazy_shared_ptr<T, Functor> right) const
    {
        return m_ptr == right.m_ptr;
    }

    std::shared_ptr<T> share() const
    {
        return (*m_ptr).get();
    }

    void reset(T* ptr)
    {
        m_ptr.reset(new holder_type(std::shared_ptr<T>(ptr)));
    }

    template <class X>
    void reset(std::unique_ptr<X> ptr)
    {
        m_ptr.reset(new holder_type(std::shared_ptr<T>(std::move(ptr))));
    }

    template <class X>
    void reset(std::shared_ptr<X> ptr)
    {
        m_ptr.reset(new holder_type(std::shared_ptr<T>(ptr)));
    }

    template <class F, class R = decltype(_STD declval<F>()())>
    void reset(F&& f)
    {
        m_ptr.reset(new holder_type(Functor(std::forward<F>(f))));
    }

private:
    std::shared_ptr<holder_type> m_ptr;
};

template <class T, class Functor>
bool operator == (lazy_shared_ptr<T, Functor> left, const lazy_shared_ptr<T, Functor> right)
{
    return left.equal(right);
}

template <class T, class Functor>
bool operator == (lazy_shared_ptr<T, Functor> left, nullptr_t)
{
    return static_cast<bool>(left) == false;
}

#pragma warning(pop)
}
//namespace stl_tools {
