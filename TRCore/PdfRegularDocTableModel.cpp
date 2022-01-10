#include "stdafx.h"
#include "PdfRegularDocTableModel.h"

using namespace TR::XML;

namespace PoDoFo {

struct XmlRegularNodeTableRow
{
    unsigned m_level = 0;
    std::wstring m_name;
    std::wstring m_value;
};

struct XmlRegularNodeRowsCollector : boost::static_visitor<void>
{
    XmlRegularNodeRowsCollector(XmlRegularNodeTableRows& rows, unsigned depth = 0) :
        m_rows(rows),
        m_depth(depth)
    {
    }

    void operator ()(const XmlStructNode& struct_node)
    {
        auto& def_node = struct_node.get_bound_def();
        m_rows.push_back({ m_depth, def_node.get_caption() });

        for (auto& regular_node : struct_node.get_members())
        {
            visit_regular_node(XmlRegularNodeRowsCollector(m_rows, m_depth + 1), regular_node);
        }
    }

    template<class T>
    void operator ()(const XmlListNode<T>& list_node)
    {
        auto& def_node = list_node.get_bound_def();
        m_rows.push_back({ m_depth, def_node.get_caption() });

        for (auto& regular_node : list_node.get_items())
        {
            visit_regular_node(XmlRegularNodeRowsCollector(m_rows, m_depth + 1), regular_node);
        }
    }

    void operator ()(const XmlBooleanNode& reg_node)
    {
        auto& def_node = reg_node.get_bound_def();
        m_rows.push_back({ m_depth, def_node.get_caption(), reg_node.get_wtext() });
    }

    void operator ()(const XmlIntegerNode& reg_node)
    {
        auto& def_node = reg_node.get_bound_def();
        m_rows.push_back({ m_depth, def_node.get_caption(), reg_node.get_wtext() });
    }

    void operator ()(const XmlStringNode& reg_node)
    {
        auto& def_node = reg_node.get_bound_def();
        m_rows.push_back({ m_depth, def_node.get_caption(), reg_node.get_value() });
    }

    void operator ()(const XmlEnumNode& reg_node)
    {
        auto& def_node = reg_node.get_bound_def();
        m_rows.push_back({ m_depth, def_node.get_caption(), reg_node.get_wtext() });
    }

    XmlRegularNodeTableRows& m_rows;
    unsigned m_depth;
};

XmlRegularNodeTableRows GetRows(const XmlRegularDoc& regDoc)
{
    XmlRegularNodeTableRows rows;
    visit_regular_node(XmlRegularNodeRowsCollector(rows), *regDoc.get_root());
    return rows;
}

XmlRegularDocPdfTableModel::XmlRegularDocPdfTableModel(const XmlRegularDoc& regDoc, unsigned hideRoot) :
    m_rows(GetRows(regDoc)),
    m_hideRoot(hideRoot)
{
}

XmlRegularDocPdfTableModel::~XmlRegularDocPdfTableModel() = default;

PdfString XmlRegularDocPdfTableModel::GetText(int col, int row) const
{
    if (row == 0)
    {
        switch (col)
        {
        case 0:
            return "Parameter";

        case 1:
            return "Value";

        default:
            _ASSERT(false);
        }

        return "";
    }

    auto& xml_row = m_rows.at(row - 1 + size_t(m_hideRoot));
    switch (col)
    {
    case 0:
    {
        std::wstring text(4 * (xml_row.m_level - size_t(m_hideRoot)), L' ');
        text += xml_row.m_name;
        return stl_tools::ucs_to_ansi(text);
    }

    case 1:
        return stl_tools::ucs_to_ansi(xml_row.m_value);

    default:
        _ASSERT(false);
    }
    return "";
}

EPdfAlignment XmlRegularDocPdfTableModel::GetAlignment(int col, int row) const
{
    return EPdfAlignment::ePdfAlignment_Left;
}

EPdfVerticalAlignment XmlRegularDocPdfTableModel::GetVerticalAlignment(int col, int row) const
{
    return EPdfVerticalAlignment::ePdfVerticalAlignment_Center;
}

PdfFont* XmlRegularDocPdfTableModel::GetFont(int col, int row) const
{
    return nullptr;
}

bool XmlRegularDocPdfTableModel::HasBackgroundColor(int col, int row) const
{
    return false;
}

PdfColor XmlRegularDocPdfTableModel::GetBackgroundColor(int col, int row) const
{
    return PdfColor(0, 0, 0);
}

PdfColor XmlRegularDocPdfTableModel::GetForegroundColor(int col, int row) const
{
    return PdfColor(0, 0, 0);
}

bool XmlRegularDocPdfTableModel::HasWordWrap(int col, int row) const
{
    return true;
}

bool XmlRegularDocPdfTableModel::HasBorders() const
{
    return true;
}

double XmlRegularDocPdfTableModel::GetBorderWidth() const
{
    return 1.0;
}

PdfColor XmlRegularDocPdfTableModel::GetBorderColor(int col, int row) const
{
    return PdfColor(0, 0, 0);
}

bool XmlRegularDocPdfTableModel::HasImage(int col, int row) const
{
    return false;
}

PoDoFo::PdfImage* XmlRegularDocPdfTableModel::GetImage(int col, int row) const
{
    return nullptr;
}

size_t XmlRegularDocPdfTableModel::GetSize() const
{
    return m_rows.size() + 1 - size_t(m_hideRoot);
}

} //namespace PoDoFo {