#include "stdafx.h"
#include "PdfCursorPainter.h"

namespace PoDoFo {

PdfCursorPainter::PdfCursorPainter(PoDoFo::PdfStreamedDocument& pdf):
    m_pdf(pdf),
    m_cursor(A4_HEIGHT - TOP_MARGIN)
{
    auto page = m_pdf.CreatePage(PdfPage::CreateStandardPageSize(ePdfPageSize_A4));
    auto size = page->GetPageSize();
    if (!page)
    {
        PODOFO_RAISE_ERROR(ePdfError_InvalidHandle);
    }

    m_painter.SetPage(page);
}

PdfCursorPainter::~PdfCursorPainter()
{
    m_painter.FinishPage();
}

void PdfCursorPainter::SetFont(const PoDoFo::PdfString& fontName, double fontSize)
{
    auto font = m_pdf.CreateFont(fontName.GetString());
    if (!font)
    {
        PODOFO_RAISE_ERROR(ePdfError_InvalidHandle);
    }

    font->SetFontSize(float(fontSize));
    m_painter.SetFont(font);
}

const PdfFont* PdfCursorPainter::GetFont() const
{
    return m_painter.GetFont();
}

void PdfCursorPainter::DrawText(double x, const PoDoFo::PdfString& text, bool moveCursor)
{
    if (moveCursor)
    {
        auto fontSize = m_painter.GetFont()->GetFontSize();
        ReserveSpace(fontSize);
        MoveCursor(fontSize);
    }

    m_painter.DrawText(x, m_cursor, text);    
}

void PdfCursorPainter::DrawTable(double x, PoDoFo::PdfTable& table, bool moveCursor)
{
    double position = 0;
    table.Draw(x, m_cursor, &m_painter, PdfRect(), nullptr, &position);
    if (moveCursor)
    {
        MoveCursor(m_cursor - position);
    }
}

void PdfCursorPainter::ReserveSpace(double space)
{
    if (m_cursor - space < BOTTOM_MARGIN)
    {
        m_painter.FinishPage();
        auto page = m_pdf.CreatePage(PdfPage::CreateStandardPageSize(ePdfPageSize_A4));
        m_painter.SetPage(page);
        m_cursor = A4_HEIGHT - TOP_MARGIN;
    }
}

void PdfCursorPainter::MoveCursor(double offset)
{
    m_cursor -= offset;
    ReserveSpace(0);
}

std::vector<double> GetRowHeights(const PdfTableModel& model, const size_t rowCount, const std::vector<double>& colWidths, const PdfFont& font)
{
    auto fontMetrics = font.GetFontMetrics();
    double fontSize = fontMetrics->GetFontSize();

    std::vector<double> rowHeights;
    for (size_t row = 0; row < rowCount; ++row)
    {
        size_t lineCount = 0;
        for (size_t col = 0; col < colWidths.size(); ++col)
        {
            lineCount = std::max(lineCount, size_t(fontMetrics->StringWidth(model.GetText(col, row)) / colWidths[col]));
        }

        rowHeights.push_back(fontSize * (lineCount + 2));
    }

    return rowHeights;
}

std::vector<double> GetColumnWidths(std::initializer_list<double> widths, double indent)
{
    auto zeroCount = 0;
    double sumWidth = 0;

    for (auto width : widths)
    {        
        sumWidth += width;
        if (width == 0.0) 
            ++zeroCount;
    }

    auto zeroWidth = zeroCount ? (A4_WIDTH - sumWidth - 2 * indent) / zeroCount: 0;

    std::vector<double> result;
    for (auto width : widths)
    {
        result.push_back((width == 0.0) ? zeroWidth: width);
    }

    return result;
}

const PdfFont* CreateFont(PdfStreamedDocument& pdf, const PdfString& fontName, double fontSize)
{
    auto font = pdf.CreateFont(fontName.GetString());
    if (!font)
    {
        PODOFO_RAISE_ERROR(ePdfError_InvalidHandle);
    }

    font->SetFontSize(float(fontSize));
    return font;
}

} //namespace PoDoFo