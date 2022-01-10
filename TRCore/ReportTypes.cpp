#include "stdafx.h"
#include "ReportTypes.h"
#include "Transaction.h"
#include "Content.h"
#include "Diff.h"
namespace TR { namespace Core {

ReportType::ReportType(UUID uuid, std::wstring name, Traits traits, int version):
    m_uuid(uuid),
    m_name(std::move(name)),
    m_version(version),
    m_traits(traits)
{
}

ReportTypeUUID ReportType::get_uuid() const
{
    return m_uuid;
}

std::wstring ReportType::get_name() const
{
    return m_name;
}

std::unique_ptr<Content> ReportType::make_content(Blob blob) const
{
    return prepare_make_content_from_blob()(std::move(blob));
}

ReportType::Traits ReportType::get_traits() const
{
    return m_traits;
}

int ReportType::get_version() const
{
    return m_version;
}

ReportTypeRef ReportType::get_ref() const
{
    if (!m_ref)
    {
        throw Exception(L"ReportType is not registred.");
    }
    return *m_ref;
}

ReportType::operator ReportTypeRef() const
{
    return get_ref();
}

TextReportType::TextReportType(UUID uuid, std::wstring name, Traits traits, int version):
    ReportType(uuid, name, traits, version)
{
}

std::unique_ptr<Content> TextReportType::make_empty_content() const
{
    return prepare_make_content_from_string()("");
}

std::unique_ptr<Diff> TextReportType::make_diff(Blob blob) const
{
    return std::make_unique<PlainTextDiff>(std::move(blob));
}

ReportType::MakeContentFrom<Blob> TextReportType::prepare_make_content_from_blob() const
{
    auto make_content = prepare_make_content_from_string();
    return[make_content](Blob blob){
        return make_content(make_text(blob));
    };
}

XmlReportType::XmlReportType(UUID uuid, std::wstring name, Traits traits, int version):
    TextReportType(uuid, name, traits, version)
{
}

std::unique_ptr<Content> XmlReportType::make_empty_content() const
{
    char* empty_xml =
        "<?xml version=\"1.0\"?>"
        "<root>"
        "</root>";

    return prepare_make_content_from_string()(empty_xml);
}

std::unique_ptr<Diff> XmlReportType::make_diff(Blob blob) const
{
    return std::make_unique<PlainXmlDiff>(std::move(blob));
}

RegularReportType::RegularReportType(UUID uuid, std::wstring name, Traits traits, int version):
    XmlReportType(uuid, name, traits, version)
{
}

std::unique_ptr<Content> RegularReportType::make_empty_content() const
{
    return prepare_make_content_from_regular()(XML::create_regular(*get_def_doc()));
}

std::unique_ptr<Diff> RegularReportType::make_diff(Blob blob) const
{
    return std::make_unique<RegularDiff>(std::move(blob), get_def_doc());
}

XmlReportType::MakeContentFromXmlDoc RegularReportType::prepare_make_content_from_xml() const
{
    auto def_doc = get_def_doc();
    auto make_content = prepare_make_content_from_regular();
    return [def_doc, make_content](std::unique_ptr<XmlDoc> doc){
        bind_definition(*doc, *def_doc);
        return make_content(stl_tools::static_pointer_cast<XmlRegularDoc>(std::move(doc)));
    };
}

struct ReportTypeUUIDIndex: stl_tools::unique_storage_index<ReportTypeUUID>
{
    static ReportTypeUUID get_key(const std::unique_ptr<ReportType>& report_type)
    {
        return report_type->get_uuid();
    }   
};

class ReportTypesStorage: public stl_tools::storage<std::unique_ptr<ReportType>, ReportTypeUUIDIndex>
{
};

ReportTypes::ReportTypes():
    m_db(nullptr),
    m_storage(std::make_unique<ReportTypesStorage>())
{
}

ReportTypes::~ReportTypes()
{
}

void ReportTypes::set_database(Database* db)
{
    _ASSERT(!m_db);
    m_db = db;
}

const ReportType& ReportTypes::add_report_type(std::unique_ptr<ReportType> report_type)
{
    Transaction t(*m_db);
    auto& added_report_type = add_report_type(std::move(report_type), t);
    t.commit();
    return added_report_type;
}

const ReportType& ReportTypes::add_report_type(std::unique_ptr<ReportType> report_type, Transaction& t)
{
    auto uuid = report_type->get_uuid();
    auto position = transact_insert(*m_storage, std::move(report_type), t);
    auto& added_report_type = **position;
    added_report_type.m_ref = ReportTypeRef(this, uuid);
    m_add_report_type_sig(added_report_type.get_ref(), t);
    return added_report_type;
}

void ReportTypes::remove_report_type(ReportTypeUUID uuid)
{
    Transaction t(*m_db);
    remove_report_type(uuid, t);
    t.commit();
}

void ReportTypes::remove_report_type(ReportTypeUUID uuid, Transaction& t)
{
    auto found = m_storage->find(uuid);
    if (found == m_storage->end())
    {
        throw Exception(L"Report type not found");
    }
    transact_erase(*m_storage, found, t);
    m_remove_report_type_sig(uuid, t);
}

bool ReportTypes::has_report_type(ReportTypeUUID uuid) const
{
    return m_storage->find(uuid) != m_storage->end();
}

const ReportType& ReportTypes::get_report_type(ReportTypeUUID uuid) const
{
    auto found = m_storage->find(uuid);
    if (found == m_storage->end())
    {
        throw Exception(L"Report type not found");
    }
    return **found;
}

ReportTypes::ReportTypeRange ReportTypes::get_report_types() const
{
    return *m_storage | boost::adaptors::indirected;
}

PlainTextReportType::PlainTextReportType(UUID uuid, std::wstring name, Traits traits, int version):
    TextReportType(uuid, name, traits, version)
{
}

TextReportType::MakeContentFromString PlainTextReportType::prepare_make_content_from_string() const
{
    return [](std::string str){
        return std::make_unique<PlainTextContent>(std::move(str), nullptr);
    };
}

PlainXmlReportType::PlainXmlReportType(UUID uuid, std::wstring name, Traits traits, int version):
    XmlReportType(uuid, name, traits, version)
{
}

TextReportType::MakeContentFromString PlainXmlReportType::prepare_make_content_from_string() const
{
    return [](std::string str){
        return std::make_unique<PlainXmlContent>(std::move(str));
    };
}

XmlReportType::MakeContentFromXmlDoc PlainXmlReportType::prepare_make_content_from_xml() const
{
    return [](std::unique_ptr<XmlDoc> doc){
        return std::make_unique<PlainXmlContent>(std::move(doc));
    };
};

GeneralRegularReportType::GeneralRegularReportType(UUID uuid, std::wstring name, stl_tools::lazy_unique_ptr<XmlDefDoc> def_doc, Traits traits, int version):
    RegularReportType(uuid, name, traits, version),
    m_def_doc(std::move(def_doc))
{
}

TextReportType::MakeContentFromString GeneralRegularReportType::prepare_make_content_from_string() const
{
    auto def_doc = m_def_doc.share();
    return [def_doc](std::string str){
        return std::make_unique<GeneralRegularContent>(std::move(str), def_doc);
    };
}

std::shared_ptr<const XmlDefDoc> GeneralRegularReportType::get_def_doc() const
{
    return m_def_doc.share();
}

RegularReportType::MakeContentFromRegularDoc GeneralRegularReportType::prepare_make_content_from_regular() const
{
    auto def_doc = m_def_doc.share();
    return [def_doc](std::unique_ptr<XmlRegularDoc> reg_doc){
        if (&reg_doc->get_bound_def() != def_doc.get())
        {
            throw Exception(L"Regular document definition does not match report type definition");
        }
        return std::make_unique<GeneralRegularContent>(std::move(reg_doc), def_doc);
    };
}

CheckListReportType::CheckListReportType(UUID uuid, std::wstring name, stl_tools::lazy_unique_ptr<XmlComplianceDef> compliance_def, Traits traits, int version):
    RegularReportType(uuid, std::move(name), traits, version),
    m_compliance_def(std::move(compliance_def))
{
}

TextReportType::MakeContentFromString CheckListReportType::prepare_make_content_from_string() const
{
    auto compliance_def = m_compliance_def.share();
    return [compliance_def](std::string str){
        return std::make_unique<CheckListContent>(std::move(str), compliance_def);
    };
}

std::shared_ptr<const XmlDefDoc> CheckListReportType::get_def_doc() const
{
    return m_compliance_def.share();
}

RegularReportType::MakeContentFromRegularDoc CheckListReportType::prepare_make_content_from_regular() const
{
    auto compliance_def = m_compliance_def.share();
    return [compliance_def](std::unique_ptr<XmlRegularDoc> reg_doc) {
        if (&reg_doc->get_bound_def() != compliance_def.get())
        {
            throw Exception(L"Regular document definition does not match report type definition");
        }
        return std::make_unique<CheckListContent>(stl_tools::static_pointer_cast<XmlComplianceDoc>(std::move(reg_doc)), compliance_def);
    };
}

std::shared_ptr<const XmlComplianceDef> CheckListReportType::get_compliance_def() const
{
    return m_compliance_def.share();
}

ComplianceReportType::ComplianceReportType(UUID uuid, std::wstring name, stl_tools::lazy_unique_ptr<XmlComplianceDef> compliance_def, Traits traits, int version):
    RegularReportType(uuid, std::move(name), traits, version),
    m_compliance_def(std::move(compliance_def))
{
}

TextReportType::MakeContentFromString ComplianceReportType::prepare_make_content_from_string() const
{
    auto compliance_def = m_compliance_def.share();
    return [compliance_def](std::string str){
        return std::make_unique<ComplianceContent>(std::move(str), compliance_def);
    };
}


std::shared_ptr<const XmlDefDoc> ComplianceReportType::get_def_doc() const
{
    return m_compliance_def.share();
}

RegularReportType::MakeContentFromRegularDoc ComplianceReportType::prepare_make_content_from_regular() const
{
    auto compliance_def = m_compliance_def.share();
    return [compliance_def](std::unique_ptr<XmlRegularDoc> reg_doc) {
        if (&reg_doc->get_bound_def() != compliance_def.get())
        {
            throw Exception(L"Regular document definition does not match report type definition");
        }
        return std::make_unique<ComplianceContent>(stl_tools::static_pointer_cast<XmlComplianceDoc>(std::move(reg_doc)), compliance_def);
    };
}

std::shared_ptr<const XmlComplianceDef> ComplianceReportType::get_compliance_def() const
{
    return m_compliance_def.share();
}

ValidatedReportType::ValidatedReportType(UUID uuid, std::wstring name, stl_tools::lazy_unique_ptr<XmlDefDoc> def_doc, Traits traits, int version):
    GeneralRegularReportType(uuid, std::move(name), std::move(def_doc), traits, version)
{
}

TextReportType::MakeContentFromString ValidatedReportType::prepare_make_content_from_string() const
{
    auto def_doc = m_def_doc.share();
    return [def_doc](std::string str){
        return std::make_unique<ValidatedContent>(std::move(str), def_doc);
    };
}

RegularReportType::MakeContentFromRegularDoc ValidatedReportType::prepare_make_content_from_regular() const
{
    auto def_doc = m_def_doc.share();
    return [def_doc](std::unique_ptr<XmlRegularDoc> reg_doc){
        if (&reg_doc->get_bound_def() != def_doc.get())
        {
            throw Exception(L"Regular document definition does not match report type definition");
        }
        return std::make_unique<ValidatedContent>(std::move(reg_doc), def_doc);
    };
}

}} //namespace TR { namespace Core {