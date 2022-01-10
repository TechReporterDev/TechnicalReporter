#include "pch.h"
#include "xslt_tools.h"
#include <STLTools/stl_tools.h>

#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include <libxslt/xsltconfig.h>
#include <libxslt/xsltutils.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/extensions.h>
#include <libexslt/exsltexports.h>
#include <boost/regex.hpp>
#include <memory>

namespace xslt_tools {

#define XSLT_TOOLS_NAMESPACE ((const xmlChar *) "http://xslt-plugin.org/tools")

struct XmlFree
{
    void operator()(void* p)
    {
        xmlFree(p);
    }
};

template<class T>
std::unique_ptr<T, XmlFree> make_xml_ptr(T* p_t)
{
    return std::unique_ptr<T, XmlFree>(p_t);
}

std::wstring xml_xpath_pop_string(xmlXPathParserContextPtr ctxt)
{
    auto xml_string = make_xml_ptr(xmlXPathPopString(ctxt));
    if (xmlXPathCheckError(ctxt) || (xml_string == nullptr)) {
        throw std::exception("xslt tools error: failed to pop string");
    }

    return stl_tools::utf8_to_ucs(xml_string.get());
}

static void regex_match(xmlXPathParserContextPtr ctxt, int nargs)
{
    try
    {
        if (nargs != 2) {
            xmlXPathSetArityError(ctxt);
            return;
        }

        auto pattern = xml_xpath_pop_string(ctxt);
        auto input = xml_xpath_pop_string(ctxt);
        bool result = boost::regex_match(input, boost::wregex(pattern));
        xmlXPathReturnBoolean(ctxt, result);
    }
    catch (std::exception& err)
    {
        xsltTransformError(xsltXPathGetTransformContext(ctxt), NULL, NULL,
            "xslt tools error: %s", err.what());
    }
}
    
}

PLUGINPUBFUN void xslt_tools_init(void)
{
    xsltRegisterExtModuleFunction((const xmlChar*)"regex-match",
        (const xmlChar*)XSLT_TOOLS_NAMESPACE,
        xslt_tools::regex_match);
}