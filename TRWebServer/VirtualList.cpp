#include "stdafx.h"
#include "VirtualList.h"

class VirtualListModel: public Wt::WAbstractTableModel
{
public:
    VirtualListModel(int rows):
        m_rows(rows)
    { 
    }

    void reset(int rows)
    {
        m_rows = rows;
        Wt::WAbstractTableModel::reset();
    }

    virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const
    {
        return parent.isValid() ? 0 : m_rows;
    }

    virtual int columnCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const
    {
        return parent.isValid() ? 0 : 1;        
    }

    virtual Wt::cpp17::any data(const Wt::WModelIndex& index, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const override
    {
        switch (role.value())
        {
        case Wt::ItemDataRole::Display:
            return Wt::WString("");

        default:
            return Wt::cpp17::any();
        }
    }

    /*virtual boost::any headerData(int section, Wt::Orientation orientation = Wt::Horizontal, int role = Wt::DisplayRole) const
    {
        return boost::any();
    }*/

private:
    int m_rows;
};

class VirtualListDelegate: public Wt::WAbstractItemDelegate
{
public:
    VirtualListDelegate(VirtualList& virtualList):
        m_virtualList(virtualList)
    {
    }

    virtual std::unique_ptr<Wt::WWidget> update(Wt::WWidget* widget, const Wt::WModelIndex& index, Wt::WFlags<Wt::ViewItemRenderFlag> flags)
    {
        if (widget)
        {
            return nullptr;
        }
        return m_virtualList.createWidget(index.row());
    }

private:
    VirtualList& m_virtualList;
};

VirtualList::VirtualList(int size)
{
    setModel(std::make_shared<VirtualListModel>(size));
    setColumnWidth(0, 1200);
    setHeaderHeight(1);
    setItemDelegate(std::make_shared<VirtualListDelegate>(*this));
}

void VirtualList::reset(int size)
{
    static_cast<VirtualListModel*>(model().get())->reset(size);
}