#include "stdafx.h"
#include "BaseTreeModel.h"
#include "Application.h"
#include <limits>

#undef max;
const size_t MAX_ROW_POS = std::numeric_limits<size_t>::max();
const size_t MAX_COL_POS = std::numeric_limits<size_t>::max();

BaseTreeModelRow::BaseTreeModelRow(BaseTreeModel* treeModel):
    m_treeModel(treeModel),
    m_parentRow(nullptr)
{
}

bool BaseTreeModelRow::checkType(const std::type_info& tpi) const
{
    return typeid(BaseTreeModelRow) == tpi;
}

BaseTreeModelRow* BaseTreeModelRow::getParentRow()
{
    return m_parentRow;
}

const BaseTreeModelRow* BaseTreeModelRow::getParentRow() const
{
    return m_parentRow;
}

size_t findRow(const std::vector<std::unique_ptr<BaseTreeModelRow>>& rows, const BaseTreeModelRow* target)
{
    for (size_t position = 0; position < rows.size(); ++position)
    {
        if (rows[position].get() == target)
        {
            return position;
        }
    }
    throw std::logic_error("Invalid BaseTreeModelRow operation");
}

size_t BaseTreeModelRow::getPosition() const
{
    if (m_parentRow)
    {
        return findRow(m_parentRow->m_childRows, this);
    }
    return 0;
}

bool BaseTreeModelRow::isTop() const
{
    _ASSERT(m_treeModel);
    return m_parentRow == &m_treeModel->getHiddenRoot();
}

void BaseTreeModelRow::emitChanged()
{
    _ASSERT(m_treeModel);
    auto position = getPosition();
    auto parentRow = getParentRow();
    auto first = m_treeModel->createIndex(position, 0, parentRow);
    auto last = m_treeModel->createIndex(position, m_treeModel->columnCount() - 1, parentRow);
    m_treeModel->dataChanged().emit(first, last);
}

BaseTreeModelRow& BaseTreeModelRow::addChildRow(std::unique_ptr<BaseTreeModelRow> childRow, size_t position)
{
    _ASSERT(childRow);
    _ASSERT(!childRow->m_treeModel);
    _ASSERT(position == MAX_ROW_POS || position <= m_childRows.size());
    
    auto _where = (position != MAX_ROW_POS) ? m_childRows.begin() + position : m_childRows.end();
    m_childRows.insert(_where, std::move(childRow));
    auto childPosition = (position != MAX_ROW_POS) ? position : getChildCount() - 1;
    auto& addedRow = *m_childRows[childPosition];
    addedRow.m_parentRow = this;

    if (m_treeModel)
    {
        addedRow.setTreeModel(m_treeModel);
        m_treeModel->insertTree(m_treeModel->createIndex(childPosition, 0, this));  
    }

    return *m_childRows[childPosition];
}

size_t BaseTreeModelRow::getChildCount() const
{
    return m_childRows.size();
}

BaseTreeModelRow& BaseTreeModelRow::getChildRow(size_t position)
{
    _ASSERT(position < m_childRows.size());
    return *m_childRows[position];
}

const BaseTreeModelRow& BaseTreeModelRow::getChildRow(size_t position) const
{
    return const_cast<BaseTreeModelRow*>(this)->getChildRow(position);
}

void BaseTreeModelRow::removeChildRow(size_t position)
{
    _ASSERT(m_treeModel);
    _ASSERT(position < m_childRows.size());

    auto thisIndex = m_treeModel->getIndexFromRow(*this);
    m_treeModel->layoutAboutToBeChanged()();
    m_treeModel->beginRemoveRows(thisIndex, position, position);

    auto _where = m_childRows.begin() + position;
    auto removedRow = std::move(*_where);   
    removedRow->m_treeModel = nullptr;
    m_childRows.erase(_where);
    
    m_treeModel->endRemoveRows();
    m_treeModel->layoutChanged()();
}

std::unique_ptr<BaseTreeModelRow> BaseTreeModelRow::releaseChildRow(size_t position)
{
    _ASSERT(m_treeModel);
    _ASSERT(position < m_childRows.size());

    auto thisIndex = m_treeModel->getIndexFromRow(*this);   
    m_treeModel->layoutAboutToBeChanged().emit();
    m_treeModel->beginRemoveRows(thisIndex, position, position);

    std::unique_ptr<BaseTreeModelRow> removed;
    auto _where = m_childRows.begin() + position;
    removed = std::move(*_where);
    removed->m_parentRow = nullptr;
    removed->setTreeModel(nullptr);
    m_childRows.erase(_where);

    m_treeModel->endRemoveRows();
    m_treeModel->layoutChanged().emit();
    return removed;
}

void BaseTreeModelRow::swapChildRows(size_t first, size_t second)
{
    _ASSERT(first < m_childRows.size() && second < m_childRows.size());
    _ASSERT(first != second);

    m_treeModel->layoutAboutToBeChanged().emit();
    std::swap(m_childRows[first], m_childRows[second]);
    m_treeModel->layoutChanged().emit();
}

BaseTreeModel* BaseTreeModelRow::getTreeModel() const
{
    return m_treeModel;
}

void BaseTreeModelRow::setTreeModel(BaseTreeModel* treeModel)
{
    _ASSERT(treeModel == nullptr || m_treeModel == nullptr);
    m_treeModel = treeModel;

    for (auto& child_row : m_childRows | boost::adaptors::indirected)
    {
        child_row.setTreeModel(treeModel);
    }
}

bool BaseTreeModelColumn::checkType(const std::type_info& tpi) const
{
    return typeid(BaseTreeModelColumn) == tpi;
}

BaseTreeModel::BaseTreeModel(std::unique_ptr<BaseTreeModelRow> hiddenRoot):
    m_hiddenRoot(hiddenRoot ? std::move(hiddenRoot) : std::make_unique<BaseTreeModelRow>())
{
    m_hiddenRoot->setTreeModel(this);
}

bool BaseTreeModel::checkType(const std::type_info& tpi) const
{
    return typeid(BaseTreeModel) == tpi;
}

void BaseTreeModel::addColumn(std::shared_ptr<BaseTreeModelColumn> column, size_t position)
{
    _ASSERT(position == MAX_COL_POS || position <= m_columns.size());
    
    if (position == MAX_COL_POS)
    {
        position = m_columns.size();
    }

    m_columns.insert(m_columns.begin() + position, std::move(column));

    // notify connected view
    beginInsertColumns(Wt::WModelIndex(), position, position);
    insertColumn(position);
    endInsertColumns();
}

BaseTreeModelColumn& BaseTreeModel::getColumn(size_t position) const
{
    _ASSERT(position < m_columns.size());
    return *m_columns[position];
}

size_t BaseTreeModel::getColumnCount() const
{
    return m_columns.size();
}

BaseTreeModelRow& BaseTreeModel::addRow(std::unique_ptr<BaseTreeModelRow> row, size_t position)
{
    _ASSERT(row);
    return m_hiddenRoot->addChildRow(std::move(row), position); 
}

size_t BaseTreeModel::getRowCount() const
{
    return m_hiddenRoot->getChildCount();
}

BaseTreeModelRow& BaseTreeModel::getRow(size_t position)
{
    return m_hiddenRoot->getChildRow(position);
}

const BaseTreeModelRow& BaseTreeModel::getRow(size_t position) const
{
    return const_cast<BaseTreeModel*>(this)->getRow(position);
}

void BaseTreeModel::removeRow(size_t position)
{
    m_hiddenRoot->removeChildRow(position);
}

void BaseTreeModel::reset(std::unique_ptr<BaseTreeModelRow> hiddenRoot)
{
    m_hiddenRoot = (hiddenRoot ? std::move(hiddenRoot) : std::make_unique<BaseTreeModelRow>());
    m_hiddenRoot->setTreeModel(this);
    Wt::WAbstractItemModel::reset();
}

BaseTreeModelRow& BaseTreeModel::getHiddenRoot()
{
    return *m_hiddenRoot;
}

Wt::WModelIndex BaseTreeModel::parent(const Wt::WModelIndex& index) const
{
    if (!index.isValid())
    {
        return Wt::WModelIndex();
    }

    auto parentRow = (BaseTreeModelRow*)index.internalPointer();
    return getIndexFromRow(*parentRow); 
}

Wt::WModelIndex BaseTreeModel::index(int row, int column, const Wt::WModelIndex& parent) const
{
    if (!parent.isValid())
    {
        return createIndex(row, column, (void*)m_hiddenRoot.get());
    }

    auto& parentRow = getRowFromIndex(parent);
    return createIndex(row, column, &parentRow);
}

int BaseTreeModel::columnCount(const Wt::WModelIndex& parent) const
{
    return m_columns.size();
}

int BaseTreeModel::rowCount(const Wt::WModelIndex& parent) const
{
    auto& parentRow = getRowFromIndex(parent);
    return parentRow.getChildCount();
}

void BaseTreeModel::sort(int column, Wt::SortOrder order)
{
    _ASSERT(size_t(column) < m_columns.size());
    
    layoutAboutToBeChanged().emit();    
    auto pred = [this, column, order](const BaseTreeModelRow& left, const BaseTreeModelRow& right)
    {
        if (order == Wt::SortOrder::Ascending)
        {
            return m_columns[column]->less(left, right);
        }
        return m_columns[column]->less(right, left);
    };
    sort(m_hiddenRoot->m_childRows, pred);
    layoutChanged().emit();
}

void BaseTreeModel::sort(std::vector<std::unique_ptr<BaseTreeModelRow>>& rows, std::function<bool(const BaseTreeModelRow& left, const BaseTreeModelRow& right)> pred)
{
    boost::sort(rows, [&](const std::unique_ptr<BaseTreeModelRow>& left, const std::unique_ptr<BaseTreeModelRow>& right)
    {
        return pred(*left, *right);
    });

    for (auto& row : rows | boost::adaptors::indirected)
    {
        sort(row.m_childRows, pred);
    }   
}

void* BaseTreeModel::toRawIndex(const Wt::WModelIndex& index) const
{
    return &getRowFromIndex(index);
}

Wt::WModelIndex BaseTreeModel::fromRawIndex(void* rawIndex) const
{
    _ASSERT(rawIndex);
    auto row = (BaseTreeModelRow*)rawIndex;
    if (row->getTreeModel())
    {
        return getIndexFromRow(*row);       
    }

    return Wt::WModelIndex();
}

Wt::cpp17::any BaseTreeModel::data(const Wt::WModelIndex& index, Wt::ItemDataRole role) const
{
    if (!index.isValid())
    {
        return boost::any();
    }

    auto& row = getRowFromIndex(index);

    switch (role.value())
    {
    case Wt::ItemDataRole::Display:
        return stl_tools::ucs_to_ansi(m_columns[index.column()]->getString(row));
    }

    return Wt::cpp17::any();
}

Wt::cpp17::any BaseTreeModel::headerData(int section, Wt::Orientation orientation, Wt::ItemDataRole role) const
{
    if (orientation == Wt::Orientation::Horizontal && role == Wt::ItemDataRole::Display)
    {
        return stl_tools::ucs_to_ansi(m_columns[section]->getName());
    }
    return Wt::cpp17::any();
}

void BaseTreeModel::insertTree(Wt::WModelIndex topIndex, bool insertTop)
{
    auto& topRow = getRowFromIndex(topIndex);

    if (insertTop)
    {
        auto parentIndex = topIndex.parent();
        auto topPosition = topIndex.row();
        beginInsertRows(parentIndex, topPosition, topPosition);
        insertRow(topPosition, parentIndex);
        endInsertRows();
    }
    
    if (auto childCount = topRow.getChildCount())
    {
        beginInsertRows(topIndex, 0, childCount);
        for (size_t childPosition = 0; childPosition < childCount; ++childPosition)
        {
            insertRow(childPosition, topIndex);
        }
        endInsertRows();

        for (size_t childPosition = 0; childPosition < childCount; ++childPosition)
        {
            insertTree(createIndex(childPosition, 0, &topRow), false);
        }
    }
}

BaseTreeModelRow& BaseTreeModel::getRowFromIndex(Wt::WModelIndex index) const
{
    if (!index.isValid())
    {
        return *m_hiddenRoot;
    }

    auto parentRow = (BaseTreeModelRow*)index.internalPointer();
    return parentRow->getChildRow(index.row());
}

Wt::WModelIndex BaseTreeModel::getIndexFromRow(BaseTreeModelRow& row) const
{
    if (&row == m_hiddenRoot.get())
    {
        return Wt::WModelIndex();
    }
    return createIndex(row.getPosition(), 0, row.getParentRow());
}

BaseTreeModelRow& getRowFromIndex(Wt::WModelIndex index)
{
    auto model = dynamic_cast<const BaseTreeModel*>(index.model());
    _ASSERT(model);
    return model->getRowFromIndex(index);
}

Wt::WModelIndex getIndexFromRow(BaseTreeModelRow& row)
{
    auto model = row.getTreeModel();
    _ASSERT(model);
    return model->getIndexFromRow(row);
}