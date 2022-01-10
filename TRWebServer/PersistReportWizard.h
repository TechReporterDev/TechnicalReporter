#pragma once
#include "PropertyWizard.h"
class AppClient;

class PersistReportWizard: public PropertyWizard
{
public:
    PersistReportWizard(Wt::WWidget* parent, const Wt::WString& title);
    virtual ~PersistReportWizard() = default;

protected:
    class ReportOwnerPage;
    enum ReportNature { STANDALONE = 0, VIEW };
    enum ReportOwner { SPECIFIED_SOURCE = 0, ANY_SOURCE };
    enum PageID { REPORT_NATURE = 0, REPORT_OWNER, REPORT_NAME };

    // property pages
    std::unique_ptr<PropertyPageWidget> createReportNaturePage();
    std::unique_ptr<PropertyPageWidget> createReportOwnerPage();
    std::unique_ptr<PropertyPageWidget> createReportNamePage();

    // PropertyWizard override
    virtual std::unique_ptr<PropertyPageWidget> nextPage(int currentPageIdentity) override;
    virtual bool isFinal(int currentPageIdentity) override;

    AppClient& m_client;
    ReportNature m_reportNature;    
    ReportOwner m_reportOwner;
    TR::SourceKey m_ownerKey;
    std::wstring m_reportName;
};

class PersistFilterWizard: public PersistReportWizard
{
public:
    PersistFilterWizard(Wt::WWidget* parent, TR::ReportTypeInfo inputInfo, TR::CustomFilterSetup filterSetup, TR::SourceKey ownerKey = TR::INVALID_SOURCE_KEY);
    virtual ~PersistFilterWizard() = default;

    // PropertyWizard override
    virtual void onFinish() override;

private:
    TR::ReportTypeInfo m_inputInfo;
    TR::CustomFilterSetup m_filterSetup;
};