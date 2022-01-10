#pragma once
#include "TRWebServer.h"
#include "BaseTreeModel.h"
#include "XmlDiffModel.h"

class RegularDiffNodeRow: private XmlDiffNodeRow
{
public:
    friend class RegularDiffModel;
    friend class RegularDiffColumn;

    using State = XmlDiffNodeRow::State;
    RegularDiffNodeRow(const TR::XML::XmlRegularNode* newRegularNode, const TR::XML::XmlRegularNode* oldRegularNode = nullptr);

    RegularDiffModel*               getRegularDiffModel() const;    
    
    const TR::XML::XmlRegularNode*  getNewRegNode() const;
    const TR::XML::XmlRegularNode*  getOldRegNode() const;
    const TR::XML::XmlDefNode&      getDefNode() const;
    State                           getDiffState() const;
};

class RegularDiffColumn: private XmlDiffColumn
{
public:
    friend class RegularDiffModel;

    virtual std::wstring    getString(const XmlDiffNodeRow& row) const override;
    virtual bool            less(const XmlDiffNodeRow& left, const XmlDiffNodeRow& right) const override;

    virtual std::wstring    getString(const RegularDiffNodeRow&) const = 0;
    virtual bool            less(const RegularDiffNodeRow& left, const RegularDiffNodeRow& right) const;
};

class RegularDiffNameColumn: public RegularDiffColumn
{
public:
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const RegularDiffNodeRow& diffRow) const override;
};

class RegularDiffNewValueColumn: public RegularDiffColumn
{
public:
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const RegularDiffNodeRow& diffRow) const override;
};

class RegularDiffOldValueColumn: public RegularDiffColumn
{
public:
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const RegularDiffNodeRow& diffRow) const override;
};


class RegularDiffDescriptionColumn: public RegularDiffColumn
{
public:
    virtual std::wstring    getName() const;
    virtual std::wstring    getString(const RegularDiffNodeRow& diffRow) const override;
};

class RegularDiffModel: private XmlDiffModel
{
public:
    friend RegularDiffModel* RegularDiffNodeRow::getRegularDiffModel() const;
    friend class RegularDiffTreeView;

    RegularDiffModel(const TR::XML::XmlRegularDoc& newDoc, const TR::XML::XmlRegularDoc& oldDoc, const std::vector<TR::Core::XmlDiffPoint>& diffPoints);
    virtual ~RegularDiffModel() = default;
    RegularDiffModel(const RegularDiffModel&) = delete;
    RegularDiffModel& operator = (const RegularDiffModel&) = delete;

    void                            addColumn(std::unique_ptr<RegularDiffColumn> column);
    RegularDiffColumn&              getColumn(size_t position) const;

private:
    //XmlDiffModel override
    virtual std::unique_ptr<XmlDiffNodeRow> createDiffNodeRow(const TR::XML::XmlNode* newNode, const TR::XML::XmlNode* oldNode = nullptr);
};