#pragma once
#include "StandardTable.h"
#include "TRWebServer.h"

using RegularDocTableRow = StandardModelRow<int>;
using RegularDocTableColumn = StandardTableColumn<int>;

template<class XmlNodeType>
class RegularNodeTableColumn: public RegularDocTableColumn
{
public:
    RegularNodeTableColumn(const TR::XML::XMLRegularTable& regularTable, int columnIndex):
        m_regularTable(regularTable),
        m_columnIndex(columnIndex)
    {
    }

    virtual std::wstring getName() const override
    {
        return m_regularTable.m_header[m_columnIndex]->get_caption();
    }

    virtual std::wstring getString(const RegularDocTableRow& row) const override
    {
        if (auto rowNode = getRowNode(row))
        {
            return getString(*rowNode);
        }
        return L"-";
    }

    virtual std::wstring getString(const XmlNodeType& node) const
    {
        return node.get_wtext();
    }

    virtual bool less(const RegularDocTableRow& left, const RegularDocTableRow& right) const override
    {
        auto rightNode = getRowNode(right);
        if (!rightNode)
        {
            return false;
        }

        auto leftNode = getRowNode(left);
        if (!leftNode)
        {
            return true;
        }

        return leftNode->get_value() < rightNode->get_value();
    }

    const XmlNodeType* getRowNode(const RegularDocTableRow& row) const
    {
        return TR::XML::cast_regular_node<XmlNodeType>(m_regularTable.m_rows[row.getData()][m_columnIndex]);
    }

protected:
    const TR::XML::XMLRegularTable& m_regularTable;
    int m_columnIndex;
};

class BooleanNodeTableColumn: public RegularNodeTableColumn<TR::XML::XmlBooleanNode>
{
public:
    BooleanNodeTableColumn(const TR::XML::XMLRegularTable& regularTable, int columnIndex);
    virtual std::wstring getString(const TR::XML::XmlBooleanNode& booleanNode) const override;  
};

class EnumNodeTableColumn: public RegularNodeTableColumn<TR::XML::XmlEnumNode>
{
public:
    EnumNodeTableColumn(const TR::XML::XMLRegularTable& regularTable, int columnIndex);
    virtual std::wstring getString(const TR::XML::XmlEnumNode& enumNode) const override;

private:
    std::vector<std::wstring> m_names;
};

class RegularDocTableView: public StandardTable<int>
{
public:
    RegularDocTableView(std::shared_ptr<const TR::XML::XmlRegularDoc> doc, std::shared_ptr<const TR::XML::XmlDefDoc> def);

private:
    std::shared_ptr<const TR::XML::XmlRegularDoc> m_doc;
    std::shared_ptr<const TR::XML::XmlDefDoc> m_def;
    TR::XML::XMLRegularTable m_regular_table;
};