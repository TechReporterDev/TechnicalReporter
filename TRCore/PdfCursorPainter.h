#pragma once
#include "stddecl.h"
#include <podofo/podofo.h>

namespace PoDoFo {

const double A4_HEIGHT = 842.0;
const double A4_WIDTH = 595.0;
const double TOP_MARGIN = 50.0;
const double BOTTOM_MARGIN = 50.0;

class PdfCursorPainter
{
public:
    PdfCursorPainter(PoDoFo::PdfStreamedDocument& pdf);
    ~PdfCursorPainter();

    void SetFont(const PdfString& fontName, double fontSize);
    const PdfFont* GetFont() const;
    void DrawText(double x, const PdfString& text, bool moveCursor = true);
    void DrawTable(double x, PdfTable& table, bool moveCursor = true);
    void ReserveSpace(double space);
    void MoveCursor(double offset);

private:
    PdfStreamedDocument& m_pdf;
    PdfPainter m_painter;
    double m_cursor;
};

std::vector<double> GetRowHeights(const PdfTableModel& model, const size_t rowCount, const std::vector<double>& colWidths, const PdfFont& font);
std::vector<double> GetColumnWidths(std::initializer_list<double> widths, double indent);
const PdfFont* CreateFont(PdfStreamedDocument& pdf, const PdfString& fontName, double fontSize);



} //namespace PoDoFo {