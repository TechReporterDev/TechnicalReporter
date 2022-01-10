#include "stdafx.h"
#include "Sources.h"
#include "SourceTypes.h"
#include "Roles.h"
#include "Transaction.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
#include "SourceDataset.h"
namespace TR { namespace Core {

static const auto ROOT_KEY = SourceKey(1);
static const auto DEFAULT_ROLE_KEY = RoleKey(1);

void Sources::install(Database& db)
{
    SourceData root_source_data(
        0,
        "Root",
        boost::none,
        DEFAULT_ROLE_KEY,
        GroupSourceType::s_uuid,
        xml_tools::as_string(*XML::create_regular(*GroupSourceType::s_config_def)));

    Key root_key = db.persist(root_source_data);
    _ASSERT(root_key == ROOT_KEY);
}

Sources::Sources():
    m_db(nullptr),
    m_source_types(nullptr),
    m_roles(nullptr)
{
}

Sources::~Sources()
{
}

void Sources::set_database(Database* db)
{
    m_db = db;  
    m_source_dataset = std::make_unique<SourceDataset>(*db);
}

void Sources::set_source_types(SourceTypes* source_types)
{
    static_assert(SOURCES_PRIORITY > SourceTypes::SOURCE_TYPES_PRIORITY, "Wrong component priority");
    
    _ASSERT(!m_source_types);
    m_source_types = source_types;
    m_source_types->connect_remove_source_type([this](UUID source_type_uuid, Transaction& t){
        on_remove_source_type(source_type_uuid);
    }, SOURCES_PRIORITY);
}

void Sources::set_roles(Roles* roles)
{
    static_assert(SOURCES_PRIORITY > Roles::ROLES_PRIORITY, "Wrong component priority");
    
    _ASSERT(!m_roles);
    m_roles = roles;
    m_roles->connect_remove_role([this](RoleKey role_key, Transaction& t){
        on_remove_role(role_key);
    }, SOURCES_PRIORITY);
}

SourceRef Sources::add_source(Source source)
{
    Transaction t(*m_db);
    auto source_ref = add_source(std::move(source), t);
    t.commit();
    return source_ref;
}

SourceRef Sources::add_source(Source source, Transaction& t)
{
    SourceData source_data(
        0,
        stl_tools::ucs_to_utf8(source.m_name),
        pack(source.m_parent_ref),
        source.m_role_ref.get_key(),
        source.get_source_type().get_uuid(),
        xml_tools::as_string(*source.m_config));
    
    SourceRef source_ref(this, m_source_dataset->persist(source_data, t));
    m_add_source_sig(source_ref, t);
    return source_ref;
}

void Sources::update_source(Source source)
{
    Transaction t(*m_db);
    update_source(std::move(source), t);
    t.commit();
}

void Sources::update_source(Source source, Transaction& t)
{
    if (!source.m_ref)
    {
        throw Exception(L"Can`t update not persistent source");
    }
        
    if (source.m_parent_ref)
    {
        if (ancestor_of(source.m_ref->get_key(), source.m_parent_ref->get_key()))
        {
            throw Exception(L"Cyclic source dependency found");
        }
    }

    SourceData next_source_data(
        source.m_ref->get_key(),
        pack(source.m_name),
        pack(source.m_parent_ref),
        source.m_role_ref.get_key(),
        source.get_source_type().get_uuid(),
        xml_tools::as_string(*source.m_config));    
    
    auto prev_source = get_source(source.m_ref->get_key());
    m_source_dataset->update(next_source_data, t);
    m_update_source_sig(source, t);

    if (prev_source.m_parent_ref != source.m_parent_ref)
    {
        m_replace_source_sig(source, prev_source.get_parent(), source.get_parent(), t);
    }

    if (prev_source.get_role() != source.get_role())
    {
        m_rerole_source_sig(source, prev_source.get_role(), source.get_role(), t);
    }
}

std::vector<Source> Sources::get_sources() const
{
    ReadOnlyTransaction t(*m_db);
    std::vector<Source> sources;
    for (auto& source_data : m_source_dataset->query())
    {
        auto& source_type = m_source_types->get_source_type(source_data.m_source_type_uuid);
        sources.push_back({
            SourceRef(this, SourceKey(source_data.m_key)),
            unpack(source_data.m_name),
            unpack(source_data.m_parent_key, this),
            RoleRef(m_roles, RoleKey(source_data.m_role_key)),
            source_type,
            parse_properties_doc(source_data.m_config, *source_type.get_config_def())
        });
    }
    return sources;
}

std::vector<Source> Sources::get_sources(SourceRef parent_ref) const
{
    ReadOnlyTransaction t(*m_db);
    std::vector<Source> sources;
    for (auto& source_data : m_source_dataset->query_parent_equal(parent_ref.get_key()))
    {
        auto& source_type = m_source_types->get_source_type(source_data.m_source_type_uuid);
        sources.push_back({
            SourceRef(this, SourceKey(source_data.m_key)),
            unpack(source_data.m_name),
            unpack(source_data.m_parent_key, this),
            RoleRef(m_roles, RoleKey(source_data.m_role_key)),
            unpack(SourceTypeUUID(source_data.m_source_type_uuid), m_source_types),
            parse_properties_doc(source_data.m_config, *source_type.get_config_def())
        });
    }
    return sources;
}

std::vector<Source> Sources::get_sources(RoleRef role_ref) const
{
    ReadOnlyTransaction t(*m_db);
    std::vector<Source> sources;
    for (auto& source_data : m_source_dataset->query_role_equal(role_ref.get_key()))
    {
        auto& source_type = m_source_types->get_source_type(source_data.m_source_type_uuid);
        sources.push_back({
            SourceRef(this, SourceKey(source_data.m_key)),
            unpack(source_data.m_name),
            unpack(source_data.m_parent_key, this),
            RoleRef(m_roles, RoleKey(source_data.m_role_key)),
            unpack(SourceTypeUUID(source_data.m_source_type_uuid), m_source_types),
            parse_properties_doc(source_data.m_config, *source_type.get_config_def())
        });
    }
    return sources;
}

Source Sources::get_source(SourceKey source_key) const
{
    ReadOnlyTransaction t(*m_db);
    auto source_data = m_source_dataset->load(source_key);
    auto& source_type = m_source_types->get_source_type(source_data.m_source_type_uuid);    
    
    return Source(
        SourceRef(this, source_key),
        unpack(source_data.m_name),
        unpack(source_data.m_parent_key, this),
        RoleRef(m_roles, RoleKey(source_data.m_role_key)),
        source_type,
        parse_properties_doc(source_data.m_config, *source_type.get_config_def()));
}

Source Sources::get_root() const
{
    ReadOnlyTransaction t(*m_db);
    return get_source(ROOT_KEY);
}

bool Sources::has_source(SourceKey source_key) const
{
    ReadOnlyTransaction t(*m_db);
    return m_source_dataset->find(source_key).is_initialized();
}

void Sources::remove_source(SourceKey source_key)
{
    Transaction t(*m_db);
    remove_source(source_key, t);
    t.commit();
}

void Sources::remove_source(SourceKey source_key, Transaction& t)
{
    if (source_key == ROOT_KEY)
    {
        throw Exception(L"Can`t remove root group");
    }

    m_db->erase<SourceData>(source_key);

    std::vector<SourceKey> removed;
    removed.push_back(source_key);
    boost::copy(get_descendants(source_key), std::back_inserter(removed));
    m_source_dataset->erase(removed, t);
    m_remove_source_sig(source_key, std::move(removed), t);
}

std::vector<SourceKey> Sources::get_childs(SourceKey source_key) const
{
    ReadOnlyTransaction t(*m_db);
    std::vector<SourceKey> childs;
    for (auto& source_data : m_source_dataset->query_parent_equal(source_key))
    {
        childs.push_back(SourceKey(source_data.m_key));
    }
    return childs;
}

std::vector<SourceKey> Sources::get_descendants(SourceKey source_key) const
{
    ReadOnlyTransaction t(*m_db);
    std::vector<SourceKey> descendants;
    for (auto& child_key : get_childs(source_key))
    {
        descendants.push_back(child_key);
        boost::copy(get_descendants(child_key), std::back_inserter(descendants));
    }
    return descendants;
}

bool Sources::ancestor_of(SourceKey ancestor_key, SourceKey descendant_key) const
{
    ReadOnlyTransaction t(*m_db);
    if (ancestor_key == descendant_key)
    {
        return true;
    }

    SourceData descendant_data;
    m_db->load<SourceData>(descendant_key, descendant_data);
    if (!descendant_data.m_parent_key)
    {
        return false;
    }

    return ancestor_of(ancestor_key, *descendant_data.m_parent_key);
}

void Sources::on_remove_source_type(UUID source_type_uuid)
{
    if (!m_source_dataset->query_source_type_equal(SourceTypeUUID(source_type_uuid)).empty())
    {
        throw Exception(L"Can`t remove 'source_type' until its sources exists");
    }
}

void Sources::on_remove_role(RoleKey role_key)
{
    if (!m_db->query<SourceData>(odb::query<SourceData>::role_key == role_key).empty())
    {
        throw Exception(L"Can`t remove 'role' until its sources exists");
    }
}

Source::Source(std::wstring name, boost::optional<SourceRef> parent_ref, RoleRef role_ref, SourceTypeRef source_type_ref, std::unique_ptr<XmlPropertiesDoc> config):
    m_name(std::move(name)),
    m_parent_ref(parent_ref),
    m_role_ref(role_ref),
    m_source_type_ref(source_type_ref),
    m_config(std::move(config))
{
}

std::wstring Source::get_name() const
{
    return m_name;
}

void Source::set_name(std::wstring name)
{
    m_name = std::move(name);
}

SourceRef Source::get_parent() const
{
    if (!m_parent_ref)
    {
        throw Exception(L"Source has no parent");
    }
    return *m_parent_ref;
}

void Source::set_parent(SourceRef parent_ref)
{
    m_parent_ref = parent_ref;
}

bool Source::has_parent() const
{
    return m_parent_ref.is_initialized();
}

RoleRef Source::get_role() const
{
    return m_role_ref;
}

void Source::set_role(RoleRef role_ref)
{
    m_role_ref = role_ref;
}

SourceTypeRef Source::get_source_type() const
{
    return m_source_type_ref;
}

void Source::set_source_type(SourceTypeRef source_type_ref)
{
    m_source_type_ref = source_type_ref;
}

std::shared_ptr<const XmlPropertiesDoc> Source::get_config() const
{
    return m_config;
}

void Source::set_config(std::unique_ptr<XmlPropertiesDoc> config)
{
    m_config = std::move(config);
}

SourceRef Source::get_ref() const
{
    _ASSERT(m_ref);
    if (!m_ref)
    {
        throw Exception(L"Source is not persist.");
    }
    return *m_ref;
}

Source::operator SourceRef() const
{
    return get_ref();
}

Source::Source(SourceRef ref, std::wstring name, boost::optional<SourceRef> parent_ref, RoleRef role_ref, SourceTypeRef source_type_ref, std::unique_ptr<XmlPropertiesDoc> config):
    m_ref(ref),
    m_name(std::move(name)),
    m_parent_ref(parent_ref),
    m_role_ref(role_ref),
    m_source_type_ref(source_type_ref),
    m_config(std::move(config))
{
}

}} // namespace TR { namespace Core {