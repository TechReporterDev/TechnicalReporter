#pragma once
#include "SourcesView.h"
#include "SourcesTableView.h"
#include "TRWebServer.h"

using SourcesTreeModel = StandardModel<SourcesView::Item>;
using SourcesTreeRow = StandardModelRow<SourcesView::Item>;

class SourcesTreeView: public SourcesView
{
public:
    SourcesTreeView(const std::vector<SourcesView::Item>& items, OnDblClick onDblClick);
    ~SourcesTreeView();
    void                                addColumn(std::shared_ptr<SourceColumn> column);
    void                                expand(int depth);

    virtual void                        addItem(Item item) override;
    virtual void                        updateItem(Item item) override;
    virtual void                        removeItem(ItemID itemID) override;
    virtual std::vector<Item>           getSelection() const override;
    virtual void                        clearSelection() override;
    virtual void                        selectItem(ItemID itemID) override; 

private:
    virtual void load() override;
    SourcesTreeRow* findItemRow(ItemID itemID, SourcesTreeRow* _where = nullptr);
    
    std::shared_ptr<SourcesTreeModel> m_model;
    Wt::WTreeView* m_treeView;
};