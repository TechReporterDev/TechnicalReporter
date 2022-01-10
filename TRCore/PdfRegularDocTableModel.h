#pragma once
#include "stddecl.h"
#include <podofo/podofo.h>

using TR::XML::XmlRegularDoc;

namespace PoDoFo {

struct XmlRegularNodeTableRow;
using XmlRegularNodeTableRows = std::vector<XmlRegularNodeTableRow>;

class XmlRegularDocPdfTableModel : public PdfTableModel
{
public:
    XmlRegularDocPdfTableModel(const XmlRegularDoc& regDoc, unsigned hideRoot = false);
    ~XmlRegularDocPdfTableModel();

    // PdfTableModel implementation
    virtual PdfString GetText(int col, int row) const;
    virtual EPdfAlignment GetAlignment(int col, int row) const;
    virtual EPdfVerticalAlignment GetVerticalAlignment(int col, int row) const;
    virtual PdfFont* GetFont(int col, int row) const;
    virtual bool HasBackgroundColor(int col, int row) const;
    virtual PdfColor GetBackgroundColor(int col, int row) const;
    virtual PdfColor GetForegroundColor(int col, int row) const;
    virtual bool HasWordWrap(int col, int row) const;
    virtual bool HasBorders() const;
    virtual double GetBorderWidth() const;
    virtual PdfColor GetBorderColor(int col, int row) const;
    virtual bool HasImage(int col, int row) const;
    virtual PoDoFo::PdfImage* GetImage(int col, int row) const;

    size_t GetSize() const;

private:
    XmlRegularNodeTableRows m_rows;
    bool m_hideRoot;
};

} //namespace PoDoFo {