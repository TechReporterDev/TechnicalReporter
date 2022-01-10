#pragma once
#include <vector>
#include <list>
#include <type_traits>
#include "index.h"
#include "finally_guard.h"
namespace stl_tools {

struct storage_index_cathegory 
{
    struct unique_index {
    };

    struct single_index {
    };

    struct multi_index {
    };
};

struct storage_index_optional
{
    struct true_type{
    };

    struct false_type{
    };
};

template <class ValueType, class PrimaryIndexTraits, class... SecondaryIndexTraits>
class storage
{
public:
    using value_type = typename ValueType;
    using key_type = typename PrimaryIndexTraits::key_type;
    using container = std::list<value_type>;
    using iterator = typename container::iterator;
    using const_iterator = typename container::const_iterator;
    
    template<class... IndexTraits>
    struct index_node;

    template <class IndexTraits, class IndexNode>
    struct find_index_node;

    using root_index_type = index_node<PrimaryIndexTraits, SecondaryIndexTraits...>;
    
    storage() = default;
    storage(const storage&) = delete;
    storage& operator = (const storage&) = delete;
    
    static key_type get_key(const value_type& value)
    {
        return PrimaryIndexTraits::get_key(value);
    }

    iterator insert(value_type value)
    {
        auto position = m_container.insert(m_container.end(), std::move(value));
        auto guard = finally([&]() noexcept
        {
            m_container.erase(position);
        });
        
        indexate<root_index_type>(position);
        
        guard.active = false;
        return position;
    }

    void erase(const_iterator position) noexcept
    {
        erase_indexes<root_index_type>(position);
        m_container.erase(position);
    }

    const_iterator begin() const
    {
        return m_container.begin();
    }

    const_iterator end() const
    {
        return m_container.end();
    }

    iterator begin()
    {
        return m_container.begin();
    }

    iterator end()
    {
        return m_container.end();
    }

    const_iterator find(typename const key_type& key) const
    {
        return find<PrimaryIndexTraits>(key);
    }

    template <class IndexTraits>
    const_iterator find(typename const IndexTraits::key_type& key) const
    {
        static_assert(std::is_same<typename IndexTraits::cathegory, storage_index_cathegory::unique_index>::value, "Not available for non unique index");
        
        auto& index = get_index<typename find_index_node<IndexTraits, root_index_type>::type>();
        auto range = index.find_range(key);
        _ASSERT(boost::distance(range) <= 1);
        return !range.empty()? range.begin().base(): end();     
    }

    template <class IndexTraits>
    typename find_index_node<IndexTraits, root_index_type>::type::range_type find_range(typename const IndexTraits::key_type& key) const
    {
        auto& index = get_index<typename find_index_node<IndexTraits, root_index_type>::type>();
        return index.find_range(key);
    }

private:
    template<class Traits, class... NextTraits>
    struct index_node<Traits, NextTraits...> : index_node<NextTraits...>
    {
        using traits_type = Traits;
        using cathegory = typename Traits::cathegory;
        using key_type = typename Traits::key_type;
        using next_node = index_node<NextTraits...>;
        using index_type = stl_tools::index<key_type, const_iterator>;
        using range_type = typename index_type::index_iterator_range;

        index_node() = default;
        index_node(const index_node&) = delete;
        index_node& operator = (const index_node&) = delete;

        static key_type get_key(const value_type& value)
        {
            return Traits::get_key(value);
        }

        template<class F>
        static void enum_keys(const value_type& value, F f)
        {
            return enum_keys(value, f, typename Traits::cathegory());
        }

        template<class F, class Cathegory>
        static void enum_keys(const value_type& value, F f, Cathegory cat)
        {
            enum_keys(value, f, cat, typename Traits::optional());
        }

        template<class F, class Cathegory>
        static void enum_keys(const value_type& value, F f, Cathegory, storage_index_optional::true_type)
        {
            if (auto key = Traits::get_key(value))
            {
                f(*key);
            }
        }

        template<class F, class Cathegory>
        static void enum_keys(const value_type& value, F f, Cathegory, storage_index_optional::false_type)
        {
            f(Traits::get_key(value));
        }

        template<class F>
        static void enum_keys(const value_type& value, F f, storage_index_cathegory::multi_index)
        {
            Traits::enum_keys(value, f);
        }

        stl_tools::index<key_type, const_iterator> m_index;
    };

    template<>
    struct index_node<>
    {
    };
    
    template <class IndexTraits, class IndexNode, bool stop>
    struct find_next_node;

    template <class IndexTraits, class IndexNode>
    struct find_index_node
    {
        static_assert(!std::is_same<IndexNode, index_node<>>::value, "Index not found");
        static const bool stop = std::is_same<IndexTraits, typename IndexNode::traits_type>::value;     
        using type = typename find_next_node<IndexTraits, IndexNode, stop>::type;
    };

    template <class IndexTraits, class IndexNode>
    struct find_next_node<IndexTraits, IndexNode, true>
    {
        using type = IndexNode;
    };

    template <class IndexTraits, class IndexNode>
    struct find_next_node<IndexTraits, IndexNode, false>
    {
        using type = typename find_index_node<IndexTraits, typename IndexNode::next_node>::type;
    };

    template<class IndexNode>
    void indexate(const_iterator position)
    {
        auto& index = get_index<IndexNode>();
        auto guard = finally([&]() noexcept
        {
            index.erase(position);
        });
    
        IndexNode::enum_keys(*position, [&](const typename IndexNode::key_type& key)
        {
            if (std::is_same<typename IndexNode::cathegory, storage_index_cathegory::unique_index>::value)
            {
                if (!index.find_range(key).empty())
                {
                    throw std::logic_error("Duplicate of unique index found");
                }
            }
            index.insert(key, position);            
        });

        indexate<typename IndexNode::next_node>(position);
        guard.active = false;
    }

    template<>
    void indexate<index_node<>>(const_iterator position)
    {
    }

    template<class IndexNode>
    void erase_indexes(const_iterator position) noexcept
    {
        auto& index = get_index<IndexNode>();
        index.erase(position);
        erase_indexes<typename IndexNode::next_node>(position);     
    }

    template<>
    void erase_indexes<index_node<>>(const_iterator position) noexcept
    {
    }

    template<class IndexNode>
    const typename IndexNode::index_type& get_index() const
    {
        return static_cast<const typename IndexNode&>(m_index_root).m_index;
    }

    template<class IndexNode>
    typename IndexNode::index_type& get_index()
    {
        return static_cast<typename IndexNode&>(m_index_root).m_index;
    }

    container m_container;
    root_index_type m_index_root;
};

template<class KeyType, class Optional = storage_index_optional::false_type>
struct unique_storage_index
{
    using cathegory = storage_index_cathegory::unique_index;
    using key_type = KeyType;
    using optional = Optional;
};

template<class KeyType, class Optional = storage_index_optional::false_type>
struct single_storage_index
{
    using cathegory = storage_index_cathegory::single_index;
    using key_type = KeyType;
    using optional = Optional;
};

template<class KeyType>
struct multi_storage_index
{
    using cathegory = storage_index_cathegory::multi_index;
    using key_type = KeyType;
};

template<class KeyType, class ValueType, KeyType (ValueType::*f)() const, class Cathegory = storage_index_cathegory::single_index>
struct memfun_index_traits
{
    using cathegory = Cathegory;
    using key_type = KeyType;
    using optional = storage_index_optional::false_type;
    
    static KeyType get_key(const ValueType& value)
    {
        return (value.*f)();
    }

    static KeyType get_key(const std::unique_ptr<ValueType>& value)
    {
        return ((*value).*f)();
    }
};

template<class KeyType, class ValueType, KeyType ValueType::* pm, class Cathegory = storage_index_cathegory::single_index>
struct memdata_index_traits
{
    using cathegory = Cathegory;
    using key_type = KeyType;
    using optional = storage_index_optional::false_type;

    static KeyType get_key(const ValueType& value)
    {
        return value.*pm;
    }

    static KeyType get_key(const std::unique_ptr<ValueType>& value)
    {
        return (*value).*pm;
    }
};

} //namespace stl_tools {