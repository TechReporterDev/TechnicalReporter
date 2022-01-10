#pragma once
#include <libxslt\xslt.h>
#include <libxslt\xsltInternals.h>
#include <libxslt\transform.h>
#include <libxslt\xsltutils.h>
#include <libexslt\exslt.h>
#include "STLTools\stl_tools.h"
#include "libxml2_wrap.h"

namespace xml_tools {

struct XsltStylesheet: xsltStylesheet
{
    XsltStylesheet() = delete;
    XsltStylesheet(const XmlDoc&) = delete;
    XsltStylesheet(XsltStylesheet&&) = delete;
    XsltStylesheet& operator = (const XsltStylesheet&) = delete;
    XsltStylesheet& operator = (XsltStylesheet&&) = delete;

    static void operator delete(void* ptr, std::size_t size)
    {
        xsltFreeStylesheet((XsltStylesheet*)ptr);
    }
};

std::unique_ptr<XsltStylesheet>     xslt_parse_stylesheet_doc(std::unique_ptr<XmlDoc> doc);
std::unique_ptr<XmlDoc>             xslt_apply_stylesheet(XsltStylesheet& stylesheet, const XmlDoc& doc);

} //namespace xml_tools {