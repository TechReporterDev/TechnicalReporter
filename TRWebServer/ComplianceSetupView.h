#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class ComplianceSetupView: public Wt::WContainerWidget
{
public:
    ComplianceSetupView(TR::XmlComplianceSetup& complianceSetup, const TR::XmlComplianceSetup& baseSetup, const TR::XmlComplianceDoc* browseDoc);

private:
    struct Rule
    {
        TR::XML::XmlRuleSetup* m_setup;
        const TR::XML::XmlRuleSetup* m_base;
    };

    std::vector<Rule> m_rules;
};