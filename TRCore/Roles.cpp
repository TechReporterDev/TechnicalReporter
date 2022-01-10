#include "stdafx.h"
#include "Roles.h"
#include "Transaction.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
#include "RoleDataset.h"

namespace TR { namespace Core {

static const auto DEFAULT_ROLE_KEY = RoleKey(1);

void Roles::install(Database& db)
{
    RoleData default_role_data(0, "Default", boost::none);
    Key default_role_key = db.persist(default_role_data);
    _ASSERT(default_role_key == DEFAULT_ROLE_KEY);
}

Roles::Roles():
    m_db(nullptr)
{
}

Roles::~Roles()
{
}

void Roles::set_database(Database* db)
{
    m_db = db;
    m_role_dataset = std::make_unique<RoleDataset>(*db);
}

void Roles::run()
{
}

RoleRef Roles::add_role(Role role)
{
    Transaction t(*m_db);
    auto role_ref = add_role(std::move(role), t);
    t.commit();
    return role_ref;
}

RoleRef Roles::add_role(Role role, Transaction& t)
{
    RoleData role_data(
        0,
        stl_tools::ucs_to_utf8(role.m_name),
        pack(role.m_parent_ref));

    RoleRef role_ref(this, m_role_dataset->persist(role_data, t));
    m_add_role_sig(role_ref, t);
    return role_ref;
}

void Roles::update_role(Role role)
{
    Transaction t(*m_db);
    update_role(std::move(role), t);
    t.commit();
}

void Roles::update_role(Role role, Transaction& t)
{
    if (!role.m_ref)
    {
        throw Exception(L"Can`t update not persistent role");
    }
        
    if (role.m_parent_ref)
    {
        if (ancestor_of(role.m_ref->get_key(), role.m_parent_ref->get_key()))
        {
            throw Exception(L"Cyclic source dependency found");
        }
    }

    RoleData role_data(
        role.m_ref->get_key(),
        stl_tools::ucs_to_utf8(role.m_name),
        pack(role.m_parent_ref));

    auto prev_role = get_role(role.m_ref->get_key());
    m_role_dataset->update(role_data, t);   
    m_update_role_sig(role, t);

    if (prev_role.m_parent_ref != role.m_parent_ref)
    {
        m_replace_role_sig(role, prev_role.get_parent(), role.get_parent(), t);
    }
}

std::vector<Role> Roles::get_roles() const
{
    ReadOnlyTransaction t(*m_db);
    std::vector<Role> roles;
    for (auto& role_data : m_role_dataset->query())
    {
        roles.push_back({
            RoleRef(this, RoleKey(role_data.m_key)),
            stl_tools::utf8_to_ucs(role_data.m_name),
            unpack(role_data.m_parent_key, this)
        });
    }
    return roles;
}

std::vector<Role> Roles::get_roles(RoleRef parent_ref) const
{
    ReadOnlyTransaction t(*m_db);
    std::vector<Role> roles;
    for (auto& role_data : m_role_dataset->query_parent_equal(parent_ref.get_key()))
    {
        roles.push_back({
            RoleRef(this, RoleKey(role_data.m_key)),
            stl_tools::utf8_to_ucs(role_data.m_name),
            unpack(role_data.m_parent_key, this)
        });
    }
    return roles;
}

Role Roles::get_role(RoleKey role_key) const
{
    ReadOnlyTransaction t(*m_db);
    RoleData role_data = m_role_dataset->load(role_key);
    return Role(
        RoleRef(this, role_key),
        stl_tools::utf8_to_ucs(role_data.m_name),
        unpack(role_data.m_parent_key, this)
    );
}

Role Roles::get_default_role() const
{
    ReadOnlyTransaction t(*m_db);
    return get_role(DEFAULT_ROLE_KEY);
}

bool Roles::has_role(RoleKey role_key) const
{
    ReadOnlyTransaction t(*m_db);
    return m_role_dataset->find(role_key).is_initialized();
}

void Roles::remove_role(RoleKey role_key)
{
    Transaction t(*m_db);
    remove_role(role_key, t);
    t.commit();
}

void Roles::remove_role(RoleKey role_key, Transaction& t)
{
    if (role_key == DEFAULT_ROLE_KEY)
    {
        throw Exception(L"Can`t remove default role");
    }

    for (auto child_key : get_childs(role_key))
    {
        remove_role(child_key, t);
    }
    m_role_dataset->erase(role_key, t);
    m_remove_role_sig(role_key, t);
}

std::vector<RoleKey> Roles::get_childs(RoleKey role_key) const
{
    ReadOnlyTransaction t(*m_db);
    std::vector<RoleKey> child_roles;
    for (auto& role_data : m_role_dataset->query_parent_equal(role_key))
    {
        child_roles.push_back(RoleKey(role_data.m_key));
    }
    return child_roles;
}

bool Roles::ancestor_of(RoleKey ancestor_key, RoleKey descendant_key) const
{
    ReadOnlyTransaction t(*m_db);
    if (ancestor_key == descendant_key)
    {
        return true;
    }

    RoleData descendant_data = m_role_dataset->load(descendant_key);
    if (!descendant_data.m_parent_key)
    {
        return false;
    }

    return ancestor_of(ancestor_key, *descendant_data.m_parent_key);
}

Role::Role(std::wstring name, RoleRef parent_ref):
    m_name(std::move(name)),
    m_parent_ref(parent_ref)
{
}

std::wstring Role::get_name() const
{
    return m_name;
}

void Role::set_name(std::wstring name)
{
    m_name = std::move(name);
}

RoleRef Role::get_parent() const
{
    _ASSERT(m_parent_ref);
    if (!m_parent_ref)
    {
        throw Exception(L"Default role has no parent");
    }
    return *m_parent_ref;
}

void Role::set_parent(RoleRef parent_ref)
{
    m_parent_ref = parent_ref;
}

bool Role::has_parent() const
{
    return m_parent_ref.is_initialized();
}

RoleRef Role::get_ref() const
{
    if (!m_ref)
    {
        throw Exception(L"Role is not persist.");
    }
    return *m_ref;
}

Role::operator RoleRef() const
{
    return get_ref();
}

Role::Role(RoleRef ref, std::wstring name, boost::optional<RoleRef> parent_ref):
    m_ref(ref),
    m_name(std::move(name)),
    m_parent_ref(parent_ref)
{
}

}} // namespace TR { namespace Core {