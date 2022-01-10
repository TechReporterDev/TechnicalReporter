#include "stdafx.h"
#include "Links.h"
#include "Sources.h"
#include "Transaction.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
#include "LinkDataset.h"

namespace TR { namespace Core {

void Links::install(Database& db)
{
}

Links::Links():
    m_db(nullptr),
    m_sources(nullptr)
{
}

Links::~Links()
{
}

void Links::set_database(Database* db)
{
    _ASSERT(!m_db);
    m_db = db;
    m_link_dataset = std::make_unique<LinkDataset>(*m_db);
}

void Links::set_sources(Sources* sources)
{
    _ASSERT(!m_sources);
    m_sources = sources;
    m_sources->connect_remove_source([this](SourceKey source_key, const std::vector<SourceKey>& removed_sources, Transaction& t){
        on_remove_sources(removed_sources, t);
    }, LINKS_PRIORITY);
}

void Links::add_link(Link link, Transaction& t)
{
    LinkData link_data(
        pack(link.m_parent_ref),
        pack(link.m_child_ref)
    );

    m_link_dataset->persist(link_data, t);  
    m_add_link_sig(link, t);
}

std::vector<Link> Links::get_links() const
{
    ReadOnlyTransaction t(*m_db);
    std::vector<Link> links;
    for (auto& link_data : m_link_dataset->query())
    {
        links.push_back({
            unpack(link_data.m_parent_key, m_sources),
            unpack(link_data.m_child_key, m_sources),
        });
    }
    return links;
}

std::vector<Link> Links::find_by_parent(SourceRef parent_ref)
{
    ReadOnlyTransaction t(*m_db);
    std::vector<Link> links;
    for (auto& link_data : m_link_dataset->query_parent_equal(parent_ref.get_key()))
    {
        links.push_back({
            unpack(link_data.m_parent_key, m_sources),
            unpack(link_data.m_child_key, m_sources),
        });
    }
    return links;
}

std::vector<Link> Links::find_by_child(SourceRef child_ref)
{
    ReadOnlyTransaction t(*m_db);
    std::vector<Link> links;
    for (auto& link_data : m_link_dataset->query_child_equal(child_ref.get_key()))
    {
        links.push_back({
            unpack(link_data.m_parent_key, m_sources),
            unpack(link_data.m_child_key, m_sources),
        });
    }
    return links;
}

void Links::remove_link(Link link, Transaction& t)
{
    m_link_dataset->erase({pack(link.m_parent_ref), pack(link.m_child_ref)}, t);
    m_remove_link_sig(link, t);
}

void Links::on_remove_sources(const std::vector<SourceKey>& removed_sources, Transaction& t)
{
    for (auto removed_source : removed_sources)
    {
        if (!m_link_dataset->query_child_equal(removed_source).empty())
        {
            throw Exception(L"Can`t remove 'source' until link exists");
        }
    }   

    m_link_dataset->erase_parent_equal(removed_sources, t);
}

}} // namespace TR { namespace Core {