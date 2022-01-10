#include "stdafx.h"
#include "RegularDiffModel.h"
#include "Application.h"

RegularDiffNodeRow::RegularDiffNodeRow(const TR::XML::XmlRegularNode* newRegularNode, const TR::XML::XmlRegularNode* oldRegularNode):
    XmlDiffNodeRow(newRegularNode, oldRegularNode)
{
}

RegularDiffModel* RegularDiffNodeRow::getRegularDiffModel() const
{
    return static_cast<RegularDiffModel*>(getXmlDiffModel());
}

const TR::XML::XmlRegularNode* RegularDiffNodeRow::getNewRegNode() const
{
    return static_cast<const TR::XML::XmlRegularNode*>(getNewNode());
}

const TR::XML::XmlRegularNode* RegularDiffNodeRow::getOldRegNode() const
{
    return static_cast<const TR::XML::XmlRegularNode*>(getOldNode());
}

const TR::XML::XmlDefNode& RegularDiffNodeRow::getDefNode() const
{
    if (auto newRegularNode = getNewRegNode())
    {
        return newRegularNode->get_bound_def();
    }
    else if (auto oldRegularNode = getOldRegNode())
    {
        return oldRegularNode->get_bound_def();
    }
    throw std::logic_error("Broken regular diff model found");
}

RegularDiffNodeRow::State RegularDiffNodeRow::getDiffState() const
{
    return XmlDiffNodeRow::getDiffState();
}

std::wstring RegularDiffColumn::getString(const XmlDiffNodeRow& row) const
{
    return getString(static_cast<const RegularDiffNodeRow&>(row));
}

bool RegularDiffColumn::less(const XmlDiffNodeRow& left, const XmlDiffNodeRow& right) const
{
    return less(static_cast<const RegularDiffNodeRow&>(left), static_cast<const RegularDiffNodeRow&>(right));
}

bool RegularDiffColumn::less(const RegularDiffNodeRow& left, const RegularDiffNodeRow& right) const
{
    return XmlDiffColumn::less(left, right);
}

std::wstring RegularDiffNameColumn::getName() const
{
    return L"Name";
}

std::wstring RegularDiffNameColumn::getString(const RegularDiffNodeRow& diffRow) const
{
    const TR::XML::XmlRegularNode* regNode = nullptr;
    return diffRow.getDefNode().get_caption();
}

std::wstring RegularDiffNewValueColumn::getName() const
{
    return L"New value";
}

std::wstring RegularDiffNewValueColumn::getString(const RegularDiffNodeRow& diffRow) const
{
    if (auto newRegNode = diffRow.getNewRegNode())
    {
        if (newRegNode->get_bound_def().is_trivial())
        {
            return xml_tools::get_node_wtext(*newRegNode);
        }
    }
    return L"";
}

std::wstring RegularDiffOldValueColumn::getName() const
{
    return L"Old value";
}

std::wstring RegularDiffOldValueColumn::getString(const RegularDiffNodeRow& diffRow) const
{
    if (auto oldRegNode = diffRow.getOldRegNode())
    {
        if (oldRegNode->get_bound_def().is_trivial())
        {
            return xml_tools::get_node_wtext(*oldRegNode);
        }
    }
    return L"";
}

std::wstring RegularDiffDescriptionColumn::getName() const
{
    return L"Description";
}

std::wstring RegularDiffDescriptionColumn::getString(const RegularDiffNodeRow& diffRow) const
{
    return diffRow.getDefNode().get_description();
}

RegularDiffModel::RegularDiffModel(const TR::XML::XmlRegularDoc& newDoc, const TR::XML::XmlRegularDoc& oldDoc, const std::vector<TR::Core::XmlDiffPoint>& diffPoints)
{
    XmlDiffModel::build(newDoc, oldDoc, diffPoints);
}

void RegularDiffModel::addColumn(std::unique_ptr<RegularDiffColumn> column)
{
    XmlDiffModel::addColumn(std::unique_ptr<XmlDiffColumn>(column.release()));
}

RegularDiffColumn& RegularDiffModel::getColumn(size_t position) const
{
    return dynamic_cast<RegularDiffColumn&>(XmlDiffModel::getColumn(position));
}

std::unique_ptr<XmlDiffNodeRow> RegularDiffModel::createDiffNodeRow(const TR::XML::XmlNode* newNode, const TR::XML::XmlNode* oldNode)
{
    return std::unique_ptr<XmlDiffNodeRow>(new RegularDiffNodeRow(
        static_cast<const TR::XML::XmlRegularNode*>(newNode),
        static_cast<const TR::XML::XmlRegularNode*>(oldNode)));
}
