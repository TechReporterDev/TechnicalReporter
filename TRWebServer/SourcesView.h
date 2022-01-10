#pragma once
#include "StandardModel.h"
#include "StandardTable.h"
#include "TRWebServer.h"

class SourcesView: public Wt::WContainerWidget
{
public:
    struct SourceItem
    {
        using ID = TR::SourceKey;
        
        ID                  getID() const;
        boost::optional<ID> getParentID() const;
        TR::SourceInfo      m_sourceInfo;
    };

    struct SuitRootItem
    {       
        using ID = TR::SourceKey;
        
        ID                  getID() const;
        boost::none_t       getParentID() const;
        TR::SourceInfo      m_sourceInfo;
    };

    struct SuitItem
    {
        using ID = TR::SourceKey;

        ID                  getID() const;
        SuitRootItem::ID    getParentID() const;
        TR::SourceInfo      m_sourceInfo;
    };

    struct LinkItem
    {
        using ID = std::pair<TR::SourceKey, TR::SourceKey>;

        ID                  getID() const;
        SuitItem::ID        getParentID() const;
        TR::Link            m_link;
    };

    using Item = boost::variant<SourceItem, SuitRootItem, SuitItem, LinkItem>;
    using ItemID = boost::variant<SourceItem::ID, LinkItem::ID>;
    using OnDblClick = std::function<void(Item)>;
    
    SourcesView(OnDblClick onDblClick);

    static ItemID                               getID(Item item);
    static boost::optional<ItemID>              getParentID(Item item);

    virtual void                                addItem(Item item) = 0;
    virtual void                                updateItem(Item item) = 0;
    virtual void                                removeItem(ItemID itemID) = 0;
    virtual std::vector<Item>                   getSelection() const = 0;
    virtual void                                clearSelection() = 0;
    virtual void                                selectItem(ItemID itemID) = 0;

protected:
    OnDblClick m_onDblClick;
};