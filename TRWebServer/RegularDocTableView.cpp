#include "stdafx.h"
#include "RegularDocTableView.h"
#include "Application.h"

BooleanNodeTableColumn::BooleanNodeTableColumn(const TR::XML::XMLRegularTable& regularTable, int columnIndex):
    RegularNodeTableColumn<TR::XML::XmlBooleanNode>(regularTable, columnIndex)
{
}

std::wstring BooleanNodeTableColumn::getString(const TR::XML::XmlBooleanNode& booleanNode) const
{
    if (booleanNode.get_value())
    {
        return L"True";
    }
    return L"False";
}

EnumNodeTableColumn::EnumNodeTableColumn(const TR::XML::XMLRegularTable& regularTable, int columnIndex):
    RegularNodeTableColumn<TR::XML::XmlEnumNode>(regularTable, columnIndex)
{
    auto& enumDef = TR::XML::cast_def_node<TR::XML::XmlEnumDef>(*m_regularTable.m_header[m_columnIndex]);
    m_names = enumDef.get_names();
}

std::wstring EnumNodeTableColumn::getString(const TR::XML::XmlEnumNode& enumNode) const
{
    return m_names.at(enumNode.get_value());
}

struct MakeTableColumn: boost::static_visitor<std::unique_ptr<RegularDocTableColumn>>
{
    MakeTableColumn(const TR::XML::XMLRegularTable& regularTable, int columnIndex):
        m_regularTable(regularTable),
        m_columnIndex(columnIndex)
    {
    }

    std::unique_ptr<RegularDocTableColumn> operator()(const TR::XML::XmlStringDef&) const
    {
        return std::make_unique<RegularNodeTableColumn<TR::XML::XmlStringNode>>(m_regularTable, m_columnIndex);
    }

    std::unique_ptr<RegularDocTableColumn> operator()(const TR::XML::XmlIntegerDef&) const
    {
        return std::make_unique<RegularNodeTableColumn<TR::XML::XmlIntegerNode>>(m_regularTable, m_columnIndex);
    }

    std::unique_ptr<StandardTableColumn<int>> operator()(const TR::XML::XmlBooleanDef&) const
    {
        return std::make_unique<BooleanNodeTableColumn>(m_regularTable, m_columnIndex);
    }

    std::unique_ptr<StandardTableColumn<int>> operator()(const TR::XML::XmlEnumDef&) const
    {
        return std::make_unique<EnumNodeTableColumn>(m_regularTable, m_columnIndex);
    }

    std::unique_ptr<StandardTableColumn<int>> operator()(const TR::XML::XmlDefNode&) const
    {
        throw std::logic_error("Invalid column type");
    }
    
    const TR::XML::XMLRegularTable& m_regularTable;
    int m_columnIndex;
};

RegularDocTableView::RegularDocTableView(std::shared_ptr<const TR::XML::XmlRegularDoc> doc, std::shared_ptr<const TR::XML::XmlDefDoc> def):
    m_doc(doc),
    m_def(def),
    m_regular_table(TR::XML::make_regular_table(const_cast<TR::XML::XmlRegularDoc&>(*doc)))
{
    for (size_t column = 0; column < m_regular_table.m_header.size(); ++column)
    {
        addColumn(TR::XML::visit_def_node(MakeTableColumn(m_regular_table, column), *m_regular_table.m_header[column]));
    }

    for (size_t row = 0; row < m_regular_table.m_rows.size(); ++row)
    {
        addRow(row);
    }
}