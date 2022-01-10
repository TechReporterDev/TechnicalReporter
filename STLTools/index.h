#pragma once
#include <vector>
#include <map>
#include <boost\range\adaptor\map.hpp>
#include "iterator_adapter.h"
#include "finally_guard.h"
namespace stl_tools {

template <class Key, class Iterator>
class index
{
public:
    typedef typename std::multimap<Key, Iterator> it_by_key;    
    struct index_iterator_traits: stl_tools::iterator_traits<
        typename it_by_key::const_iterator,
        std::forward_iterator_tag, 
        typename Iterator::value_type, 
        typename Iterator::pointer,
        typename Iterator::reference,
        typename Iterator>
    {
        static position_type next(position_type position)
        {
            return ++position;
        };

        static position_type prev(position_type position)
        {           
            return --position;
        };

        static pointer get_ptr(position_type position)
        {
            return &*position->second;
        };

        static reference get_ref(position_type position)
        {
            return *position->second;
        };

        static base_type get_base(position_type position)
        {
            return position->second;
        };
    };
    
    typedef stl_tools::iterator_adapter<index_iterator_traits> index_iterator;
    typedef boost::iterator_range<index_iterator> index_iterator_range;

    void insert(const Key& key, Iterator it)
    {
        _ASSERT(! exist(key, it));
        auto pos = m_it_by_key.insert(std::make_pair(key, it));
        auto guard = finally(
            [&]{m_it_by_key.erase(pos); 
        });

        m_pos_by_it.insert(std::make_pair(it, pos));
        guard.active = false;
    }

    index_iterator_range find_range(const Key& key) const
    {
        auto range = m_it_by_key.equal_range(key);
        return boost::make_iterator_range(index_iterator(range.first), index_iterator(range.second));
    }

    void erase(Iterator it)
    {
        auto range = m_pos_by_it.equal_range(it);
        for (auto pos : boost::make_iterator_range(range) | boost::adaptors::map_values)
        {
            m_it_by_key.erase(pos);
        }
        m_pos_by_it.erase(range.first, range.second);
    }

    bool exist(const Key& key, Iterator it) const
    {
        auto range = m_pos_by_it.equal_range(it);
        for (auto pos : boost::make_iterator_range(range) | boost::adaptors::map_values)
        {
            if (equal_keys(pos->first, key))
            {
                return true;
            }
        }
        return false;
    }

private:
    struct Less: public std::binary_function<Iterator, Iterator, bool>
    {
        bool operator ()(const Iterator& left, const Iterator& right) const
        {
            return &*left < &*right;
        }
    };

    static bool equal_keys(const Key& left, const Key& right)
    {
        return !(left < right || right < left);
    }

    typedef typename std::multimap<Key, Iterator>::iterator Position;
    std::multimap<Key, Iterator> m_it_by_key;
    std::multimap<Iterator, Position, Less> m_pos_by_it;
};

} //namespace stl_tools {