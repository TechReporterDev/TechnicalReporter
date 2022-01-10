#include "stdafx.h"
#include "uuid.h"
namespace stl_tools {

uuid gen_uuid(const std::string& str_uuid)
{
    static boost::uuids::string_generator generator;
    return generator(str_uuid);
}

uuid gen_uuid(const std::wstring& str_uuid)
{
    static boost::uuids::string_generator generator;
    return generator(str_uuid);
}

uuid gen_uuid()
{
    static boost::uuids::random_generator generator;
    return generator();
}

uuid null_uuid()
{
    return boost::uuids::nil_uuid();
}

std::string to_string(uuid arg)
{
    return boost::lexical_cast<std::string>(arg);
}

std::wstring to_wstring(uuid arg)
{
    return boost::lexical_cast<std::wstring>(arg);
}

} //namespace stl_tools {