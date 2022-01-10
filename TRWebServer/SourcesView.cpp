#include "stdafx.h"
#include "SourcesView.h"
#include "Application.h"

SourcesView::SourceItem::ID SourcesView::SourceItem::getID() const
{
    return m_sourceInfo.m_key;
}

boost::optional<SourcesView::SourceItem::ID> SourcesView::SourceItem::getParentID() const
{
    if (m_sourceInfo.m_key == 1)
    {
        return boost::none;
    }

    return m_sourceInfo.m_parent_key;
}

SourcesView::SuitRootItem::ID SourcesView::SuitRootItem::getID() const
{
    return m_sourceInfo.m_key;
}

boost::none_t SourcesView::SuitRootItem::getParentID() const
{
    return boost::none;
}

SourcesView::SuitItem::ID SourcesView::SuitItem::getID() const
{
    return m_sourceInfo.m_key;
}

SourcesView::SuitRootItem::ID SourcesView::SuitItem::getParentID() const
{
    return m_sourceInfo.m_parent_key;
}

SourcesView::LinkItem::ID SourcesView::LinkItem::getID() const
{
    return std::make_pair(m_link.m_parent_key, m_link.m_child_key);
}

SourcesView::SuitItem::ID SourcesView::LinkItem::getParentID() const
{
    return m_link.m_parent_key;
}

SourcesView::SourcesView(OnDblClick onDblClick):
    m_onDblClick(onDblClick)
{
}

namespace {
struct GetID: boost::static_visitor<SourcesView::ItemID>
{
    template<class ItemType>
    SourcesView::ItemID operator()(const ItemType& item) const
    {
        return item.getID();
    }
};
} //namespace {

SourcesView::ItemID SourcesView::getID(Item item)
{
    return boost::apply_visitor(GetID(), item);
}

namespace {
struct GetParentID: boost::static_visitor<boost::optional<SourcesView::ItemID>>
{
    template<class ItemType>
    boost::optional<SourcesView::ItemID> operator()(const ItemType& item) const
    {
        return boost::optional<SourcesView::ItemID>(item.getParentID());
    }
};
} //namespace {

boost::optional<SourcesView::ItemID> SourcesView::getParentID(Item item)
{
    return boost::apply_visitor(GetParentID(), item);
}