#pragma once
#include "RegistryDecl.h"
namespace TR { namespace Core {
class LinkDataset;

struct Link
{
    Link(SourceRef parent_ref, SourceRef child_ref):
        m_parent_ref(parent_ref),
        m_child_ref(child_ref)
    {
    }

    SourceRef m_parent_ref;
    SourceRef m_child_ref;
};

class Links
{
public:
    static const Priority LINKS_PRIORITY = 450;
    static void             install(Database& db);

    Links();
    ~Links();
    void                    set_database(Database* db);
    void                    set_sources(Sources* sources);

    void                    add_link(Link link, Transaction& t);
    std::vector<Link>       get_links() const;
    std::vector<Link>       find_by_parent(SourceRef parent_ref);
    std::vector<Link>       find_by_child(SourceRef child_ref);
    void                    remove_link(Link link, Transaction& t);
    
    template<class T>
    void connect_add_link(T slot, Priority priority) { m_add_link_sig.connect(slot, priority); }

    template<class T>
    void connect_remove_link(T slot, Priority priority) { m_remove_link_sig.connect(slot, priority); }

private:
    void on_remove_sources(const std::vector<SourceKey>& removed_sources, Transaction& t);

    Database* m_db;
    Sources* m_sources;
    std::unique_ptr<LinkDataset> m_link_dataset;

    OrderedSignal<void(Link Link, Transaction& t)> m_add_link_sig;
    OrderedSignal<void(Link Link, Transaction& t)> m_remove_link_sig;
};

}} //namespace TR { namespace Core {