#pragma once
#include "BackgndFwd.h"
#include "TrivialTypes.h"
#include "stddecl.h"
namespace TR { namespace Core {
struct CoreImpl;
class FeatureStorage;

class Feature
{
public:
    friend class Features;
    
    Feature(UUID uuid, std::wstring name);
    virtual ~Feature(){}

    UUID                    get_uuid() const;
    std::wstring            get_name() const;
    std::vector<UUID>       get_dependencies() const;

protected:
    virtual void    install(CoreImpl& core_impl, bool restore, Transaction& t) = 0;
    virtual void    uninstall(CoreImpl& core_impl, Transaction& t) = 0;

    UUID m_uuid;
    std::vector<UUID> m_dependencies;
    std::wstring m_name;
};

class Features
{
public:
    Features();
    ~Features();

    void                            restore_installed(CoreImpl* core_impl, Transaction& t);
    void                            run(Executive* executive, Transaction& t);

    void                            add_feature(std::unique_ptr<Feature> feature);
    void                            remove_feature(UUID feature_uuid);
    std::vector<const Feature*>     get_features() const;
    const Feature&                  get_feature(UUID uuid) const;
    const Feature*                  find_feature(UUID feature_uuid) const;

    void                            install_feature(UUID feature_uuid);
    void                            uninstall_feature(UUID feature_uuid);
    bool                            is_installed(UUID uuid) const;      

private:    
    CoreImpl* m_core_impl;
    Database* m_db;
    
    std::unique_ptr<FeatureStorage> m_storage;
    bool m_running;
};

}} //namespace TR { namespace Core {