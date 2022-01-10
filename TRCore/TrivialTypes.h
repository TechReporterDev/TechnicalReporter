#pragma once
#include "Restrictions.h"
#include <vector>
namespace TR { namespace Core {

//prefer typdef syntax, because of ODB does`t support C++11 'using'
typedef std::vector<char> Blob; 
typedef boost::uuids::uuid UUID;
typedef int Key;

static const Key INVALID_KEY = 0;
static const UUID INVALID_UUID = stl_tools::gen_uuid("2617E6B7-1BAC-4723-ACD1-37D5937617D1");

struct XmlQueryDocText: StringRestriction
{
    XmlQueryDocText() = default;
    
    explicit XmlQueryDocText(std::string text):
        StringRestriction(std::move(text))
    {
    }
};

struct XmlDefDocText: StringRestriction
{
    XmlDefDocText() = default;

    explicit XmlDefDocText(std::string text):
        StringRestriction(std::move(text))
    {
    }   
};

template <>
struct DefaultRestrictionValue<UUID>
{
    static UUID get_value()
    {
        return stl_tools::null_uuid();
    }
};

typedef Restriction<UUID> UUIDRestriction;
typedef Restriction<Key> KeyRestriction;

}} //namespace TR { namespace Core {