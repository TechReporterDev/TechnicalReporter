#pragma once
#include "Value.h"
#include "RefValue.h"
#include "CustomValue.h"
#include "SelfArray.h"

template<class ValueType_, class Container>
class ContainerArray: public ValueArray<ValueType_>
{
public:
    using GetItemImpl = std::function<Item(typename Container::reference)>;
    ContainerArray(Container& container, GetItemImpl getItemImpl):
        m_container(container),
        m_getItemImpl(getItemImpl)
    {
    }

    virtual std::shared_ptr<Value> clone() const override
    {
        auto selfArray = std::make_shared<SelfArray<ValueType>>();
        selfArray->assign(*this);
        return selfArray;
    }

    virtual size_t size() const override
    {
        return m_container.size();
    }

    virtual Item getItem(size_t position) const override
    {
        return m_getItemImpl(*std::next(m_container.begin(), position));
    }

    virtual void addItem(size_t position, const ValueType& value) override
    {
        auto it = m_container.insert(std::next(m_container.begin(), position), typename Container::value_type());
        m_getItemImpl(*it)->assign(value);
    }

    virtual void removeItem(size_t position) override
    {
        m_container.erase(std::next(m_container.begin(), position));
    }

private:
    Container& m_container;
    GetItemImpl m_getItemImpl;
};

template<class ValueType, class Container>
std::shared_ptr<ContainerArray<ValueType, Container>> containerArray(Container& container,
    typename ContainerArray<ValueType, Container>::GetItemImpl getItemImpl)
{
    return std::make_shared<ContainerArray<ValueType, Container>>(container, getItemImpl);
}

template<class T, class Container>
std::shared_ptr<ContainerArray<SingleValue<T>, Container>> containerArray(Container& container,
    typename T Container::value_type::* memfn)
{
    return containerArray<SingleValue<T>, Container>(container, [memfn](typename Container::reference value){
        return refValue(value.*memfn);
    });
}

template<class Container>
std::shared_ptr<ContainerArray<SingleValue<typename Container::value_type>, Container>> containerArray(Container& container)
{
    return containerArray<SingleValue<typename Container::value_type>, Container>(container, (typename Container::reference value){
        return refValue(value);
    });
}

inline std::shared_ptr<ContainerArray<SingleValue<bool>, std::vector<bool>>> containerArray(std::vector<bool>& container)
{
    return containerArray<SingleValue<bool>, std::vector<bool>>(container, [](std::vector<bool>::reference ref){
        return customValue<bool>(
            [=]{ return ref; }, 
            [=](bool value) mutable { ref = value; }
        );
    });
}