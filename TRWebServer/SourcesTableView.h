#pragma once
#include "SourcesView.h"
#include "StandardTable.h"
#include "TRWebServer.h"

using SourceRow = StandardModelRow<SourcesView::Item>;
using SourceColumn = StandardTableColumn<SourcesView::Item>;

class SourceNameColumn: public SourceColumn
{
public:
    SourceNameColumn(std::map<TR::SourceKey, TR::SourceInfo>& sourceByKey);

    virtual std::wstring getName() const override;
    virtual std::wstring getString(const SourceRow& row) const override;

private:
    std::map<TR::SourceKey, TR::SourceInfo>& m_sourceByKey;
};

class SourceParentColumn: public SourceColumn
{
public:
    SourceParentColumn(std::map<TR::SourceKey, TR::SourceInfo>& sourceByKey);

    virtual std::wstring getName() const override;
    virtual std::wstring getString(const SourceRow& row) const override;

private:
    std::map<TR::SourceKey, TR::SourceInfo>& m_sourceByKey;
};

class SourceTypeColumn: public SourceColumn
{
public:
    SourceTypeColumn(std::map<TR::UUID, TR::SourceTypeInfo>& sourceTypeByUUID);

    virtual std::wstring getName() const override;
    virtual std::wstring getString(const SourceRow& row) const override;

private:
    std::map<TR::UUID, TR::SourceTypeInfo>& m_sourceTypeByUUID;
};

class SourcesTableView: public SourcesView
{
public:
    SourcesTableView(const std::vector<SourcesView::Item>& items, OnDblClick onDblClick);
    void addColumn(std::unique_ptr<SourceColumn> column);


    // SourcesView override
    virtual void                                addItem(Item item) override;
    virtual void                                updateItem(Item item) override;
    virtual void                                removeItem(ItemID itemID) override;
    virtual std::vector<Item>                   getSelection() const override;
    virtual void                                clearSelection() override;
    virtual void                                selectItem(ItemID itemID) override;

private:
    size_t                                      findItemRow(ItemID itemID);
    StandardTable<Item>* m_tableView;
};
