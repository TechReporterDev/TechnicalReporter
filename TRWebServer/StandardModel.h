#pragma once
#include "BaseTreeModel.h"
#include "TRWebServer.h"

template <class T>
class StandardModel;

template <class T>
class StandardModelRow: public BaseTreeModelRow
{
public:
    StandardModelRow(T data, StandardModel<T>* standardModel = nullptr):
        BaseTreeModelRow(standardModel),
        m_data(std::move(data))
    {
    }

    StandardModelRow(const StandardModelRow&) = delete;
    StandardModelRow& operator = (const StandardModelRow&) = delete;

    StandardModelRow* getParentRow()
    {
        return dynamic_cast<StandardModelRow*>(BaseTreeModelRow::getParentRow());
    }

    size_t getPosition() const
    {
        return BaseTreeModelRow::getPosition();
    }

    StandardModelRow& addChildRow(std::unique_ptr<StandardModelRow> childRow, size_t position = MAX_ROW_POS)
    {
        return static_cast<StandardModelRow&>(BaseTreeModelRow::addChildRow(std::move(childRow), position));
    }

    size_t getChildCount() const
    {
        return BaseTreeModelRow::getChildCount();
    }

    StandardModelRow& getChildRow(size_t position)
    {
        return static_cast<StandardModelRow&>(BaseTreeModelRow::getChildRow(position));
    }

    const StandardModelRow& getChildRow(size_t position) const
    {
        return static_cast<const StandardModelRow&>(BaseTreeModelRow::getChildRow(position));
    }

    void removeChildRow(size_t position)
    {
        return BaseTreeModelRow::removeChildRow(position);
    }

    std::unique_ptr<StandardModelRow> releaseChildRow(size_t position)
    {
        return stl_tools::static_pointer_cast<StandardModelRow>(BaseTreeModelRow::releaseChildRow(position));
    }

    StandardModel<T>* getStandardModel() const;

    void setData(T data)
    {
        m_data = std::move(data);
        emitChanged();
    }

    const T& getData() const
    {
        return m_data;
    }

protected:
    T m_data;   
};

template<class T>
class StandardModelColumn: public BaseTreeModelColumn
{
public:
    virtual std::wstring getString(const BaseTreeModelRow& row) const
    {
        return getString(static_cast<const StandardModelRow<T>&>(row));
    }

    virtual std::wstring getString(const StandardModelRow<T>& row) const = 0;

    virtual bool less(const BaseTreeModelRow& left, const BaseTreeModelRow& right) const
    {
        return less(static_cast<const StandardModelRow<T>&>(left), static_cast<const StandardModelRow<T>&>(right));
    }

    virtual bool less(const StandardModelRow<T>& left, const StandardModelRow<T>& right) const
    {
        return getString(left) < getString(right);
    }
};

template <class T>
class StandardModel: public BaseTreeModel
{
public:
    StandardModel():
        BaseTreeModel(nullptr)
    {
    }

    StandardModel(const BaseTreeModel&) = delete;
    StandardModel& operator = (const BaseTreeModel&) = delete;

    void addColumn(std::shared_ptr<StandardModelColumn<T>> column)
    {
        BaseTreeModel::addColumn(column);
        Wt::WAbstractItemModel::reset();
    }

    StandardModelColumn<T>& getColumn(size_t position) const
    {
        return static_cast<StandardModelColumn<T>&>(BaseTreeModel::getColumn(position));
    }

    StandardModelRow<T>& addRow(std::unique_ptr<StandardModelRow<T>> row)
    {
        return static_cast<StandardModelRow<T>&>(BaseTreeModel::addRow(std::move(row)));
    }

    size_t getRowCount() const
    {
        return BaseTreeModel::getRowCount();
    }

    StandardModelRow<T>& getRow(size_t position)
    {
        return static_cast<StandardModelRow<T>&>(BaseTreeModel::getRow(position));
    }

    const StandardModelRow<T>& getRow(size_t position) const
    {
        return static_cast<const StandardModelRow<T>&>(BaseTreeModel::getRow(position));
    }

    void removeRow(size_t position)
    {
        BaseTreeModel::removeRow(position);
    }

    void reset()
    {
        BaseTreeModel::reset();
    }
};

template <class T>
StandardModel<T>* StandardModelRow<T>::getStandardModel() const
{
    return static_cast<StandardModel<T>*>(getTreeModel());
}