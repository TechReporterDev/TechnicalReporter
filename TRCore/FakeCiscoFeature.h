#pragma once
#include "Features.h"
#include "ComposedFeature.h"
#include "stddecl.h"
#include "ReportTypes.h"
#include "SourceTypes.h"
#include "Selections.h"
#include "Groupings.h"
#include "Compliance.h"
#include "Validation.h"
#include "Content.h"
namespace TR { namespace Core {

class FakeCiscoRunning: public PlainTextReportType
{
public:
    FakeCiscoRunning();
};

class FakeCiscoXmlReport: public PlainXmlReportType
{
public:
    FakeCiscoXmlReport();
};

class FakeCiscoParsedRunning: public GeneralRegularReportType
{
public:
    FakeCiscoParsedRunning();
};

class FakeCiscoRunningParser: public Selection
{
public:
    FakeCiscoRunningParser();
    void set_running_ref(ReportTypeRef running_ref);
    void set_parsed_running_ref(ReportTypeRef parsed_running_ref);

    //override
    virtual SelectionPlan       get_plan() const override;
    virtual Functor             prepare_functor(SourceRef /*source_ref*/) const override;

private:
    boost::optional<ReportTypeRef> m_running_ref;
    boost::optional<ReportTypeRef> m_parsed_running_ref;
};

class FakeCiscoCompliance: public CheckListReportType
{
public:
    FakeCiscoCompliance();
};

class FakeCiscoValidation: public GeneralRegularReportType
{
public:
    FakeCiscoValidation();
};

class FakeCiscoSourceType: public SourceType
{
public:
    FakeCiscoSourceType();
    void set_running_ref(ReportTypeRef running_ref);
    void set_xml_report_ref(ReportTypeRef xml_report_ref);
    void set_compliance_ref(ReportTypeRef compliance_ref);
    void set_validation_ref(ReportTypeRef validation_ref);

    // override
    virtual std::vector<ReportTypeRef>      get_downloads() const override;
    virtual DownloadFunctor                 prepare_download(ReportTypeRef report_type_ref, SourceRef source_ref) const override;

    virtual std::vector<StreamTypeRef>      get_streams() const override;
    virtual const StreamTransport&          get_transport(StreamTypeRef stream_type_ref) const override;

    virtual std::vector<ActionRef>          get_actions() const override;
    virtual std::vector<ReportTypeRef>      get_reloads(ActionRef action_ref) const override;
    virtual ActionFunctor                   prepare_action(ActionRef action_ref, SourceRef source_ref) const override;

private:
    boost::optional<ReportTypeRef> m_running_ref;
    boost::optional<ReportTypeRef> m_xml_report_ref;
    boost::optional<ReportTypeRef> m_compliance_ref;
    boost::optional<ReportTypeRef> m_validation_ref;
};

class FakeCiscoFeature: public ComposedFeature
{
public:
    static UUID get_feature_uuid();
    FakeCiscoFeature();
};

}} //namespace TR { namespace Core {