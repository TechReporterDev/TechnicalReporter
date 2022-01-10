#include "stdafx.h"
#include "Database.h"
namespace TR { namespace Core {
using namespace stl_tools;

const wchar_t* default_schema = L"tech_reporter";

void install_database(const std::wstring& user, const std::wstring& passwd, const std::wstring& host, const std::wstring& schema)
{
    std::unique_ptr<Database> db(new odb::mysql::database(ucs_to_ansi(user), ucs_to_ansi(passwd), "", ucs_to_ansi(host), 3306));
    odb::transaction t(db->begin());
    db->execute((boost::format("CREATE DATABASE %1%;") % ucs_to_ansi(schema)).str());
    t.commit(); 
}

void uninstall_database(const std::wstring& user, const std::wstring& passwd, const std::wstring& host, const std::wstring& schema)
{
    std::unique_ptr<Database> db(new odb::mysql::database(ucs_to_ansi(user), ucs_to_ansi(passwd), "", ucs_to_ansi(host), 3306));
    odb::transaction t(db->begin ());
    db->execute((boost::format("DROP DATABASE IF EXISTS %1%;") % ucs_to_ansi(schema)).str());
    t.commit(); 
}

std::unique_ptr<Database> connect_database(const std::wstring& user, const std::wstring& passwd, const std::wstring& host, const std::wstring& schema)
{
    return std::make_unique<odb::mysql::database>(ucs_to_ansi(user), ucs_to_ansi(passwd), ucs_to_ansi(schema), ucs_to_ansi(host), 3306);
}

bool verify_database(const std::wstring& user, const std::wstring& passwd, const std::wstring& host, const std::wstring& schema)
{
    try
    {
        odb::mysql::database db(ucs_to_ansi(user), ucs_to_ansi(passwd), ucs_to_ansi(schema), ucs_to_ansi(host), 3306);
        odb::transaction t(db.begin());
    }
    catch(std::exception&)
    {
        return false;
    }
    return true;
}

}} //namespace TR { namespace Core {