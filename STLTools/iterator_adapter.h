#pragma once
#include <iterator>
namespace stl_tools {

template <class IteratorTraits>
class iterator_adapter: public std::iterator<
    typename IteratorTraits::iterator_category, 
    typename IteratorTraits::value_type, 
    ptrdiff_t, 
    typename IteratorTraits::pointer, 
    typename IteratorTraits::reference>
{
public:
    typedef typename IteratorTraits::position_type position_type;
    typedef typename IteratorTraits::base_type base_type;

    iterator_adapter():
        m_position()
    {
    }

    explicit iterator_adapter(position_type position):
        m_position(std::move(position))
    {
    }

    iterator_adapter& operator++()
    {
        m_position = IteratorTraits::next(m_position);
        return *this;
    }

    iterator_adapter operator++(int)
    {
        iterator_adapter tmp(*this);
        operator++();
        return tmp;
    }

    iterator_adapter& operator--()
    {
        m_position = IteratorTraits::prev(m_position);
        return *this;
    }

    iterator_adapter operator--(int)
    {
        iterator_adapter tmp(*this);
        operator--();
        return tmp;
    }

    reference operator*() const
    {
        return IteratorTraits::get_ref(m_position);
    }

    pointer operator->() const
    {
        return IteratorTraits::get_ptr(m_position);
    }

    bool operator == (const iterator_adapter& right) const
    {
        return m_position == right.m_position;
    }

    bool operator != (const iterator_adapter& right) const
    {
        return m_position != right.m_position;
    }

    position_type get_pos() const
    {
        return m_position;
    }

    base_type base() const
    {
        return IteratorTraits::get_base(m_position);
    }

private:
    position_type m_position;
};

template<class Position, class IteratorCategory, class ValueType, class Pointer, class Reference, class Base = nullptr_t>
struct iterator_traits
{

    typedef Position            position_type;
    typedef IteratorCategory    iterator_category;
    typedef ValueType           value_type;
    typedef Pointer             pointer;
    typedef Reference           reference;
    typedef Base                base_type;
};

template<class ValueType, class IteratorCategory>
struct primitive_iterator_traits: iterator_traits<ValueType*, IteratorCategory, ValueType, ValueType* , ValueType&>
{
};

template<class Position, class IteratorCategory, class ValueType>
struct trivial_iterator_traits: iterator_traits<Position, IteratorCategory, ValueType, ValueType* , ValueType&>
{
};

template<class Position, class IteratorCategory, class Pointer, class Reference>
struct nontrivial_iterator_traits: iterator_traits<Position, IteratorCategory, void, Pointer, Reference>
{
};

template<
    class IteratorTraits, 
    typename IteratorTraits::position_type Next(typename IteratorTraits::position_type),
    typename IteratorTraits::position_type Prev(typename IteratorTraits::position_type),
    typename IteratorTraits::pointer GetPtr(typename IteratorTraits::position_type),
    typename IteratorTraits::reference GetRef(typename IteratorTraits::position_type)>
struct static_iterator_traits: IteratorTraits
{
    using typename IteratorTraits::position_type;
    using typename IteratorTraits::pointer;
    using typename IteratorTraits::reference;

    static position_type next(position_type position)
    {
        //static_assert(Next, "Next(position) is not defined");
        _ASSERT(Next);
        return Next(position); 
    };

    static position_type prev(position_type position)
    {
        //static_assert(Prev, "Prev(position) is not defined");
        _ASSERT(Prev);
        return Prev(position);
    };

    static pointer get_ptr(position_type position)
    { 
        //static_assert(GetPtr, "GetPtr(position) is not defined");
        _ASSERT(GetPtr);
        return GetPtr(position); 
    };

    static reference get_ref(position_type position)
    { 
        //static_assert(GetRef, "GetRef(position) is not defined");
        _ASSERT(GetRef);
        return GetRef(position); 
    };
};

template<class ValueType, ValueType* Next(ValueType*), ValueType* Prev(ValueType*), class IteratorCategory>
struct primitive_static_iterator_traits: static_iterator_traits<primitive_iterator_traits<ValueType, IteratorCategory>, Next, Prev, nullptr, nullptr>
{   
    static ValueType* get_ptr(ValueType* position)
    {       
        return position; 
    };

    static ValueType& get_ref(ValueType* position)
    {       
        return *position; 
    };
};

template<class Position, Position Next(Position), Position Prev(Position), class IteratorCategory, class ValueType, ValueType* GetPtr(Position)>
struct trivial_static_iterator_traits: static_iterator_traits<trivial_iterator_traits<Position, IteratorCategory, ValueType>, Next, Prev, GetPtr, nullptr>
{   
    static ValueType& get_ref(Position position)
    {       
        //static_assert(GetRef, "GetPtr(position) is not defined");
        _ASSERT(GetPtr);
        return *GetPtr(position); 
    };
};

template<class Position, Position Next(Position), Position Prev(Position), class IteratorCategory, class Pointer, Pointer GetPtr(Position), class Reference, Reference GetRef(Position)>
struct nontrivial_static_iterator_traits: static_iterator_traits<nontrivial_iterator_traits<Position, IteratorCategory, Pointer, Reference>, Next, Prev, GetPtr, GetRef>
{   
};

template<class ValueType, ValueType* Next(ValueType*)>
struct primitive_forward_static_iterator_traits: 
    primitive_static_iterator_traits<ValueType, Next, nullptr, std::forward_iterator_tag>
{
};

template<class Position, Position Next(Position), class ValueType, ValueType* GetPtr(Position)>
struct trivial_forward_static_iterator_traits: 
    trivial_static_iterator_traits<Position, Next, nullptr, std::forward_iterator_tag, ValueType, GetPtr>
{
};

template<class Position, Position Next(Position), class Pointer, Pointer GetPtr(Position), class Reference, Reference GetRef(Position)>
struct nontrivial_forward_static_iterator_traits: 
    public nontrivial_static_iterator_traits<Position, Next, nullptr, std::forward_iterator_tag, Pointer, GetPtr, Reference, GetRef>
{
};

} //namespace stl_tools {