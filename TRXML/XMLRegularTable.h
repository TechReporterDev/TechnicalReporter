#pragma once
#include "XMLRegular.h"
#include "TRXMLStd.h"
namespace TR { namespace XML {

struct XMLRegularTable
{
    using Header = std::vector<const XmlDefNode*>;
    using Row = std::vector<XmlRegularNode*>;
    using Rows = std::vector<Row>;

    XMLRegularTable(std::vector<const XmlDefNode*> header, std::vector<Row> rows);

    XMLRegularTable(const XMLRegularTable& regular_table) = default;
    XMLRegularTable& operator = (const XMLRegularTable& regular_table) = default;

    XMLRegularTable(XMLRegularTable&& regular_table);// = default
    XMLRegularTable& operator = (XMLRegularTable&& regular_table); // = default

    Header      m_header;
    Rows        m_rows;
};

XMLRegularTable make_regular_table(XmlRegularDoc& regular_doc);

}} //namespace TR { namespace XML {