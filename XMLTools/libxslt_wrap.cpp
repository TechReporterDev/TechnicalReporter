#include "stdafx.h"
#include "libxslt_wrap.h"

namespace xml_tools {

std::unique_ptr<XsltStylesheet> xslt_parse_stylesheet_doc(std::unique_ptr<XmlDoc> doc)
{
    std::unique_ptr<XsltStylesheet> stylesheet((XsltStylesheet*)xsltParseStylesheetDoc(doc.get()));
    if (!stylesheet)
    {
        throw std::runtime_error("Parse XSLT stylesheet error");
    }
    doc.release();
    return stylesheet;
}

std::unique_ptr<XmlDoc> xslt_apply_stylesheet(XsltStylesheet& stylesheet, const XmlDoc& doc)
{
    std::unique_ptr<XmlDoc> result((XmlDoc*)xsltApplyStylesheet(&stylesheet, const_cast<XmlDoc*>(&doc), nullptr));
    if (!result)
    {
        throw std::runtime_error("XSLT apply stylesheet error");
    }
    return result;
}

} //namespace xml_tools {