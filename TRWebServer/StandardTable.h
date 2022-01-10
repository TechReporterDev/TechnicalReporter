#pragma once
#include "StandardModel.h"
#include "TRWebServer.h"
#include "MainFrame.h"

template<class T>
class StandardTableColumn: public Wt::WAbstractItemDelegate, public StandardModelColumn<T>
{
public:
    StandardTableColumn()
    {
    }

    virtual std::unique_ptr<Wt::WWidget> update(Wt::WWidget* widget, const Wt::WModelIndex& index, Wt::WFlags<Wt::ViewItemRenderFlag> flags)
    {
        if (widget)
        {
            //return widget;
        }

        auto container = std::make_unique<Wt::WContainerWidget>();
        if (flags.test(Wt::ViewItemRenderFlag::Selected))
        {
            container->addStyleClass("active");
        }

        auto& row = getRowFromIndex(index);
        create(dynamic_cast<const StandardModelRow<T>&>(row), *container, flags);
        return container;
    }

    virtual void create(const StandardModelRow<T>& row, Wt::WContainerWidget& container, Wt::WFlags<Wt::ViewItemRenderFlag> flags)
    {
        auto cellWidget = create(row, flags);
        container.addWidget(std::move(cellWidget));
    }

    virtual std::unique_ptr<Wt::WWidget> create(const StandardModelRow<T>& row, Wt::WFlags<Wt::ViewItemRenderFlag> flags)
    {
        return std::make_unique<Wt::WText>(getString(row), Wt::TextFormat::Plain);
    }
};

template<class T>
class StandardTable: public Wt::WTableView
{
public:
    StandardTable():
        m_model(std::make_shared<StandardModel<T>>())
    {
        setModel(m_model);
    }   

    void addColumn(std::shared_ptr<StandardTableColumn<T>> column)
    {       
        m_model->addColumn(column);
        setItemDelegateForColumn(columnCount() - 1, column);
    }

    size_t getColumnCount() const
    {
        return m_model->getColumnCount();
    }

    void addRow(T data)
    {
        m_model->addRow(std::make_unique<StandardModelRow<T>>(std::move(data)));
    }

    size_t getRowCount() const
    {
        return m_model->getRowCount();
    }

    const T& getData(size_t position) const
    {
        return m_model->getRow(position).getData();
    }

    void updateRow(size_t position, T data)
    {
        auto& row = m_model->getRow(position);
        row.setData(std::move(data));
    }

    void removeRow(size_t position)
    {
        m_model->removeRow(position);
    }

    void clear()
    {
        m_model->reset();
    }

    std::vector<size_t> getSelectedRows() const
    {
        std::vector<size_t> selectedRows;
        for (auto index : selectedIndexes())
        {
            selectedRows.push_back(index.row());
        }
        return selectedRows;
    }

    void selectRow(size_t position)
    {
        select(m_model->fromRawIndex(&m_model->getRow(position)));
    }

    void clearSelection()
    {
        setSelectedIndexes(Wt::WModelIndexSet());
    }

protected:
    std::shared_ptr<StandardModel<T>> m_model;
};