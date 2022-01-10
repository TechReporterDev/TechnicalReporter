#include "stdafx.h"
#include "SourcesTreeView.h"
#include "SourcePage.h"
#include "Application.h"

class SourceTreeModelBuilder
{
public:
    SourceTreeModelBuilder(SourcesTreeModel& model):
        m_model(model)
    {
    }

    void build(const std::vector<SourcesView::Item>& items)
    {
        for (auto& item : items)
        {
            m_itemByParentID.insert(std::make_pair(SourcesView::getParentID(item), &item));
        }

        for (auto& childItem : m_itemByParentID.equal_range(boost::none) | boost::adaptors::map_values)
        {
            auto& topRow = m_model.addRow(std::make_unique<SourcesTreeRow>(*childItem));
            build(topRow);
        }           
    }   

private:
    void build(StandardModelRow<SourcesTreeView::Item>& parentRow)
    {
        auto parentID = SourcesTreeView::getID(parentRow.getData());
        for (auto& childItem : m_itemByParentID.equal_range(parentID) | boost::adaptors::map_values)
        {
            auto& childRow = parentRow.addChildRow(std::make_unique<SourcesTreeRow>(*childItem));
            build(childRow);
        }
    }

    StandardModel<SourcesTreeView::Item>& m_model;
    std::multimap<boost::optional<SourcesView::ItemID>, const SourcesView::Item*> m_itemByParentID;
};

SourcesTreeView::SourcesTreeView(const std::vector<SourcesView::Item>& items, OnDblClick onDblClick):
    SourcesView(onDblClick),
    m_model(std::make_shared<SourcesTreeModel>()),
    m_treeView(nullptr)
{
    SourceTreeModelBuilder(*m_model).build(items);
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);

    m_treeView = vbox->addWidget(std::make_unique<Wt::WTreeView>());
    m_treeView->setModel(m_model);
    m_treeView->setSelectionMode(Wt::SelectionMode::Single);
    m_treeView->doubleClicked().connect(std::bind([this](Wt::WModelIndex index){
        if (!index.isValid())
        {
            return;
        }

        auto treeRow = (SourcesTreeRow*)m_model->toRawIndex(index);
        m_onDblClick(treeRow->getData());
    }, std::placeholders::_1));
}

SourcesTreeView::~SourcesTreeView()
{
}

void SourcesTreeView::addColumn(std::shared_ptr<SourceColumn> column)
{
    m_model->addColumn(column);
    m_treeView->setItemDelegateForColumn(m_model->columnCount() - 1, column);
}

void SourcesTreeView::expand(int depth)
{
    m_treeView->expandToDepth(depth);
}

void SourcesTreeView::addItem(Item item)
{
    if (auto parentID = getParentID(item))
    {
        if (auto parentRow = findItemRow(*parentID))
        {
            parentRow->addChildRow(std::make_unique<SourcesTreeRow>(item));
        }
    }
}

void SourcesTreeView::updateItem(Item item)
{
    if (auto itemRow = findItemRow(getID(item)))
    {
        if (getParentID(itemRow->getData()) != getParentID(item))
        {
            if (auto nextParent = findItemRow(*getParentID(item)))
            {
                nextParent->addChildRow(itemRow->getParentRow()->releaseChildRow(itemRow->getPosition()));
            }
            else
            {
                itemRow->getParentRow()->removeChildRow(itemRow->getPosition());
                return;
            }
        }
        itemRow->setData(item);
    }
}

void SourcesTreeView::removeItem(ItemID itemID)
{
    clearSelection();
    if (auto itemRow = findItemRow(itemID))
    {
        auto parentRow = itemRow->getParentRow();
        _ASSERT(parentRow);
        parentRow->removeChildRow(itemRow->getPosition());
    }
}

std::vector<SourcesTreeView::Item> SourcesTreeView::getSelection() const
{
    std::vector<SourcesTreeView::Item> selection;
    for (auto index : m_treeView->selectedIndexes())
    {
        auto row = (SourcesTreeRow*)m_model->toRawIndex(index);
        selection.push_back(row->getData());
    }
    return selection;
}

void SourcesTreeView::clearSelection()
{
    m_treeView->setSelectedIndexes(Wt::WModelIndexSet());
}
void SourcesTreeView::selectItem(ItemID itemID)
{
    if (auto itemRow = findItemRow(itemID))
    {
        auto index = m_model->fromRawIndex(itemRow);
        auto parent = m_model->parent(index);
        if (parent.isValid() && !m_treeView->isExpanded(parent))
        {
            m_treeView->expand(parent);
        }
        m_treeView->select(index);
    }   
}

void SourcesTreeView::load()
{
    auto selectedIndexes = m_treeView->selectedIndexes();
    if (!selectedIndexes.empty())
    {
        m_treeView->scrollTo(*selectedIndexes.begin());
    }
}

SourcesTreeRow* SourcesTreeView::findItemRow(ItemID itemID, SourcesTreeRow* _where)
{
    if (!_where)
    {
        for (size_t position = 0; position < m_model->getRowCount(); ++position)
        {
            if (auto found = findItemRow(itemID, &m_model->getRow(position)))
            {
                return found;
            }
        }
        return nullptr;
    }

    if (getID(_where->getData()) == itemID)
    {
        return _where;
    }

    for (size_t position = 0; position < _where->getChildCount(); ++position)
    {
        if (auto found = findItemRow(itemID, &_where->getChildRow(position)))
        {
            return found;
        }
    }

    return nullptr;
}