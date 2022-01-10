#pragma once
#include <string>
#include <boost\uuid\uuid.hpp>
namespace stl_tools {

typedef boost::uuids::uuid uuid;

uuid            gen_uuid(const std::string& str_uuid);
uuid            gen_uuid(const std::wstring& str_uuid);
uuid            gen_uuid();
uuid            null_uuid();
std::string     to_string(uuid arg);
std::wstring    to_wstring(uuid arg);

} //namespace stl_tools {