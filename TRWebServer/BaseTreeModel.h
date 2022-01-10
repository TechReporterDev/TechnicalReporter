#pragma once
#include "TRWebServer.h"

extern const size_t MAX_ROW_POS;
extern const size_t MAX_COL_POS;

class BaseTreeModelRow
{
public:
    friend class BaseTreeModel;

    BaseTreeModelRow(BaseTreeModel* treeModel = nullptr);
    virtual ~BaseTreeModelRow() = default;

    BaseTreeModelRow(const BaseTreeModelRow&) = delete;
    BaseTreeModelRow& operator = (const BaseTreeModelRow&) = delete;

    //dynamicDowncast support
    virtual bool checkType(const std::type_info& tpi) const;

    BaseTreeModelRow*                   getParentRow();
    const BaseTreeModelRow*             getParentRow() const;
    size_t                              getPosition() const;
    bool                                isTop() const;
    void                                emitChanged();

    BaseTreeModelRow&                   addChildRow(std::unique_ptr<BaseTreeModelRow> childRow, size_t position = MAX_ROW_POS);
    size_t                              getChildCount() const;  
    BaseTreeModelRow&                   getChildRow(size_t position);
    const BaseTreeModelRow&             getChildRow(size_t position) const;
    void                                removeChildRow(size_t position);
    std::unique_ptr<BaseTreeModelRow>   releaseChildRow(size_t position);
    void                                swapChildRows(size_t first, size_t second);

    BaseTreeModel*                      getTreeModel() const;
    
private:
    void                                setTreeModel(BaseTreeModel* treeModel); 

    BaseTreeModel* m_treeModel;
    BaseTreeModelRow* m_parentRow;
    std::vector<std::unique_ptr<BaseTreeModelRow>> m_childRows;
};

class BaseTreeModelColumn
{
public:
    virtual ~BaseTreeModelColumn() = default;

    //dynamicDowncast support
    virtual bool            checkType(const std::type_info& tpi) const;

    virtual std::wstring    getName() const = 0;
    virtual std::wstring    getString(const BaseTreeModelRow& row) const = 0;
    
    virtual bool less(const BaseTreeModelRow& left, const BaseTreeModelRow& right) const
    {
        return getString(left) < getString(right);
    }
};

class BaseTreeModel: public Wt::WAbstractItemModel
{
public:
    friend class BaseTreeModelRow;

    BaseTreeModel(std::unique_ptr<BaseTreeModelRow> hiddenRoot = nullptr);
    virtual ~BaseTreeModel(){}

    BaseTreeModel(const BaseTreeModel&) = delete;
    BaseTreeModel& operator = (const BaseTreeModel&) = delete;  

    //dynamicDowncast support
    virtual bool checkType(const std::type_info& tpi) const;

    void                        addColumn(std::shared_ptr<BaseTreeModelColumn> column, size_t position = MAX_COL_POS);
    BaseTreeModelColumn&        getColumn(size_t position) const;
    size_t                      getColumnCount() const;

    BaseTreeModelRow&           addRow(std::unique_ptr<BaseTreeModelRow> row, size_t position = MAX_ROW_POS);
    size_t                      getRowCount() const;
    BaseTreeModelRow&           getRow(size_t position);
    const BaseTreeModelRow&     getRow(size_t position) const;
    void                        removeRow(size_t position);
    void                        reset(std::unique_ptr<BaseTreeModelRow> hiddenRoot = nullptr);

    BaseTreeModelRow&           getHiddenRoot();

public:
    //Wt::WAbstractItemModel override
    using Wt::WAbstractItemModel::removeRow;
    virtual Wt::WModelIndex     parent(const Wt::WModelIndex& index) const;
    virtual Wt::WModelIndex     index(int row, int column, const Wt::WModelIndex& parent = Wt::WModelIndex()) const;
    virtual int                 columnCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const;
    virtual int                 rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const;
    virtual void                sort(int column, Wt::SortOrder order);
    virtual void                sort(std::vector<std::unique_ptr<BaseTreeModelRow>>& rows, std::function<bool(const BaseTreeModelRow& left, const BaseTreeModelRow& right)> pred);
    virtual void*               toRawIndex(const Wt::WModelIndex& index) const;
    virtual Wt::WModelIndex     fromRawIndex(void *rawIndex) const;
    virtual Wt::cpp17::any      data(const Wt::WModelIndex& index, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const;
    virtual Wt::cpp17::any      headerData(int section, Wt::Orientation orientation = Wt::Orientation::Horizontal, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const;
    void                        insertTree(Wt::WModelIndex topIndex, bool insertTop = true);

    BaseTreeModelRow&           getRowFromIndex(Wt::WModelIndex index) const;
    Wt::WModelIndex             getIndexFromRow(BaseTreeModelRow& row) const;

private:
    std::unique_ptr<BaseTreeModelRow> m_hiddenRoot;
    std::vector<std::shared_ptr<BaseTreeModelColumn>> m_columns;
};

BaseTreeModelRow& getRowFromIndex(Wt::WModelIndex index);
Wt::WModelIndex getIndexFromRow(BaseTreeModelRow& row);

template<class D, class B>
D& dynamicDowncast(B& base_ref)
{
    static_assert(std::is_base_of<B, D>::value, "");
    if (base_ref.checkType(typeid(D)))
    {
        return static_cast<D&>(base_ref);
    }
    throw std::bad_cast();
}

template<class D, class B>
D* dynamicDowncast(B* base_ptr)
{
    static_assert(std::is_base_of<B, D>::value, "");
    if (base_ptr->checkType(typeid(D)))
    {
        return static_cast<D*>(base_ptr);
    }
    return nullptr;
}

#define STATIC_POINTER_CAST(TYPE, Pointer) std::unique_ptr<TYPE>(static_cast<TYPE*>(Pointer.release()))
