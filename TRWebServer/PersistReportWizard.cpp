#include "stdafx.h"
#include "PersistReportWizard.h"
#include "PropertyDialog.h"
#include "CommonPropertyPage.h"
#include "CommonPropertyDoc.h"
#include "ContainerArray.h"
#include "SourceBrowseDlg.h"
#include "Application.h"

class PersistReportWizard::ReportOwnerPage: public CommonPropertyPage
{
public:
    ReportOwnerPage(PersistReportWizard* wizard):
        CommonPropertyPage(L"Report owner", L"Select which source can be owner of this report", PersistReportWizard::REPORT_OWNER),
        m_wizard(wizard)
    {
        CommonProperty properties(L"", L"",
            SwitchCommonProperty(L"", L"", {
                SwitchCommonProperty::Case{
                    L"Specified source",
                    {L"", L"", getBrowseProperty(DEFAULT_PROPERTY_ATTRIBUTES)},
                    {L"", L"", getBrowseProperty(DEFAULT_PROPERTY_ATTRIBUTES | PropertyAttribute::READ_ONLY)}
                },
                SwitchCommonProperty::Case{
                    L"Any available source",
                    {L"", L"Report will be visible for any source"},
                    {L"", L"Report will be visible for any source"}
                }}, 
                refValue(m_wizard->m_reportOwner)
            )
        );
        show(std::move(properties));
    }

private:
    BrowseProperty getBrowseProperty(PropertyAttributes attributes)
    {
        return BrowseProperty(L"", L"", L"Browse...", boost::any(),
            [this](boost::any){
                _ASSERT(m_wizard->m_ownerKey != TR::INVALID_SOURCE_KEY);
                return m_wizard->m_client.getSourceInfo(m_wizard->m_ownerKey).m_name;
            }, attributes);
    }

    virtual void handleEvent(boost::any any) override
    {
        auto browseDlg = new SourceBrowseDlg(m_wizard->m_ownerKey);
        browseDlg->connectApply([this](const TR::SourceInfo& ownerSourceInfo){
            m_wizard->m_ownerKey = ownerSourceInfo.m_key;
            refresh();
        });
        browseDlg->show();
    }

    PersistReportWizard* m_wizard;
};

PersistReportWizard::PersistReportWizard(Wt::WWidget* parent, const Wt::WString& title):
    PropertyWizard(title),
    m_client(Application::instance()->getClient()),
    m_reportNature(STANDALONE),
    m_reportOwner(ANY_SOURCE),
    m_ownerKey(TR::ROOT_GROUP_KEY)
{
    init(createReportNaturePage());
}

std::unique_ptr<PropertyPageWidget> PersistReportWizard::createReportNaturePage()
{
    return std::make_unique<CommonPropertyPage>(CommonProperty{L"Report behaviour", L"Select report behaviour",
        SwitchProperty(L"", L"",
            {L"Standalone - generate as soon as possible,\narchving and integrity checking are available.", L"View - generate only by user request,\nàny processing is not available."},
            refValue(m_reportNature)
        )
    }, REPORT_NATURE);
}

std::unique_ptr<PropertyPageWidget> PersistReportWizard::createReportOwnerPage()
{
    return std::make_unique<ReportOwnerPage>(this);
}

std::unique_ptr<PropertyPageWidget> PersistReportWizard::createReportNamePage()
{
    return std::make_unique<CommonPropertyPage>(CommonProperty{L"Last step", L"Enter name and click 'Finish' to create report.",
        StringProperty(L"Report name", L"", refValue(m_reportName))
    }, REPORT_NAME);
}

std::unique_ptr<PropertyPageWidget> PersistReportWizard::nextPage(int currentPageIdentity)
{
    switch (currentPageIdentity)
    {
        case REPORT_NATURE:
            return createReportOwnerPage();

        case REPORT_OWNER:
            return createReportNamePage();

        default:
            _ASSERT(false);     
    }
    return nullptr;
}

bool PersistReportWizard::isFinal(int currentPageIdentity)
{
    return currentPageIdentity == REPORT_NAME;
}

PersistFilterWizard::PersistFilterWizard(Wt::WWidget* parent, TR::ReportTypeInfo inputInfo, TR::CustomFilterSetup filterSetup, TR::SourceKey ownerKey):
    PersistReportWizard(parent, L"Create report filter"),
    m_inputInfo(std::move(inputInfo)),
    m_filterSetup(std::move(filterSetup))
{
    auto inputOptions = m_client.getOptions(TR::DEFAULT_ROLE_KEY, inputInfo.m_uuid);
    if (inputOptions.m_report_generating.m_defer_report_policy == TR::DeferReportPolicy::DEFER_REPORT)
    {
        m_reportNature = VIEW;
    }

    if (ownerKey != TR::INVALID_SOURCE_KEY)
    {
        m_reportOwner = SPECIFIED_SOURCE;
        m_ownerKey = ownerKey;
    }   
}

void PersistFilterWizard::onFinish()
{
    m_client.addCustomFilter(m_reportName, m_inputInfo.m_uuid, m_filterSetup);
}