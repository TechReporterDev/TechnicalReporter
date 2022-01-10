#include "stdafx.h"
#include "Value.h"

void ValueCollection::assign(const Value& value)
{
    auto& valueCollection = dynamic_cast<const ValueCollection&>(value);
    if (size() != valueCollection.size())
    {
        throw std::logic_error("Invalid operation");
    }

    for (size_t position = 0; position < size(); ++position)
    {
        getMember(position)->assign(*valueCollection.getMember(position));
    }
}