#pragma once
#include <list>

namespace stl_tools {

template<class T>
class container_initializer
{
public:
    using container = std::list<T>;
    using iterator = typename container::iterator;
    using move_iterator = std::move_iterator<iterator>;

    template<class... Args>
    container_initializer(Args&&... args)
    {
        construct(std::forward<Args>(args)...);
    }

    container_initializer(const container_initializer&) = delete;
    container_initializer(container_initializer&& ci):
        m_list(std::move(ci.m_list))
    {
    }

    container_initializer& operator = (const container_initializer&) = delete;
    container_initializer& operator = (container_initializer&& ci)
    {
        m_list = std::move(ci.m_list);
        return *this;
    }
    
    iterator begin()
    {
        return m_list.begin();
    }

    iterator end()
    {
        return m_list.end();
    }

    move_iterator move_begin()
    {
        return std::make_move_iterator(m_list.begin());
    }

    move_iterator move_end()
    {
        return std::make_move_iterator(m_list.end());
    }

private:
    template<class Arg, class... Args>
    void construct(Arg&& arg, Args&&... args)
    {
        m_list.push_back(std::forward<Arg>(arg));
        construct(std::forward<Args>(args)...);
    }

    void construct(){}

    std::list<T> m_list;
};

} //namespace stl_tools {