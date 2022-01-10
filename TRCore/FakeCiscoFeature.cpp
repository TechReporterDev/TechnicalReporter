#include "stdafx.h"
#include "FakeCiscoFeature.h"
#include "CoreImpl.h"
namespace TR { namespace Core {
using namespace stl_tools;
using namespace XML;

auto fake_cisco_runnig_uuid = ReportTypeUUID(gen_uuid(L"797954F2-4568-4E42-A4DE-E02CFCDB617D"));
FakeCiscoRunning::FakeCiscoRunning():
    PlainTextReportType(fake_cisco_runnig_uuid, L"FakeCiscoRunnig")
{
}

auto fake_cisco_xml_uuid = ReportTypeUUID(gen_uuid(L"CCC5C275-9AE7-424F-8C4A-A36283FE2DF5"));
FakeCiscoXmlReport::FakeCiscoXmlReport():
    PlainXmlReportType(fake_cisco_xml_uuid, L"FakeCiscoXmlReport")
{
};


std::string fake_cisco_parsed_runnig_def(
    "<?xml version=\"1.0\"?>"
    "<struct class='STRUCT' caption='struct'>"
        "<list class='LIST' caption='list'>"
            "<struct class='STRUCT' caption='struct'>"
                "<string class='STRING' caption='string'/>"
                "<integer class='INTEGER' caption='integer'/>"
                "<boolean class='BOOLEAN' caption='boolean'/>"
                "<enum class='ENUM' caption='enum' names='NAME1|NAME2|NAME3'/>"
            "</struct>"
        "</list>"
    "</struct>");

auto fake_cisco_parsed_runnig_uuid = ReportTypeUUID(gen_uuid(L"E8328BA8-4F14-4EC3-B791-FED1EE288BCF"));
FakeCiscoParsedRunning::FakeCiscoParsedRunning():
    GeneralRegularReportType(fake_cisco_parsed_runnig_uuid, L"FakeCiscoParsedRunnig", parse_definition(fake_cisco_parsed_runnig_def))
{
}

FakeCiscoRunningParser::FakeCiscoRunningParser():
    Selection(stl_tools::null_uuid())
{
}

void FakeCiscoRunningParser::set_running_ref(ReportTypeRef running_ref)
{
    _ASSERT(!m_running_ref);
    m_running_ref = running_ref;
}

void FakeCiscoRunningParser::set_parsed_running_ref(ReportTypeRef parsed_running_ref)
{
    _ASSERT(!m_parsed_running_ref);
    m_parsed_running_ref = parsed_running_ref;
}

SelectionPlan FakeCiscoRunningParser::get_plan() const
{
    _ASSERT(m_running_ref && m_parsed_running_ref);
    return {*m_running_ref, *m_parsed_running_ref};
}

FakeCiscoRunningParser::Functor FakeCiscoRunningParser::prepare_functor(SourceRef /*source_ref*/) const
{
    auto& parsed_running = dynamic_cast<const FakeCiscoParsedRunning&>(**m_parsed_running_ref);
    auto make_content = parsed_running.prepare_make_content_from_xml();
    return [make_content](const std::vector<std::shared_ptr<Content>>& inputs){
        static int counter = 0;

        std::string doc(
            "<?xml version=\"1.0\"?>"
            "<struct>"
                "<list>"
                    "<struct>"
                        "<string>"
                            "value%1%" 
                        "</string>"
                        "<integer>"
                            "%1%"
                        "</integer>"
                        "<boolean>"
                            "%2%"
                        "</boolean>"
                        "<enum>"
                            "%3%"
                        "</enum>"
                    "</struct>"
                "</list>"
            "</struct>");

        return make_content(xml_tools::parse((boost::format(doc) % ++counter % (counter%2) % (counter%3)).str()));
    };  
}

std::string fake_cisco_compliance_def(
    "<?xml version=\"1.0\"?>"
    "<compliance>"
        "<rule1 caption='Rule1' result='Result' value='Value'>"         
        "</rule1>"
        "<rule2 caption='Rule2' result='Result' value='Value'>"
        "</rule2>"
    "</compliance>");

auto fake_cisco_compliance_uuid = ReportTypeUUID(gen_uuid(L"70BBA394-48BE-4AA6-8E6F-4881BFEC59B7"));
FakeCiscoCompliance::FakeCiscoCompliance():
    CheckListReportType(fake_cisco_compliance_uuid, L"FakeCiscoCompliance", parse_compliance_def(fake_cisco_compliance_def))
{
}

std::string fake_cisco_validation_def(
    "<?xml version=\"1.0\"?>"
    "<list class='LIST' caption='list'>"
    "<trivial class='STRING' caption='string'/>"
    "</list>");

auto fake_cisco_validation_uuid = ReportTypeUUID(gen_uuid(L"16507ADE-D7F6-4D07-97D5-00C865816DBD"));
FakeCiscoValidation::FakeCiscoValidation():
    GeneralRegularReportType(fake_cisco_validation_uuid, L"FakeCiscoRegularValidation", parse_definition(fake_cisco_validation_def))
{
}

static char cisco_definition[] = {
    "<?xml version=\"1.0\"?>"
    "<settings style=\"COLLECTION\" caption=\"Settings\" description=\"Description\">"
        "<connection style=\"COLLECTION\" caption=\"Connection\" description=\"\">"
            "<login style=\"STRINGLINE\" caption=\"Login\" description=\"\">"
            "</login>"
            "<password style=\"STRINGLINE\" caption=\"Password\" description=\"\">"
            "</password>"
        "</connection>"
        "<terminal style=\"COLLECTION\" caption=\"Terminal\" description=\"\">"
            "<ssh style=\"COLLECTION\" caption=\"SSH\" description=\"\">"
                "<prompt style=\"STRINGLINE\" caption=\"Prompt\" description=\"\">"
                "</prompt>"
            "</ssh>"
            "<telnet style=\"COLLECTION\" caption=\"Telnet\" description=\"\">"
                "<prompt style=\"STRINGLINE\" caption=\"Prompt\" description=\"\">"
                "</prompt>"
            "</telnet>"

            "<addresses style=\"COLLECTION\" caption=\"Addresses\" description=\"\">"
                "<list style=\"LIST\" caption=\"List\" description=\"\">"
                    "<address style=\"COLLECTION\" caption=\"Address\" description=\"\">"
                        "<ips style=\"STRINGLINE\" caption=\"IP\" description=\"\">"
                        "</ips>"
                        "<mask style=\"STRINGLINE\" caption=\"Mask\" description=\"\">"
                        "</mask>"
                    "</address>"
                "</list>"
                "<prompt style=\"TEXTAREA\" caption=\"Prompt\" description=\"\" line_count='5'>"
                "</prompt>"
            "</addresses>"
        "</terminal>"
    "</settings>"
};

auto fake_cisco_uuid = gen_uuid(L"29E49FDF-6456-4075-BECB-81835AD03910");
FakeCiscoSourceType::FakeCiscoSourceType():
    SourceType(fake_cisco_uuid, L"FakeCisco", parse_properties_def(cisco_definition), null_uuid())
{
}

void FakeCiscoSourceType::set_running_ref(ReportTypeRef running_ref)
{
    _ASSERT(!m_running_ref);
    m_running_ref = running_ref;
}

void FakeCiscoSourceType::set_xml_report_ref(ReportTypeRef xml_report_ref)
{
    _ASSERT(!m_xml_report_ref);
    m_xml_report_ref = xml_report_ref;
}

void FakeCiscoSourceType::set_compliance_ref(ReportTypeRef compliance_ref)
{
    _ASSERT(!m_compliance_ref);
    m_compliance_ref = compliance_ref;
}

void FakeCiscoSourceType::set_validation_ref(ReportTypeRef validation_ref)
{
    _ASSERT(!m_validation_ref);
    m_validation_ref = validation_ref;
}

std::vector<ReportTypeRef> FakeCiscoSourceType::get_downloads() const
{
    _ASSERT(m_running_ref);
    _ASSERT(m_compliance_ref);
    _ASSERT(m_validation_ref);
    return{*m_running_ref, *m_xml_report_ref, *m_compliance_ref, *m_validation_ref};
}

SourceType::DownloadFunctor FakeCiscoSourceType::prepare_download(ReportTypeRef report_type_ref, SourceRef source_ref) const
{
    auto make_content = (*report_type_ref).prepare_make_content_from_blob();

    if (report_type_ref == *m_running_ref)
    {
        return [make_content](const XmlDoc&){
            std::string config("cisco ios config \r\n");
            config += std::to_string(time(nullptr));
            return make_content(Blob(config.begin(), config.end()));
        };
    }
    else if (report_type_ref == *m_xml_report_ref)
    {
        return [make_content](const XmlDoc&){
            static size_t counter = 0;
            std::string docs[] = {
                "<?xml version=\"1.0\"?>"
                "<root>"
                    "<node>"
                        "<id>1</id>"
                        "value1"
                    "</node>"                   
                "</root>",

                "<?xml version=\"1.0\"?>"
                "<root>"
                    "<node>"
                        "<id>1</id>"
                        "value2"
                    "</node>"                   
                "</root>",

                "<?xml version=\"1.0\"?>"
                "<root>"
                    "<node>"
                        "<id>2</id>"
                        "value3"
                    "</node>"
                    "<node>"
                        "<id>1</id>"
                        "value2"
                    "</node>"                           
                "</root>"
            };

            auto& doc = docs[counter++ % _countof(docs)];
            return make_content(Blob(doc.begin(), doc.end()));
        };
    }
    else if (report_type_ref == *m_compliance_ref)
    {
        return [make_content](const XmlDoc&){
            std::string doc = "<?xml version=\"1.0\"?>"
                "<compliance>"
                    "<rule1>"
                        "<result>1</result>"
                        "<value>value</value>"
                    "</rule1>"
                    "<rule2>"
                        "<result>0</result>"
                        "<value>value</value>"
                    "</rule2>"
                "</compliance>";            
            return make_content(Blob(doc.begin(), doc.end()));
        };
    }
    else if (report_type_ref == *m_validation_ref)
    {
        return [make_content](const XmlDoc&){
            std::string doc = "<?xml version=\"1.0\"?>"
                "<list>"
                    "<trivial condition='valid'>"
                        "value1"
                    "</trivial>"
                    "<trivial condition='invalid'>"
                        "value3"
                    "</trivial>"
                "</list>";
            return make_content(Blob(doc.begin(), doc.end()));
        };
    }
    _ASSERT(false);
    return nullptr;
}

std::vector<StreamTypeRef> FakeCiscoSourceType::get_streams() const
{
    return{};
}

const StreamTransport& FakeCiscoSourceType::get_transport(StreamTypeRef stream_type_ref) const
{
    _ASSERT(false);
    throw std::logic_error("Invalid stream type");
}

std::vector<ActionRef> FakeCiscoSourceType::get_actions() const
{
    return {};
}

std::vector<ReportTypeRef> FakeCiscoSourceType::get_reloads(ActionRef action_ref) const
{
    _ASSERT(false);
    return {};
}

SourceType::ActionFunctor FakeCiscoSourceType::prepare_action(ActionRef action_ref, SourceRef source_ref) const
{
    _ASSERT(false);
    return nullptr;
}

auto fake_cisco_feature_uuid = gen_uuid(L"3115AE86-CE14-40C1-9BA8-61ED5917F1F4");
FakeCiscoFeature::FakeCiscoFeature():
    ComposedFeature(fake_cisco_feature_uuid, L"Fake Cisco Feature")
{
    add_report_type_factory(std::make_unique<CustomReportTypeFactory>(fake_cisco_runnig_uuid, L"FakeCiscoRunnig", [](const CoreImpl& core_impl){
        return std::make_unique<FakeCiscoRunning>();
    }));

    add_report_type_factory(std::make_unique<CustomReportTypeFactory>(fake_cisco_xml_uuid, L"FakeCiscoXmlReport", [](const CoreImpl& core_impl){
        return std::make_unique<FakeCiscoXmlReport>();
    }));

    add_report_type_factory(std::make_unique<CustomReportTypeFactory>(fake_cisco_parsed_runnig_uuid, L"FakeCiscoParsedRunnig", [](const CoreImpl& core_impl){
        return std::make_unique<FakeCiscoParsedRunning>();
    }));

    add_report_type_factory(std::make_unique<CustomReportTypeFactory>(fake_cisco_compliance_uuid, L"FakeCiscoComplianceBase", [](const CoreImpl& core_impl){
        return std::make_unique<FakeCiscoCompliance>();
    }));

    add_report_type_factory(std::make_unique<CustomReportTypeFactory>(fake_cisco_validation_uuid, L"FakeCiscoValidation", [](const CoreImpl& core_impl){
        return std::make_unique<FakeCiscoValidation>();
    }));
    
    add_selection_factory(std::make_unique<CustomSelectionFactory>(std::vector<ReportTypeUUID>{fake_cisco_runnig_uuid}, fake_cisco_parsed_runnig_uuid, [](const CoreImpl& core_impl){
        auto running_parser = std::make_unique<FakeCiscoRunningParser>();
        running_parser->set_running_ref(core_impl.m_core_domain.m_basis.m_report_types.get_report_type(fake_cisco_runnig_uuid));
        running_parser->set_parsed_running_ref(core_impl.m_core_domain.m_basis.m_report_types.get_report_type(fake_cisco_parsed_runnig_uuid));
        return running_parser;
    }));

    add_source_type_factory(std::make_unique<CustomSourceTypeFactory>(fake_cisco_uuid, L"FakeCisco", [](const CoreImpl& core_impl){
        auto cisco_source = std::make_unique<FakeCiscoSourceType>();
        cisco_source->set_running_ref(core_impl.m_core_domain.m_basis.m_report_types.get_report_type(fake_cisco_runnig_uuid));
        cisco_source->set_xml_report_ref(core_impl.m_core_domain.m_basis.m_report_types.get_report_type(fake_cisco_xml_uuid));
        cisco_source->set_compliance_ref(core_impl.m_core_domain.m_basis.m_report_types.get_report_type(fake_cisco_compliance_uuid));
        cisco_source->set_validation_ref(core_impl.m_core_domain.m_basis.m_report_types.get_report_type(fake_cisco_validation_uuid));
        return cisco_source;
    }));

    add_compliance_definition({L"FakeCiscoCompliance", fake_cisco_compliance_uuid, ReportTypeUUID(stl_tools::gen_uuid())});
    add_validation_definition({L"FakeCiscoValidation", fake_cisco_parsed_runnig_uuid, ReportTypeUUID(stl_tools::gen_uuid())});
}

UUID FakeCiscoFeature::get_feature_uuid()
{
    return fake_cisco_feature_uuid;
}

}}