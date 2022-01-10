#include "stdafx.h"
#include "Packing.h"
namespace TR { namespace Core {

std::string Packing<std::wstring>::pack(const std::wstring& wstr)
{
    return stl_tools::ucs_to_utf8(wstr);
};

std::wstring Unpacking<std::string>::unpack(const std::string& str)
{
    return stl_tools::utf8_to_ucs(str);
}

boost::optional<XmlQueryDocText> Packing<XmlQueryDoc*>::pack(const XmlQueryDoc* query_doc)
{
    return query_doc ? boost::make_optional(XmlQueryDocText(as_string(*query_doc))) : boost::none;
}

std::unique_ptr<XmlQueryDoc> Unpacking<boost::optional<XmlQueryDocText>>::unpack(const boost::optional<XmlQueryDocText>& query_xml, const XmlDefDoc& def_doc)
{
    if (!query_xml)
    {
        return nullptr;
    }
    return XML::parse_query(*query_xml, def_doc);
}

XmlQueryDocText Packing<XmlQueryDoc>::pack(const XmlQueryDoc& query_doc)
{
    return XmlQueryDocText(as_string(query_doc));
}

std::unique_ptr<XmlQueryDoc> Unpacking<XmlQueryDocText>::unpack(const XmlQueryDocText& query_xml, const XmlDefDoc& def_doc)
{
    return XML::parse_query(query_xml, def_doc);
}

boost::optional<XmlDefDocText> Packing<XmlDefDoc*>::pack(const XmlDefDoc* def_doc)
{
    return def_doc ? boost::make_optional(XmlDefDocText(as_string(*def_doc))) : boost::none;
}

std::unique_ptr<XmlDefDoc> Unpacking<boost::optional<XmlDefDocText>>::unpack(const boost::optional<XmlDefDocText>& def_xml)
{
    if (!def_xml)
    {
        return nullptr;
    }
    return XML::parse_definition(*def_xml);
}

XmlDefDocText Packing<XmlDefDoc>::pack(const XmlDefDoc& def_doc)
{
    return XmlDefDocText(as_string(def_doc));
}

std::unique_ptr<XmlDefDoc> Unpacking<XmlDefDocText>::unpack(const XmlDefDocText& def_xml)
{
    return XML::parse_definition(def_xml);
}

}} //namespace TR { namespace Core {