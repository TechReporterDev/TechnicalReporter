#pragma once
#include <odb\database.hxx>
#include <odb\transaction.hxx>
#include "BackgndFwd.h"
#include "stddecl.h"
namespace TR { namespace Core {

class Activator
{
public:
    Activator(bool value);
    void activate(bool value = true) noexcept;
    bool get_value() const;

private:
    std::shared_ptr<bool> m_value;
};

class Transaction
{
public:
    Transaction(Database& db);
    ~Transaction();

    void        commit();
    void        rollback();

    Activator   connect_commit(std::function<void()> slot, bool activate = true);
    Activator   connect_rollback(std::function<void()> slot, bool activate = true);

private:
    Signal<void()> m_commit_sig;
    Signal<void()> m_rollback_sig;
    odb::transaction m_t;
};

class ReadOnlyTransaction
{
public:
    ReadOnlyTransaction(Database& db);

private:
    odb::transaction m_t;
};

template <class T>
void assign(T& target, T value)
{
    target = std::move(value);
}

template<class T, class V>
T& transact_assign(T& target, V&& value, Transaction& t)
{
    auto activator = t.connect_rollback(std::bind(&assign<T>, std::ref(target), stl_tools::force_move(target)), false);
    target = std::forward<V>(value);
    activator.activate();
    return target;
}

template <class T>
void swap_args(T& right, T& left)
{
    std::swap(left, right);
}

template<class T>
void transact_swap(T& left, T& right, Transaction& t)
{
    auto activator = t.connect_rollback(std::bind(&swap_args<T>, std::ref(left), std::ref(right)), false);
    std::swap(left, right);
    activator.activate();   
}

template<class Action, class Rollback, stl_tools::disable_void_t<std::result_of_t<Action()>> = 0>
typename std::result_of<Action()>::type transact_action(Transaction& t, Action action, Rollback rollback)
{
    auto activator = t.connect_rollback(std::move(rollback), false);
    auto result = action(); 
    activator.activate();   
    return result;
}

template<class Action, class Rollback, stl_tools::enable_void_t<std::result_of_t<Action()>> = 0>
void transact_action(Transaction& t, Action action, Rollback rollback)
{
    auto activator = t.connect_rollback(std::move(rollback), false);
    action();
    activator.activate();   
}

template<class Storage, class Value>
typename Storage::iterator transact_insert(Storage& storage, Value&& value, Transaction& t)
{
    auto key = Storage::get_key(value);
    auto activator = t.connect_rollback([&storage, key](){
        auto position = storage.find(key);
        _ASSERT(position != storage.end());
        storage.erase(position);
    }, false);

    auto position = storage.insert(std::forward<Value>(value));
    activator.activate();
    return position;
}

template<class Storage>
typename Storage::const_iterator make_storage_iterator(typename Storage::const_iterator position)
{
    return position;
}

template<class Storage>
typename Storage::const_iterator make_storage_iterator(typename Storage::iterator position)
{
    return position;
}

template<class Storage, class Iterator>
typename Storage::const_iterator make_storage_iterator(Iterator position)
{
    return position.base();
}

template<class Storage, class Iterator>
void transact_erase(Storage& storage, Iterator position, Transaction& t)
{
    auto backup = std::make_shared<boost::optional<typename Storage::value_type>>();
    auto activator = t.connect_rollback([&storage, backup](){
        storage.insert(std::move(**backup));
    }, false);

    *backup = std::move(const_cast<typename Storage::value_type&>(*position));
    storage.erase(make_storage_iterator<Storage>(position));
    activator.activate();
}

template<class Storage, class Range>
void transact_erase_range(Storage& storage, const Range& range, Transaction& t)
{
    for (auto i = range.begin(); i != range.end();)
    {
        transact_erase(storage, i++, t);
    }
}

template<class Storage, class Value>
typename Storage::iterator transact_update(Storage& storage, Value&& value, Transaction& t)
{
    auto key = Storage::get_key(value);
    auto position = storage.find(key);
    _ASSERT(position != storage.end());

    transact_erase(storage, position, t);
    return transact_insert(storage, std::forward<Value>(value), t);
}

}} //namespace TR { namespace Core {