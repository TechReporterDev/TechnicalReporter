#include "stdafx.h"
#include "SourcesTableView.h"
#include "SourcePage.h"
#include "Application.h"

SourceNameColumn::SourceNameColumn(std::map<TR::SourceKey, TR::SourceInfo>& sourceByKey):
    m_sourceByKey(sourceByKey)
{
}

std::wstring SourceNameColumn::getName() const
{
    return L"Name";
}

struct GetSourceName: boost::static_visitor<std::wstring>
{
    GetSourceName(std::map<TR::SourceKey, TR::SourceInfo>& sourceByKey):
        m_sourceByKey(sourceByKey)
    {
    }

    template<class ItemType>
    std::wstring operator()(const ItemType& sourceItem) const
    {
        return sourceItem.m_sourceInfo.m_name;
    }

    std::wstring operator()(const SourcesView::SuitRootItem& suitRootItem) const
    {
        return L"Suits";
    }

    std::wstring operator()(const SourcesView::LinkItem& linkItem) const
    {
        return m_sourceByKey[linkItem.m_link.m_child_key].m_name;
    }

    std::map<TR::SourceKey, TR::SourceInfo>& m_sourceByKey;
};

std::wstring SourceNameColumn::getString(const SourceRow& row) const
{
    return boost::apply_visitor(GetSourceName(m_sourceByKey), row.getData());
}

SourceParentColumn::SourceParentColumn(std::map<TR::SourceKey, TR::SourceInfo>& sourceByKey):
    m_sourceByKey(sourceByKey)
{
}

std::wstring SourceParentColumn::getName() const
{
    return L"Parent";
}

struct GetSourceParent: boost::static_visitor<std::wstring>
{
    GetSourceParent(std::map<TR::SourceKey, TR::SourceInfo>& sourceByKey):
        m_sourceByKey(sourceByKey)
    {
    }

    template<class ItemType>
    std::wstring operator()(const ItemType& sourceItem) const
    {
        if (sourceItem.m_sourceInfo.m_parent_key)
        {
            return m_sourceByKey[sourceItem.m_sourceInfo.m_parent_key].m_name;
        }
        return L"";
    }
    
    std::wstring operator()(const SourcesView::LinkItem& linkItem) const
    {
        return m_sourceByKey[linkItem.m_link.m_parent_key].m_name;
    }

    std::map<TR::SourceKey, TR::SourceInfo>& m_sourceByKey;
};

std::wstring SourceParentColumn::getString(const SourceRow& row) const
{
    return boost::apply_visitor(GetSourceParent(m_sourceByKey), row.getData());
}

SourceTypeColumn::SourceTypeColumn(std::map<TR::UUID, TR::SourceTypeInfo>& sourceTypeByUUID):
    m_sourceTypeByUUID(sourceTypeByUUID)
{
}

std::wstring SourceTypeColumn::getName() const
{
    return L"SourceType";
}

struct GetSourceType: boost::static_visitor<std::wstring>
{
    GetSourceType(std::map<TR::UUID, TR::SourceTypeInfo>& sourceTypeByUUID):
        m_sourceTypeByUUID(sourceTypeByUUID)
    {
    }

    template<class ItemType>
    std::wstring operator()(const ItemType& sourceItem) const
    {
        return m_sourceTypeByUUID[sourceItem.m_sourceInfo.m_source_type_uuid].m_name;
    }
    
    std::wstring operator()(const SourcesView::LinkItem& linkItem) const
    {
        return L"Link";
    }

    std::map<TR::UUID, TR::SourceTypeInfo>& m_sourceTypeByUUID;
};
    
std::wstring SourceTypeColumn::getString(const SourceRow& row) const
{
    return boost::apply_visitor(GetSourceType(m_sourceTypeByUUID), row.getData());
}

SourcesTableView::SourcesTableView(const std::vector<SourcesView::Item>& items, OnDblClick onDblClick):
    SourcesView(onDblClick)
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    m_tableView = vbox->addWidget(std::make_unique<StandardTable<Item>>());
    m_tableView->setSelectionMode(Wt::SelectionMode::Single);
    
    m_tableView->doubleClicked().connect(std::bind([this](Wt::WModelIndex index){
        if (!index.isValid())
        {
            return;
        }

        m_onDblClick(m_tableView->getData(index.row()));
    }, std::placeholders::_1));

    for (auto& item : items)
    {
        m_tableView->addRow(item);
    }
}

void SourcesTableView::addColumn(std::unique_ptr<SourceColumn> column)
{
    m_tableView->addColumn(std::move(column));
}

void SourcesTableView::addItem(Item item)
{
    m_tableView->addRow(item);
}

void SourcesTableView::updateItem(Item item)
{
    auto position = findItemRow(getID(item));
    if (position >= 0)
    {
        m_tableView->updateRow(position, item);
    }
}

void SourcesTableView::removeItem(ItemID itemID)
{
    auto position = findItemRow(itemID);
    if (position >= 0)
    {
        m_tableView->removeRow(position);
    }

    std::vector<ItemID> childItems;

    for (size_t position = 0; position < m_tableView->getRowCount(); ++position)
    {
        auto& item = m_tableView->getData(position);
        if (getParentID(item) == itemID)
        {
            childItems.push_back(getID(item));
        }
    }

    for (auto& childItem : childItems)
    {
        removeItem(childItem);
    }
}

std::vector<SourcesView::Item> SourcesTableView::getSelection() const
{
    std::vector<SourcesView::Item> selection;
    for (auto position : m_tableView->getSelectedRows())
    {
        selection.push_back(m_tableView->getData(position));
    }
    return selection;
}

void SourcesTableView::clearSelection()
{
    m_tableView->clearSelection();
}

void SourcesTableView::selectItem(ItemID itemID)
{
    auto position = findItemRow(itemID);
    if (position >= 0)
    {
        m_tableView->selectRow(position);
    }   
}

size_t SourcesTableView::findItemRow(ItemID itemID)
{
    for (size_t position = 0; position < m_tableView->getRowCount(); ++position)
    {
        if (getID(m_tableView->getData(position)) == itemID)
        {
            return position;
        }
    }
    return -1;
}