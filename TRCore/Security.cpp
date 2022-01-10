#include "stdafx.h"
#include "Security.h"
#include "CoreDomain.h"
#include "Transaction.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
#include "SecurityDataset.h"
namespace TR { namespace Core {

void Security::install(Database& db)
{
    UserData user_data(0, "admin");
    auto key = db.persist(user_data);
    _ASSERT(key == 1);

    AccessEntryData access_entry_data(SourceKey(1), key, int(Access::FULL_ACCESS));
    db.persist(access_entry_data);
}

Security::Security(Database& db, CoreDomain& core_domain):
    m_db(db),
    m_sources(core_domain.m_registry.m_sources),
    m_access_entry_dataset(std::make_unique<AccessEntryDataset>(m_db))
{
    m_sources.connect_remove_source([this](SourceKey source_key, const std::vector<SourceKey>& removed_sources, Transaction& t){
        on_remove_sources(removed_sources, t);
    }, SECURITY_PRIORITY);
}

Security::~Security()
{
}

static Key next_session_key()
{
    static Key prev_key = 0;
    return ++prev_key;
}

Key Security::open_session(const std::wstring& user_name)
{
    ReadOnlyTransaction t(m_db);
    auto result = m_db.query<UserData>(odb::query<UserData>::name == stl_tools::ucs_to_utf8(user_name));
    if (result.empty())
    {
        throw Exception(L"Invalid user name");
    }

    auto next_key = next_session_key();
    m_sessions.insert(std::make_pair(next_key, Session{next_key, result.begin()->m_key}));
    m_open_session_sig(next_key);

    return next_key;
}

void Security::close_session(Key session_key)
{
    auto position = m_sessions.find(session_key);
    if (position == m_sessions.end())
    {
        throw Exception(L"Invalid session key");
    }
    m_sessions.erase(position);
    m_close_session_sig(session_key);
}

Session Security::get_session(Key session_key) const
{
    auto position = m_sessions.find(session_key);
    if (position == m_sessions.end())
    {
        throw Exception(L"Invalid session key");
    }
    return position->second;
}

std::vector<Session> Security::get_sessions() const
{
    auto sessions = m_sessions | boost::adaptors::map_values;
    return {sessions.begin(), sessions.end()};
}

Key Security::add_user(const std::wstring& name, Transaction& t)
{
    UserData user_data(0, stl_tools::ucs_to_utf8(name));
    return m_db.persist(user_data);
}

void Security::update_user(Key user_key, const std::wstring& name, Transaction& t)
{
    UserData user_data(user_key, stl_tools::ucs_to_utf8(name));
    m_db.update(user_data);
}

void Security::remove_user(Key user_key, Transaction& t)
{   
    m_db.erase<UserData>(user_key); 
    m_access_entry_dataset->erase_user_equal(user_key, t);
    m_remove_user_sig(user_key, t);
}

std::vector<User> Security::get_users() const
{
    ReadOnlyTransaction t(m_db);
    std::vector<User> result;
    for (auto& user_data : m_db.query<UserData>())
    {
        result.push_back({user_data.m_key, stl_tools::utf8_to_ucs(user_data.m_name)});
    }
    return result;
}

boost::optional<User> Security::find_user(const std::wstring& user_name) const
{
    ReadOnlyTransaction t(m_db);
    auto result = m_db.query<UserData>(odb::query<UserData>::name == stl_tools::ucs_to_utf8(user_name));
    if (!result.empty())
    {
        return User(result.begin()->m_key, user_name);
    }
    return boost::none;
}

boost::optional<User> Security::find_user(Key user_key) const
{
    ReadOnlyTransaction t(m_db);
    UserData user_data;
    if (m_db.find<UserData>(user_key, user_data))
    {
        return User(user_data.m_key, stl_tools::utf8_to_ucs(user_data.m_name));
    }
    return boost::none;
}

boost::optional<AccessRights> Security::get_access_rights(Key user_key, SourceRef source_ref) const
{
    ReadOnlyTransaction t(m_db);
    if (auto access_entry_data = m_access_entry_dataset->get(user_key, source_ref.get_key()))
    {
        return Access(access_entry_data->m_access_rights);
    }
    return boost::none;
}

bool Security::check_access_rights(Key user_key, SourceRef source_ref, AccessRights access_rights) const
{
    return get_ace(user_key, source_ref).m_access_rights.contains(access_rights);
}

void Security::set_access_rights(Key user_key, SourceRef source_ref, AccessRights access_rights, Transaction& t)
{
    AccessEntryData access_entry_data(pack(source_ref), user_key, access_rights.get_value());
    m_access_entry_dataset->put(access_entry_data, t);
}

void Security::reset_access_rights(Key user_key, SourceRef source_ref, Transaction& t)
{
    m_access_entry_dataset->clear(user_key, source_ref.get_key(), t);
}

ACE Security::get_ace(Key user_key, SourceRef source_ref) const
{
    if (auto access_rights = get_access_rights(user_key, source_ref))
    {
        return ACE(source_ref, user_key, *access_rights);
    }

    return get_default_ace(user_key, *source_ref);  
}

ACE Security::get_default_ace(Key user_key, Source source) const
{
    if (!source.has_parent())
    {
        return ACE(source, user_key, Access::EMPTY_ACCESS);
    }
    return get_ace(user_key, source.get_parent());
}

ACL Security::get_acl(SourceRef source_ref) const
{
    ReadOnlyTransaction t(m_db);
    ACL acl;
    for (auto& access_entry_data : m_access_entry_dataset->query_source_equal(source_ref.get_key()))
    {
        acl.push_back({source_ref, access_entry_data.m_identity.m_user_key, Access(access_entry_data.m_access_rights)});        
    }
    return acl;
}

void Security::set_acl(SourceRef source_ref, const ACL& acl) const
{
    Transaction t(m_db);
    m_access_entry_dataset->erase_source_equal(source_ref.get_key(), t);
    for (auto& ace : acl)
    {
        AccessEntryData access_entry_data(pack(source_ref), ace.m_user_key, ace.m_access_rights.get_value());
        m_access_entry_dataset->put(access_entry_data, t);
    }
    t.commit();
}

void Security::on_remove_sources(const std::vector<SourceKey>& removed_sources, Transaction& t)
{
    m_access_entry_dataset->erase_source_equal(removed_sources, t);
}

}} //namespace TR { namespace Core {