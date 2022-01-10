#pragma once
#include <functional>
#include <map>
#include <vector>
#include "finally_guard.h"
namespace stl_tools {

class ready_slot_pool
{
public:
    typedef std::function<void(void)>   slot_type;
    typedef int                         slot_priority_type;

    ready_slot_pool()
    {
        if (s_current_pool)
        {
            throw std::logic_error("Only single instance of 'ready_slot_pool' can exist at once");
        }
        s_current_pool = this;
    }

    ~ready_slot_pool()
    {
        s_current_pool = nullptr;
    }

    ready_slot_pool(const ready_slot_pool&) = delete;
    ready_slot_pool& operator = (const ready_slot_pool&) = delete;
    ready_slot_pool(ready_slot_pool&&) = delete;
    ready_slot_pool& operator = (ready_slot_pool&&) = delete;

    void insert(slot_type slot, slot_priority_type slot_priority)
    {
        m_ready_slots.insert(std::make_pair(slot_priority, slot));
    }

    void execute()
    {
        while (!m_ready_slots.empty())
        {
            auto head = m_ready_slots.begin();
            auto ready_slot = std::move(head->second);
            m_ready_slots.erase(head);
            ready_slot();
        }       
    }

    static ready_slot_pool* get_current()
    {       
        return s_current_pool;
    }

private:
    std::multimap<slot_priority_type, slot_type> m_ready_slots;
    static ready_slot_pool* s_current_pool;
};

template<class Signature>
class ordered_signal;

template<class... Args>
class ordered_signal< void(Args...) >
{
public:
    typedef int slot_priority_type;

    template<class T>
    void connect(T slot, slot_priority_type slot_priority)
    {
        m_slots.push_back(std::make_pair(slot, slot_priority));
    }

    template<class... _Args>
    void operator()(_Args&&... args)
    {
        if (ready_slot_pool::get_current())
        {
            emit(std::forward<_Args>(args)...);
        }
        else
        {
            ready_slot_pool pool;
            emit(std::forward<_Args>(args)...);
        }
    }   

private:
    template <class T>
    typename std::conditional<std::is_reference<T>::value, T, const T&>::type safe_ref(T& ref)
    {
        return ref;
    }

    template<class... _Args>
    void emit(_Args&&... args)
    {
        auto* current_pool = ready_slot_pool::get_current();
        _ASSERT(current_pool);

        prepare_slots(safe_ref<_Args>(args)...);
        current_pool->execute();
    }

    template<class... _Args>
    void prepare_slots(_Args&... args)
    {
        auto* current_pool = ready_slot_pool::get_current();
        _ASSERT(current_pool);

        for (auto& slot : m_slots)
        {
            current_pool->insert(std::bind(slot.first, std::ref(args)...), slot.second);        
        }
    }   

    std::vector<std::pair<std::function<void(Args...)>, slot_priority_type>> m_slots;
};

} //namespace stl_tools {