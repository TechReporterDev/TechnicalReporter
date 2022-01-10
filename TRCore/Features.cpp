#include "stdafx.h"
#include "Features.h"
#include "CoreImpl.h"
#include "Transaction.h"
#include "FeatureData-odb.hxx"
namespace TR { namespace Core {

Feature::Feature(UUID uuid, std::wstring name):
    m_uuid(uuid),
    m_name(std::move(name))
{
}

UUID Feature::get_uuid() const
{
    return m_uuid;
}

std::wstring Feature::get_name() const
{
    return m_name;
}

std::vector<UUID> Feature::get_dependencies() const
{
    return m_dependencies;
}

class FeatureStorage: public std::vector<std::unique_ptr<Feature>>
{
public:
    using Base = std::vector<std::unique_ptr<Feature>>;
    using Iterator = Base::iterator;
    using ConstIterator = Base::const_iterator;

    const Feature& get_feature(UUID feature_uuid) const
    {
        return const_cast<FeatureStorage&>(*this).get_feature(feature_uuid);
    }

    Feature& get_feature(UUID feature_uuid)
    {
        auto found = find_feature(feature_uuid);
        if (found == end())
        {
            throw Exception(L"Feature not found");
        }
        return **found;
    }

    ConstIterator find_feature(UUID feature_uuid) const
    {
        return const_cast<FeatureStorage&>(*this).find_feature(feature_uuid);
    }

    Iterator find_feature(UUID feature_uuid)
    {
        return std::find_if(begin(), end(), [feature_uuid](std::unique_ptr<Feature>& feature){
            return feature->get_uuid() == feature_uuid;
        });
    }
};

Features::Features():
    m_core_impl(nullptr),
    m_db(nullptr),
    m_storage(std::make_unique<FeatureStorage>()),
    m_running(false)
{
}

Features::~Features()
{
}

void Features::restore_installed(CoreImpl* core_impl, Transaction& t)
{
    _ASSERT(!m_core_impl);

    transact_assign(m_core_impl, core_impl, t);
    transact_assign(m_db, core_impl->m_db.get(), t);

    for (auto& feature_info : m_db->query<FeatureData>())
    {
        auto& feature = m_storage->get_feature(feature_info.m_uuid);
        feature.install(*m_core_impl, true, t);
    }
}

void Features::run(Executive* executive, Transaction& t)
{   
    _ASSERT(!m_running);

    for (auto& feature : *m_storage | boost::adaptors::indirected)
    {
        //feature.run(*m_core_impl, t);
    }
    transact_assign(m_running, true, t);
}

void Features::add_feature(std::unique_ptr<Feature> feature)
{
    auto found = m_storage->find_feature(feature->get_uuid());
    if (found != m_storage->end())
    {
        throw Exception(L"Feature already added");
    }
    m_storage->push_back(std::move(feature));
}

void Features::remove_feature(UUID uuid)
{
    auto found = m_storage->find_feature(uuid);
    if (found == m_storage->end())
    {
        throw Exception(L"Feature not found");
    }
    
    if (m_running)
    {
        if (is_installed(uuid))
        {
            throw Exception(L"Can`t remove installed feature.");
        }
    }

    m_storage->erase(found);        
}

std::vector<const Feature*> Features::get_features() const
{
    std::vector<const Feature*> features;
    for (auto& feature : *m_storage | boost::adaptors::indirected)
    {
        features.push_back(&feature);
    }
    return features;
}

const Feature& Features::get_feature(UUID uuid) const
{
    return m_storage->get_feature(uuid);
}

const Feature* Features::find_feature(UUID feature_uuid) const
{
    auto found = m_storage->find_feature(feature_uuid);
    if (found == m_storage->end())
    {
        return nullptr;
    }
    return found->get();
}

void Features::install_feature(UUID feature_uuid)
{
    _ASSERT(m_running); //call "run" before
        
    auto& feature = m_storage->get_feature(feature_uuid);

    Transaction t(*m_db);   
    if (m_db->find<FeatureData>(feature_uuid) != nullptr)
    {
        throw Exception(L"Feature is already installed");
    }

    m_db->persist(FeatureData{feature.get_uuid(), true});
    feature.install(*m_core_impl, false, t);
    t.commit();
}

void Features::uninstall_feature(UUID feature_uuid)
{
    _ASSERT(m_running); //call "run" before

    auto& feature = m_storage->get_feature(feature_uuid);

    Transaction t(*m_db);
    if (m_db->find<FeatureData>(feature_uuid) == nullptr)
    {
        throw Exception(L"Feature is NOT installed");
    }

    m_db->erase<FeatureData>(feature.get_uuid());
    feature.uninstall(*m_core_impl, t);
    t.commit();
}

bool Features::is_installed(UUID uuid) const
{
    _ASSERT(m_running); //call "run" before 
    
    ReadOnlyTransaction t(*m_db);
    return m_db->find<FeatureData>(uuid) != nullptr;
}

}} //namespace TR { namespace Core {