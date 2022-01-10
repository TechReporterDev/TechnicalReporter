// All of "Content" types are immutable and can be used concurrenty without any restrictions
#pragma once
#include "Exception.h"
#include "TrivialTypes.h"
#include "TRXML\TRXMLFwd.h"
#include "STLTools\lazy.h"
#include <string>
#include <memory>

namespace TR { namespace Core {
class Diff;

class Content
{
public:
    virtual ~Content(){}
    
    virtual Blob                                as_blob() const = 0;    
    virtual bool                                is_equal(const Content& right) const = 0;       
    virtual std::unique_ptr<Diff>               make_diff(const Content& next) const = 0;   
};

class BinaryContent : public Content
{
public:
    BinaryContent(Blob blob);

    //Content override
    virtual Blob                                as_blob() const override;
    virtual bool                                is_equal(const Content& right) const override;
    virtual std::unique_ptr<Diff>               make_diff(const Content& next) const override;

private:
    Blob m_blob;
};

class TextContent: public Content
{
public:
    using XmlDoc = XML::XmlDoc;

    //Content override
    virtual Blob                                as_blob() const override;
    virtual bool                                is_equal(const Content& right) const override;
    virtual std::unique_ptr<Diff>               make_diff(const Content& next) const override;

    //TextContent methods
    virtual std::shared_ptr<const XmlDoc>       get_keywords() const;
    virtual const std::string&                  as_string() const = 0;
    virtual std::wstring                        as_wstring() const;
};

class XmlContent: public TextContent
{
public:
    using XmlDoc = XML::XmlDoc;
    
    //Content override
    virtual std::unique_ptr<Diff>               make_diff(const Content& next) const override;

    //XmlContent methods
    virtual const XmlDoc&                       as_xml_doc() const = 0;
};

class RegularContent: public XmlContent
{
public:
    using XmlRegularDoc = XML::XmlRegularDoc;
    using XmlDefDoc = XML::XmlDefDoc;
    
    //Content override
    virtual std::unique_ptr<Diff>               make_diff(const Content& next) const override;

    //XmlContent override
    virtual const XmlDoc&                       as_xml_doc() const override;

    //RegularContent methods
    virtual const XmlRegularDoc&                as_regular_doc() const = 0;
    virtual std::shared_ptr<const XmlDefDoc>    get_def_doc() const = 0;
};

std::string make_text(const Blob& blob);

////////////
// Implementation
////////////

class PlainTextContent: public TextContent
{
public:
    PlainTextContent(std::string text, std::shared_ptr<const XmlDoc> keywords);

    // TextContent override
    virtual std::shared_ptr<const XmlDoc>       get_keywords() const override;
    virtual const std::string&                  as_string() const override;

protected:
    std::string m_text;
    std::shared_ptr<const XmlDoc> m_keywords;
};


class PlainXmlContent: public XmlContent
{
public:     
    PlainXmlContent(std::string text);
    PlainXmlContent(std::unique_ptr<XmlDoc> doc);

    // TextContent override
    virtual const std::string&                  as_string() const override;

    // XmlContent override
    virtual const XmlDoc&                       as_xml_doc() const override;

private:
    stl_tools::lazy_unique_ptr<XmlDoc> m_doc;
    stl_tools::lazy<std::string> m_text;
};

class GeneralRegularContent: public RegularContent
{
public:
    GeneralRegularContent(std::string text, std::shared_ptr<const XmlDefDoc> def_doc);
    GeneralRegularContent(std::unique_ptr<XmlRegularDoc> doc, std::shared_ptr<const XmlDefDoc> def_doc);

    // TextContent override
    virtual const std::string&                  as_string() const override;

    // RegularContent override
    virtual const XmlRegularDoc&                as_regular_doc() const override;
    virtual std::shared_ptr<const XmlDefDoc>    get_def_doc() const override;

protected:
    stl_tools::lazy_unique_ptr<XmlRegularDoc> m_regular_doc;
    stl_tools::lazy<std::string> m_text;
    std::shared_ptr<const XmlDefDoc> m_def_doc;
};

class CheckListContent: public RegularContent
{
public:
    using XmlComplianceDoc = XML::XmlComplianceDoc;
    using XmlComplianceDef = XML::XmlComplianceDef;

    CheckListContent(std::string text, std::shared_ptr<const XmlComplianceDef> compliance_def);
    CheckListContent(std::unique_ptr<XmlComplianceDoc> doc, std::shared_ptr<const XmlComplianceDef> compliance_def);

    // TextContent override
    virtual const std::string&                  as_string() const override;

    // RegularContent override
    virtual const XmlRegularDoc&                as_regular_doc() const override;
    virtual std::shared_ptr<const XmlDefDoc>    get_def_doc() const override;

    // ComplianceContent methods
    const XmlComplianceDoc&                     get_compliance_doc() const;
    std::shared_ptr<const XmlComplianceDef>     get_compliance_def() const;

private:
    stl_tools::lazy_unique_ptr<XmlComplianceDoc> m_compliance_doc;
    stl_tools::lazy<std::string> m_text;
    std::shared_ptr<const XmlComplianceDef> m_compliance_def;
};

class ComplianceContent: public CheckListContent
{
public:
    ComplianceContent(std::string text, std::shared_ptr<const XmlComplianceDef> compliance_def);
    ComplianceContent(std::unique_ptr<XmlComplianceDoc> doc, std::shared_ptr<const XmlComplianceDef> compliance_def);
};

class ValidatedContent: public GeneralRegularContent
{
public:
    ValidatedContent(std::string text, std::shared_ptr<const XmlDefDoc> def_doc);
    ValidatedContent(std::unique_ptr<XmlRegularDoc> doc, std::shared_ptr<const XmlDefDoc> def_doc);
};

}} //namespace TR { namespace Core {