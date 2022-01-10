#pragma once
#include "StandardTable.h"
#include "TRWebServer.h"
#include "MainFrame.h"

class AlertsTable;
using AlertRow = StandardModelRow<TR::VariantAlert>;
using AlertTableColumn = StandardTableColumn<TR::VariantAlert>;

class AlertSeverityColumn: public AlertTableColumn
{
public:
    virtual void            create(const AlertRow& row, Wt::WContainerWidget& container, Wt::WFlags<Wt::ViewItemRenderFlag> flags) override;
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const AlertRow& row) const override;
};

class AlertSourceColumn: public AlertTableColumn
{
public:
    AlertSourceColumn(const std::map<TR::SourceKey, TR::SourceInfo>& sourcesInfo);
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const AlertRow& row) const override;

private:
    const std::map<TR::SourceKey, TR::SourceInfo>& m_sourcesInfo;
};

class AlertResourceColumn: public AlertTableColumn
{
public:
    AlertResourceColumn(const std::map<TR::ReportTypeUUID, TR::ReportTypeInfo>& reportTypesInfo, const std::map<TR::ActionUUID, TR::ActionInfo>& actionsInfo, const std::map<TR::UUID, TR::ActionShortcutInfo>& shortcutsInfo);
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const AlertRow& row) const override;

private:
    const std::map<TR::ReportTypeUUID, TR::ReportTypeInfo>& m_reportTypesInfo;
    const std::map<TR::ActionUUID, TR::ActionInfo>& m_actionsInfo;
    const std::map<TR::UUID, TR::ActionShortcutInfo>& m_shortcutsInfo;
};

class AlertMessageColumn: public AlertTableColumn
{
public:
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const AlertRow& row) const override;
};

class AlertsTable: public StandardTable<TR::VariantAlert>
{
public:
    AlertsTable();
};