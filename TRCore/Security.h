#pragma once
#include "AppDecl.h"
namespace TR { namespace Core {
class AccessEntryDataset;
enum class Access { EMPTY_ACCESS = 0x00, READ_ACCESS = 0x01, WRITE_ACCESS = 0x02, EXEC_ACCESS = 0x04, FULL_ACCESS = 0x07 };
using AccessRights = stl_tools::flag_type<Access>;

struct Session
{
    Session(Key key, Key user_key):
        m_key(key),
        m_user_key(user_key)
    {
    }

    Key m_key;
    Key m_user_key;
};

struct User
{
    User(Key key, std::wstring name):
        m_key(key),
        m_name(name)
    {
    }

    Key m_key;
    std::wstring m_name;    
};

struct ACE
{
    ACE(SourceRef source_ref, Key user_key, AccessRights access_rights):
        m_source_ref(source_ref),
        m_user_key(user_key),
        m_access_rights(access_rights)
    {
    }

    SourceRef       m_source_ref;
    Key             m_user_key;
    AccessRights    m_access_rights;
};

using ACL = std::vector<ACE>;

class Security
{
public:
    static const Priority SECURITY_PRIORITY = 10000;
    
    static void install(Database& db);

    Security(Database& db, CoreDomain& core_domain);
    ~Security();

    Security(const Security&) = delete;
    Security& operator = (const Security&) = delete;
    Security(Security&&) = delete;
    Security& operator = (Security&&) = delete;

    // session
    Key                                     open_session(const std::wstring& user_name);
    void                                    close_session(Key session_key);
    Session                                 get_session(Key session_key) const;
    std::vector<Session>                    get_sessions() const;

    // users
    Key                                     add_user(const std::wstring& name, Transaction& t);
    void                                    update_user(Key user_key, const std::wstring& name, Transaction& t);
    void                                    remove_user(Key user_key, Transaction& t);
    std::vector<User>                       get_users() const;
    boost::optional<User>                   find_user(const std::wstring& user_name) const;
    boost::optional<User>                   find_user(Key user_key) const;

    // access
    boost::optional<AccessRights>           get_access_rights(Key user_key, SourceRef source_ref) const;
    bool                                    check_access_rights(Key user_key, SourceRef source_ref, AccessRights access_rights) const;
    void                                    set_access_rights(Key user_key, SourceRef source_ref, AccessRights access_rights, Transaction& t);
    void                                    reset_access_rights(Key user_key, SourceRef source_ref, Transaction& t);
    
    ACE                                     get_ace(Key user_key, SourceRef source_ref) const;
    ACE                                     get_default_ace(Key user_key, Source source) const;
    ACL                                     get_acl(SourceRef source_ref) const;
    void                                    set_acl(SourceRef source_ref, const ACL& acl) const;

    template<class T>
    void connect_open_session(T slot, Priority priority) { m_open_session_sig.connect(slot, priority); }

    template<class T>
    void connect_close_session(T slot, Priority priority) { m_close_session_sig.connect(slot, priority); }

    template<class T>
    void connect_remove_user(T slot, Priority priority) { m_remove_user_sig.connect(slot, priority); }

private:
    void                                    on_remove_sources(const std::vector<SourceKey>& removed_sources, Transaction& t);

    std::map<Key, Session> m_sessions;  
    Database& m_db;
    Sources& m_sources;
    std::unique_ptr<AccessEntryDataset> m_access_entry_dataset;

    OrderedSignal<void(Key, Transaction&)> m_remove_user_sig;
    OrderedSignal<void(Key)> m_open_session_sig;
    OrderedSignal<void(Key)> m_close_session_sig;
};

}} //namespace TR { namespace Core {