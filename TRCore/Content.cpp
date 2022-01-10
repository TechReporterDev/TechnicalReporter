#include "stdafx.h"
#include "Content.h"
#include "Diff.h"
namespace TR { namespace Core {
using XML::XmlDefDoc;
using XML::XmlDoc;

BinaryContent::BinaryContent(Blob blob) :
    m_blob(std::move(blob))
{
}

Blob BinaryContent::as_blob() const
{
    return m_blob;
}

bool BinaryContent::is_equal(const Content& right) const
{
    if (typeid(*this) != typeid(right))
    {
        return false;
    }
    return m_blob == static_cast<const BinaryContent&>(right).m_blob;
}

std::unique_ptr<Diff> BinaryContent::make_diff(const Content& next) const
{
    return nullptr;
}

Blob TextContent::as_blob() const
{
    auto& text = as_string();
    return Blob(text.begin(), text.end());
}

bool TextContent::is_equal(const Content& right) const
{
    if (typeid(*this) != typeid(right))
    {
        return false;
    }
    return as_string() == static_cast<const TextContent&>(right).as_string();
}

std::unique_ptr<Diff> TextContent::make_diff(const Content& next) const
{
    if (typeid(*this) != typeid(next))
    {
        return nullptr;
    }

    auto& text_content = static_cast<const TextContent&>(next);
    return std::make_unique<PlainTextDiff>(Core::make_diff(as_wstring(), text_content.as_wstring()));
}

std::shared_ptr<const XmlDoc> TextContent::get_keywords() const
{
    return nullptr;
}

std::wstring TextContent::as_wstring() const
{
    return stl_tools::utf8_to_ucs(as_string());
}

std::unique_ptr<Diff> XmlContent::make_diff(const Content& next) const
{
    if (typeid(*this) != typeid(next))
    {
        return nullptr;
    }

    auto& xml_content = static_cast<const XmlContent&>(next);
    return std::make_unique<PlainXmlDiff>(Core::make_diff(as_xml_doc(), xml_content.as_xml_doc(), "@id"));
}

std::unique_ptr<Diff> RegularContent::make_diff(const Content& next) const
{
    if (typeid(*this) != typeid(next))
    {
        return nullptr;
    }

    auto& regular_content = dynamic_cast<const RegularContent&>(next);
    return std::make_unique<RegularDiff>(
        Core::make_diff(as_regular_doc(), regular_content.as_regular_doc(), "@id"),
        get_def_doc());
}

const XmlDoc& RegularContent::as_xml_doc() const
{
    return as_regular_doc();
}

PlainTextContent::PlainTextContent(std::string text, std::shared_ptr<const XmlDoc> keywords):
    m_text(std::move(text)),
    m_keywords(keywords)
{
}

std::shared_ptr<const XmlDoc> PlainTextContent::get_keywords() const
{
    return m_keywords;
}

const std::string& PlainTextContent::as_string() const
{
    return m_text;
}

PlainXmlContent::PlainXmlContent(std::string text):
    m_doc([this]{return xml_tools::parse(m_text);}),
    m_text(std::move(text))
{
}

PlainXmlContent::PlainXmlContent(std::unique_ptr<XmlDoc> doc):
    m_doc(std::move(doc)),
    m_text([this]{return xml_tools::as_string(*m_doc); })
{
}

const std::string& PlainXmlContent::as_string() const
{
    return m_text.get();
}

const XmlDoc& PlainXmlContent::as_xml_doc() const
{   
    return *m_doc;
}

GeneralRegularContent::GeneralRegularContent(std::string text, std::shared_ptr<const XmlDefDoc> def_doc):
    m_regular_doc([this]{return parse_regular(m_text, *m_def_doc); }),
    m_text(std::move(text)),
    m_def_doc(def_doc)
{
}

GeneralRegularContent::GeneralRegularContent(std::unique_ptr<XmlRegularDoc> regular_doc, std::shared_ptr<const XmlDefDoc> def_doc):
    m_regular_doc(std::move(regular_doc)),
    m_text([this]{return xml_tools::as_string(*m_regular_doc); }),
    m_def_doc(def_doc)
{
    _ASSERT(&m_regular_doc->get_bound_def() == def_doc.get());
}

const std::string& GeneralRegularContent::as_string() const
{
    return m_text.get();
}

const XmlRegularDoc& GeneralRegularContent::as_regular_doc() const
{
    return *m_regular_doc;
}

std::shared_ptr<const XmlDefDoc> GeneralRegularContent::get_def_doc() const
{
    return m_def_doc;
}

CheckListContent::CheckListContent(std::string text, std::shared_ptr<const XmlComplianceDef> compliance_def):
    m_compliance_doc([this]{return parse_compliance_doc(m_text, *m_compliance_def); }),
    m_text(std::move(text)),
    m_compliance_def(compliance_def)
{
}

CheckListContent::CheckListContent(std::unique_ptr<XmlComplianceDoc> compliance_doc, std::shared_ptr<const XmlComplianceDef> compliance_def):
    m_compliance_doc(std::move(compliance_doc)),
    m_text([this]{return xml_tools::as_string(*m_compliance_doc); }),
    m_compliance_def(compliance_def)
{
    _ASSERT(&m_compliance_doc->get_bound_def() == m_compliance_def.get());
}

const std::string& CheckListContent::as_string() const
{
    return m_text.get();
}

const XmlRegularDoc& CheckListContent::as_regular_doc() const
{
    return *m_compliance_doc;
}

std::shared_ptr<const XmlDefDoc> CheckListContent::get_def_doc() const
{
    return m_compliance_def;
}

const XmlComplianceDoc& CheckListContent::get_compliance_doc() const
{
    return *m_compliance_doc;
}

std::shared_ptr<const XmlComplianceDef> CheckListContent::get_compliance_def() const
{
    return m_compliance_def;
}

ComplianceContent::ComplianceContent(std::string text, std::shared_ptr<const XmlComplianceDef> compliance_def):
    CheckListContent(std::move(text), compliance_def)
{
}

ComplianceContent::ComplianceContent(std::unique_ptr<XmlComplianceDoc> doc, std::shared_ptr<const XmlComplianceDef> compliance_def):
    CheckListContent(std::move(doc), compliance_def)
{
}

ValidatedContent::ValidatedContent(std::string text, std::shared_ptr<const XmlDefDoc> def_doc):
    GeneralRegularContent(std::move(text), def_doc)
{
}

ValidatedContent::ValidatedContent(std::unique_ptr<XmlRegularDoc> doc, std::shared_ptr<const XmlDefDoc> def_doc):
    GeneralRegularContent(std::move(doc), def_doc)
{
}

std::string make_text(const Blob& blob)
{
    std::string text;
    text.reserve(blob.size());
    for (auto ch : blob)
    {
        if (ch != '\r')
        {
            text.push_back(ch);
        }
    }
    return text;
}

}} //namespace TR { namespace Core {