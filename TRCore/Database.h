#pragma once
#include "BackgndFwd.h"
#include "stddecl.h"
namespace TR { namespace Core {

extern const wchar_t* default_schema;

void                        install_database(const std::wstring& user, const std::wstring& passwd, const std::wstring& host, const std::wstring& schema = default_schema);
void                        uninstall_database(const std::wstring& user, const std::wstring& passwd, const std::wstring& host, const std::wstring& schema = default_schema);
std::unique_ptr<Database>   connect_database(const std::wstring& user, const std::wstring& passwd, const std::wstring& host, const std::wstring& schema = default_schema);
bool                        verify_database(const std::wstring& user, const std::wstring& passwd, const std::wstring& host, const std::wstring& schema = default_schema);

}} //namespace TR { namespace Core {