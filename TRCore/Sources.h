#pragma once
#include "RegistryDecl.h"
namespace TR { namespace Core {
class SourceDataset;

class Source
{
public:
    friend class Sources;
    Source(std::wstring name, boost::optional<SourceRef> parent_ref, RoleRef role_ref, SourceTypeRef source_type_ref, std::unique_ptr<XmlPropertiesDoc> settings);

    std::wstring                            get_name() const;
    void                                    set_name(std::wstring name);
    SourceRef                               get_parent() const;
    void                                    set_parent(SourceRef parent_ref);
    bool                                    has_parent() const;
    RoleRef                                 get_role() const;
    void                                    set_role(RoleRef role_ref);
    SourceTypeRef                           get_source_type() const;
    void                                    set_source_type(SourceTypeRef source_type_ref);
    std::shared_ptr<const XmlPropertiesDoc> get_config() const;
    void                                    set_config(std::unique_ptr<XmlPropertiesDoc> config);

    SourceRef                               get_ref() const;
    operator SourceRef() const;

private:
    Source(SourceRef ref, std::wstring name, boost::optional<SourceRef> parent_ref, RoleRef role_ref, SourceTypeRef source_type_ref, std::unique_ptr<XmlPropertiesDoc> settings);

    std::wstring m_name;
    boost::optional<SourceRef> m_parent_ref;
    RoleRef m_role_ref;
    SourceTypeRef m_source_type_ref;
    std::shared_ptr<XmlPropertiesDoc> m_config;
    boost::optional<SourceRef> m_ref;
};

class Sources
{
public:
    static const Priority SOURCES_PRIORITY = 400;
    static void             install(Database& db);

    Sources();
    ~Sources();
    void                    set_database(Database* db);
    void                    set_source_types(SourceTypes* source_types);
    void                    set_roles(Roles* roles);

    SourceRef               add_source(Source source);
    SourceRef               add_source(Source source, Transaction& t);
    void                    update_source(Source source);
    void                    update_source(Source source, Transaction& t);
    std::vector<Source>     get_sources() const;
    std::vector<Source>     get_sources(SourceRef parent_ref) const;
    std::vector<Source>     get_sources(RoleRef role_ref) const;
    Source                  get_source(SourceKey source_key) const;
    Source                  get_root() const;
    bool                    has_source(SourceKey source_key) const;
    void                    remove_source(SourceKey source_key);
    void                    remove_source(SourceKey source_key, Transaction& t);

    template<class T>
    void connect_add_source(T slot, Priority priority) { m_add_source_sig.connect(slot, priority); }

    template<class T>
    void connect_update_source(T slot, Priority priority) { m_update_source_sig.connect(slot, priority); }

    template<class T>
    void connect_remove_source(T slot, Priority priority) { m_remove_source_sig.connect(slot, priority); }

    template<class T>
    void connect_replace_source(T slot, Priority priority) { m_replace_source_sig.connect(slot, priority); }

    template<class T>
    void connect_rerole_source(T slot, Priority priority) { m_rerole_source_sig.connect(slot, priority); }
    
private:    
    std::vector<SourceKey>  get_childs(SourceKey source_key) const;
    std::vector<SourceKey>  get_descendants(SourceKey source_key) const;
    bool                    ancestor_of(SourceKey ancestor_key, SourceKey descendant_key) const;

    void                    on_remove_source_type(UUID source_type_uuid);
    void                    on_remove_role(RoleKey role_key);

    OrderedSignal<void(SourceRef source_ref, Transaction& t)> m_add_source_sig;
    OrderedSignal<void(SourceRef source_ref, Transaction& t)> m_update_source_sig;
    OrderedSignal<void(SourceKey source_key, const std::vector<SourceKey>& removed_sources, Transaction& t)> m_remove_source_sig;
    OrderedSignal<void(SourceRef source_ref, SourceRef prev_parent_ref, SourceRef next_parent_ref, Transaction& t)> m_replace_source_sig;
    OrderedSignal<void(SourceRef source_ref, RoleRef prev_role_ref, RoleRef next_role_ref, Transaction& t)> m_rerole_source_sig;

    Database* m_db;
    SourceTypes* m_source_types;
    Roles* m_roles;

    std::unique_ptr<SourceDataset> m_source_dataset;
};

}} //namespace TR { namespace Core {