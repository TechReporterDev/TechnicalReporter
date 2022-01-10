#pragma once
#include "TRWebServer.h"
#include <functional>
#include <boost\variant.hpp>
#include <boost\optional.hpp>
#include <boost\any.hpp>

#pragma warning(push)
#pragma warning(disable:4521)

class Value
{
public:
    Value() = default;
    Value(const Value&) = delete;
    Value& operator = (const Value&) = delete;
    virtual ~Value() = default;

    virtual void                        assign(const Value&) = 0;
    virtual std::shared_ptr<Value>      clone() const = 0;
};

template<class NativeType_>
class SingleValue: public Value
{
public:
    using NativeType = NativeType_;
    
    // Value override
    virtual void        assign(const Value& value) override;

    virtual NativeType  get() const = 0;
    virtual void        set(NativeType value) = 0;
};

template <class ValueType_>
class ValueArray: public Value
{
public:
    using ValueType = ValueType_;
    using Item = std::shared_ptr<ValueType>;
    
    // Value override
    virtual void    assign(const Value& value) override;

    virtual size_t  size() const = 0;
    virtual Item    getItem(size_t position) const = 0;
    virtual void    addItem(size_t position, const ValueType& value) = 0;
    virtual void    removeItem(size_t position) = 0;
};

class ValueCollection: public Value
{
public:
    using Member = std::shared_ptr<Value>;
    
    // Value override
    virtual void    assign(const Value& value) override;

    virtual size_t  size() const = 0;
    virtual Member  getMember(size_t position) const = 0;   
};

///////////////
// implementation
///////////////

template<class NativeType>
void SingleValue<NativeType>::assign(const Value& value)
{
    auto& singleValue = dynamic_cast<const SingleValue&>(value);
    set(singleValue.get());
}

template <class ValueType>
void ValueArray<ValueType>::assign(const Value& value)
{
    auto& valueArray = dynamic_cast<const ValueArray&>(value);
    while (size())
    {
        removeItem(0);
    }

    for (size_t position = 0; position < valueArray.size(); ++position)
    {
        addItem(position, *valueArray.getItem(position));
    }
}

#pragma warning(pop)