#pragma once
#include "TRWebServer.h"
#include "CommonProperty.h"
#include "PropertyGrid.h"
#include "StandardTable.h"

using CommonPropertyGrid = PropertyGrid<StringPropertyFactory, SelectorPropertyFactory, DynamicSelectorPropertyFactory, AnyPropertyFactory>;

class CommonPropertyTable: public StandardTable<size_t>
{
public:
    using AnyPresenter = std::function<std::unique_ptr<Wt::WWidget>(AnyProperty& anyProperty)>;

    CommonPropertyTable(CommonPropertyGrid propertyGrid);
    CommonPropertyTable(std::initializer_list<CommonPropertyGrid::Column> columns);

    void            setAnyPresenter(AnyPresenter anyPresenter);
    AnyPresenter    getAnyPresenter() const;

private:
    virtual void load() override;

    AnyPresenter m_anyPresenter;
    CommonPropertyGrid m_propertyGrid;
};