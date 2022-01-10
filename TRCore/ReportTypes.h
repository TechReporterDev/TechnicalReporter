#pragma once
#include "BackgndFwd.h"
#include "BasisFwd.h"
#include "BasisRefs.h"
#include "Content.h"
#include "Exception.h"
#include "stddecl.h"
namespace TR { namespace Core {
class Content;
class TextContent;
class ReportTypesStorage;

class ReportType
{
public:
    friend class ReportTypes;

    enum class Trait {VIEW = 0x01};
    using Traits = stl_tools::flag_type<Trait>;
    template<class Arg> using MakeContentFrom = std::function<std::unique_ptr<Content>(Arg)>;
    
    ReportType(UUID uuid, std::wstring name, Traits traits, int version);
    virtual ~ReportType(){}

    ReportTypeUUID                      get_uuid() const;
    std::wstring                        get_name() const;
    Traits                              get_traits() const;
    int                                 get_version() const;    

    ReportTypeRef                       get_ref() const;
    operator ReportTypeRef() const;

    // override
    virtual std::unique_ptr<Content>    make_content(Blob blob) const;
    virtual std::unique_ptr<Content>    make_empty_content() const = 0;
    virtual std::unique_ptr<Diff>       make_diff(Blob blob) const = 0;
    virtual MakeContentFrom<Blob>       prepare_make_content_from_blob() const = 0;

protected:
    ReportTypeUUID m_uuid;
    std::wstring m_name;
    int m_version;      
    Traits m_traits;

    boost::optional<ReportTypeRef> m_ref;
};

class TextReportType: public ReportType
{
public:
    using MakeContentFromString = std::function<std::unique_ptr<TextContent>(std::string)>;
    TextReportType(UUID uuid, std::wstring name, Traits traits, int version);

    // ReportType override
    virtual std::unique_ptr<Content>    make_empty_content() const override;
    virtual std::unique_ptr<Diff>       make_diff(Blob blob) const override;
    virtual MakeContentFrom<Blob>       prepare_make_content_from_blob() const override;

    // TextReportType methods
    virtual MakeContentFromString       prepare_make_content_from_string() const = 0;
};

class XmlReportType: public TextReportType
{
public:
    using MakeContentFromXmlDoc = std::function<std::unique_ptr<XmlContent>(std::unique_ptr<XmlDoc>)>;
    XmlReportType(UUID uuid, std::wstring name, Traits traits, int version);

    // ReportType override
    virtual std::unique_ptr<Content>    make_empty_content() const override;
    virtual std::unique_ptr<Diff>       make_diff(Blob blob) const override;
    
    // XmlReportType methods
    virtual MakeContentFromXmlDoc       prepare_make_content_from_xml() const = 0;
};

class RegularReportType: public XmlReportType
{
public:
    using MakeContentFromRegularDoc = std::function<std::unique_ptr<RegularContent>(std::unique_ptr<XmlRegularDoc>)>;
    RegularReportType(UUID uuid, std::wstring name, Traits traits, int version);

    // ReportType override
    virtual std::unique_ptr<Content>            make_empty_content() const override;
    virtual std::unique_ptr<Diff>               make_diff(Blob blob) const override;

    // XmlReportType methods
    virtual MakeContentFromXmlDoc               prepare_make_content_from_xml() const override;

    // RegularReportType methods
    virtual std::shared_ptr<const XmlDefDoc>    get_def_doc() const = 0;
    virtual MakeContentFromRegularDoc           prepare_make_content_from_regular() const = 0;
};

class ReportTypes
{
public:
    static const Priority REPORT_TYPES_PRIORITY = 100;  
    using ReportTypeRange = boost::any_range<const ReportType, boost::forward_traversal_tag>;

    ReportTypes();
    ~ReportTypes();
    void                    set_database(Database* db);

    const ReportType&       add_report_type(std::unique_ptr<ReportType> report_type);
    const ReportType&       add_report_type(std::unique_ptr<ReportType> report_type, Transaction& t);
    void                    remove_report_type(ReportTypeUUID uuid);
    void                    remove_report_type(ReportTypeUUID uuid, Transaction& t);
    bool                    has_report_type(ReportTypeUUID uuid) const;
    const ReportType&       get_report_type(ReportTypeUUID uuid) const;
    ReportTypeRange         get_report_types() const;

    template<class T>
    void connect_add_report_type(T slot, Priority priority) { m_add_report_type_sig.connect(slot, priority); }

    template<class T>
    void connect_remove_report_type(T slot, Priority priority) { m_remove_report_type_sig.connect(slot, priority); }

private:
    Database* m_db;
    std::unique_ptr<ReportTypesStorage> m_storage;

    OrderedSignal<void(ReportTypeRef, Transaction&)> m_add_report_type_sig;
    OrderedSignal<void(ReportTypeUUID, Transaction&)> m_remove_report_type_sig;
};

////////////////
// Implementation
////////////////

class PlainTextReportType: public TextReportType
{
public:
    PlainTextReportType(UUID uuid, std::wstring name, Traits traits = Traits(), int version = 0);

    // TextReportType override
    virtual MakeContentFromString   prepare_make_content_from_string() const override;  
};

class PlainXmlReportType: public XmlReportType
{
public:
    PlainXmlReportType(UUID uuid, std::wstring name, Traits traits = Traits(), int version = 0);

    // TextReportType override
    virtual MakeContentFromString   prepare_make_content_from_string() const override;

    // XmlReportType override
    virtual MakeContentFromXmlDoc   prepare_make_content_from_xml() const override;
};

class GeneralRegularReportType: public RegularReportType
{
public:
    GeneralRegularReportType(UUID uuid, std::wstring name, stl_tools::lazy_unique_ptr<XmlDefDoc> def_doc, Traits traits = Traits(), int version = 0);

    // TextReportType override
    virtual MakeContentFromString               prepare_make_content_from_string() const override;

    // RegularReportType override
    virtual std::shared_ptr<const XmlDefDoc>    get_def_doc() const override;
    virtual MakeContentFromRegularDoc           prepare_make_content_from_regular() const override;

protected:
    stl_tools::lazy_shared_ptr<const XmlDefDoc> m_def_doc;
};

class CheckListReportType: public RegularReportType
{
public:
    CheckListReportType(UUID uuid, std::wstring name, stl_tools::lazy_unique_ptr<XmlComplianceDef> compliance_def, Traits traits = Traits(), int version = 0);

    // TextReportType override
    virtual MakeContentFromString               prepare_make_content_from_string() const override;

    // RegularReportType override
    virtual std::shared_ptr<const XmlDefDoc>    get_def_doc() const override;
    virtual MakeContentFromRegularDoc           prepare_make_content_from_regular() const override;

    // ComplianceReportType methods
    std::shared_ptr<const XmlComplianceDef>     get_compliance_def() const;

private:
    stl_tools::lazy_shared_ptr<XmlComplianceDef> m_compliance_def;
};

class ComplianceReportType: public RegularReportType
{
public:
    ComplianceReportType(UUID uuid, std::wstring name, stl_tools::lazy_unique_ptr<XmlComplianceDef> compliance_def, Traits traits = Trait::VIEW, int version = 0);
    
    // TextReportType override  
    virtual MakeContentFromString               prepare_make_content_from_string() const override;

    // RegularReportType override
    virtual std::shared_ptr<const XmlDefDoc>    get_def_doc() const override;
    virtual MakeContentFromRegularDoc           prepare_make_content_from_regular() const override;

    // ComplianceReportType methods
    std::shared_ptr<const XmlComplianceDef>     get_compliance_def() const;

private:
    stl_tools::lazy_shared_ptr<XmlComplianceDef> m_compliance_def;
};

class ValidatedReportType: public GeneralRegularReportType
{
public:
    ValidatedReportType(UUID uuid, std::wstring name, stl_tools::lazy_unique_ptr<XmlDefDoc> def_doc, Traits traits = Trait::VIEW, int version = 0);

    // TextReportType override
    virtual MakeContentFromString       prepare_make_content_from_string() const override;

    // RegularReportType override
    virtual MakeContentFromRegularDoc   prepare_make_content_from_regular() const override;
};

}} //namespace TR { namespace Core {