#include "stdafx.h"
#include "ComplianceSetupView.h"
#include "CommonPropertyTable.h"
#include "CommonPropertyPage.h"
#include "PropertyDialog.h"
#include "CustomValue.h"
#include "StaticValue.h"
#include "ContainerArray.h"
#include "ComposedCollection.h"
#include "Application.h"
#include <boost/algorithm/string.hpp>

namespace {

class ExceptionsDlg: public PropertyDialog
{
public:
    ExceptionsDlg(TR::XML::XmlRuleSetup& ruleSetup, const TR::XML::XmlRuleSetup& baseRuleSetup):
        PropertyDialog(L"Exceptions"),
        m_ruleSetup(ruleSetup),
        m_baseRuleSetup(baseRuleSetup)
    {

        for (auto& exception : m_ruleSetup.get_exceptions())
        {
            if (!m_exceptions.empty())
            {
                m_exceptions.append(L"\n");
            }
            m_exceptions.append(exception);
        }

        for (auto& exception : m_baseRuleSetup.get_exceptions())
        {
            if (!m_baseExceptions.empty())
            {
                m_baseExceptions.append(L"\n");
            }
            m_baseExceptions.append(exception);
        }

        init(std::make_unique<CommonPropertyPage>(CommonProperty{L"", L"Setup values to except from compliance",
            TextProperty(L"Exceptions", L"", 3, refValue(m_exceptions)),
            TextProperty(L"Base exceptions", L"", 3, refValue(m_baseExceptions), PropertyAttribute::READ_ONLY)
        }, 0));
    }

    virtual void apply() override
    {
        std::vector<std::wstring> exceptions;
        boost::split(exceptions, m_exceptions, boost::is_any_of(L"\r\n"), boost::algorithm::token_compress_on);
        m_ruleSetup.set_exceptions(exceptions);
    }

private:
    TR::XML::XmlRuleSetup& m_ruleSetup;
    const TR::XML::XmlRuleSetup& m_baseRuleSetup;
    std::wstring m_exceptions;
    std::wstring m_baseExceptions;
};

} //namespace {

ComplianceSetupView::ComplianceSetupView(TR::XmlComplianceSetup& complianceSetup, const TR::XmlComplianceSetup& baseSetup, const TR::XmlComplianceDoc* browse_doc)
{
    stl_tools::associate(complianceSetup.get_rule_setups(), baseSetup.get_rule_setups(), [&](TR::XML::XmlRuleSetup* ruleSetup, const TR::XML::XmlRuleSetup* baseRuleSetup)
    {
        m_rules.push_back({ruleSetup, baseRuleSetup});
    });

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    auto tableView = vbox->addWidget(std::make_unique<CommonPropertyTable>(std::initializer_list<CommonPropertyGrid::Column>{
        PropertyArray<StringPropertyFactory>(L"", L"", {L"Name", L"Rule name", STRING_ATTRIBUTES(READ_ONLY)},
            containerArray<SingleValue<std::wstring>>(m_rules, [](Rule& rule){
                return staticValue(rule.m_setup->get_bound_def().get_caption());
            })
        ),

        PropertyArray<DynamicSelectorPropertyFactory>(L"", L"", {L"Checking", L"Enable checking policy"},
            containerArray<ValueCollection>(m_rules, [](Rule& rule){
                std::wstring base_enable_str = rule.m_base->get_checking_policy() == TR::XML::XmlRuleSetup::DISABLE_CHECKING ? L"disable" : L"enable";
                return composedCollection({
                    customValue<int>(
                        [&]{ return rule.m_setup->get_checking_policy(); },
                        [&](int value){ rule.m_setup->set_checking_policy(TR::XML::XmlRuleSetup::CheckingPolicy(value)); }
                    ),
                    selfArray<std::wstring>({
                        L"Default(" + base_enable_str + L")",
                        L"Enable",
                        L"Disable"
                    })
                });
            })
        ),

        PropertyArray<AnyPropertyFactory>(L"", L"", {L"Exceptions", L"", L""},
            containerArray<SingleValue<boost::any>>(m_rules, [](Rule& rule){
                return staticValue(boost::any(rule));
            })
        ),

        PropertyArray<StringPropertyFactory>(L"", L"", {L"Description", L"Rule description", STRING_ATTRIBUTES(READ_ONLY)},
            containerArray<SingleValue<std::wstring>>(m_rules, [](Rule& rule){
                return staticValue(rule.m_setup->get_bound_def().get_description());
            })
        )
    }), 1);

    tableView->setAnyPresenter([](AnyProperty& anyProperty){
        auto rule = boost::any_cast<Rule>(anyProperty.getAny());
        auto exceptionsButton = std::make_unique<Wt::WPushButton>("Edit");
        exceptionsButton->setStyleClass("btn-xs");
        exceptionsButton->clicked().connect([rule](Wt::WMouseEvent){
            auto exceptionDlg = new ExceptionsDlg(*rule.m_setup, *rule.m_base);
            exceptionDlg->show();
        });
        return exceptionsButton;
    });
}
