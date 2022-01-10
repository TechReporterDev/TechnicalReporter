#pragma once
#include "RegistryDecl.h"
namespace TR { namespace Core {
class RoleDataset;

class Roles
{
public:
    static const Priority ROLES_PRIORITY = 350;
    static void             install(Database& db);

    Roles();
    ~Roles();
    void                    set_database(Database* db);
    void                    run();

    RoleRef                 add_role(Role role);
    RoleRef                 add_role(Role role, Transaction& t);
    void                    update_role(Role role);
    void                    update_role(Role role, Transaction& t);
    std::vector<Role>       get_roles() const;
    std::vector<Role>       get_roles(RoleRef parent_ref) const;
    Role                    get_role(RoleKey role_key) const;
    Role                    get_default_role() const;
    bool                    has_role(RoleKey role_key) const;
    void                    remove_role(RoleKey role_key);
    void                    remove_role(RoleKey role_key, Transaction& t);

    template<class T>
    void connect_add_role(T slot, Priority priority) { m_add_role_sig.connect(slot, priority); }

    template<class T>
    void connect_update_role(T slot, Priority priority) { m_update_role_sig.connect(slot, priority); }

    template<class T>
    void connect_replace_role(T slot, Priority priority) { m_replace_role_sig.connect(slot, priority); }

    template<class T>
    void connect_remove_role(T slot, Priority priority) { m_remove_role_sig.connect(slot, priority); }

private:
    std::vector<RoleKey>    get_childs(RoleKey role_key) const;
    bool                    ancestor_of(RoleKey ancestor_key, RoleKey descendant_key) const;

    OrderedSignal<void(RoleRef role_ref, Transaction& t)> m_add_role_sig;
    OrderedSignal<void(RoleRef role_ref, Transaction& t)> m_update_role_sig;
    OrderedSignal<void(RoleRef role_ref, RoleRef prev_parent_ref, RoleRef next_parent_ref, Transaction& t)> m_replace_role_sig;
    OrderedSignal<void(RoleKey role_key, Transaction& t)> m_remove_role_sig;

    Database* m_db;
    std::unique_ptr<RoleDataset> m_role_dataset;
};

class Role
{
public:
    friend class Roles;
    Role(std::wstring name, RoleRef parent_ref);

    std::wstring                        get_name() const;
    void                                set_name(std::wstring name);
    RoleRef                             get_parent() const;
    void                                set_parent(RoleRef parent_ref);
    bool                                has_parent() const;

    RoleRef                             get_ref() const;
    operator RoleRef() const;

private:
    Role(RoleRef ref, std::wstring name, boost::optional<RoleRef> parent_ref);

    std::wstring m_name;
    boost::optional<RoleRef> m_parent_ref;
    boost::optional<RoleRef> m_ref;
};

}} //namespace TR { namespace Core {