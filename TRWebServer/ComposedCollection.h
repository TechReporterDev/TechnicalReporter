#pragma once
#include "Value.h"
#include "RefValue.h"

class ComposedCollection: public ValueCollection
{
public:
    ComposedCollection() = default;
    
    ComposedCollection(std::vector<Member> members):
        m_members(std::move(members))
    {
    }

    virtual std::shared_ptr<Value> clone() const override
    {
        auto valueCollection = std::make_shared<ComposedCollection>();
        for (auto& member : m_members)
        {
            valueCollection->appendMember(member->clone());
        }
        return valueCollection;
    }

    void appendMember(Member member) 
    {
        m_members.push_back(member);
    }

    virtual size_t size() const override
    {
        return m_members.size();
    }

    virtual Member getMember(size_t position) const override
    {
        return m_members[position];
    }

private:
    std::vector<Member> m_members;
};

inline std::shared_ptr<ComposedCollection> composedCollection(std::vector<ComposedCollection::Member> members)
{
    return std::make_shared<ComposedCollection>(std::move(members));
}