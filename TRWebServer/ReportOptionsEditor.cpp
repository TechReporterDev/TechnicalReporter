#include "stdafx.h"
#include "ReportOptionsEditor.h"
#include "OptionProperties.h"
#include "RefValue.h"
#include "CustomValue.h"
#include "StaticValue.h"
#include "ContentView.h"
#include "PatternEditor.h"
#include "ComplianceEditor.h"
#include "ValidatorEditor.h"
#include "CommonPropertyPage.h"
#include "Application.h"

namespace {

enum class ReportGeneratingPolicy { DEFAULT = 0, USE_STORAGE, ON_THE_FLY, DISABLED };

ReportGeneratingPolicy getReportGeneratingPolicy(const TR::SubjectReportGenerating& generating)
{
    if (generating.m_report_visibility == TR::ReportVisibility::DEFAULT_VISIBILITY &&
        generating.m_defer_report_policy == TR::DeferReportPolicy::DEFAULT_POLICY)
    {
        return ReportGeneratingPolicy::DEFAULT;
    }

    bool visibile = generating.m_report_visibility == TR::ReportVisibility::DEFAULT_VISIBILITY ? generating.m_default_visibile :
        generating.m_report_visibility == TR::ReportVisibility::VISIBLE_REPORT;

    if (!visibile)
    {
        return ReportGeneratingPolicy::DISABLED;
    }

    bool defered = generating.m_defer_report_policy == TR::DeferReportPolicy::DEFAULT_POLICY ? generating.m_default_deferred :
        generating.m_defer_report_policy == TR::DeferReportPolicy::DEFER_REPORT;

    return defered ? ReportGeneratingPolicy::ON_THE_FLY : ReportGeneratingPolicy::USE_STORAGE;
}

ReportGeneratingPolicy getDefaultReportGeneratingPolicy(const TR::SubjectReportGenerating& generating)
{
    if (!generating.m_default_visibile)
    {
        return ReportGeneratingPolicy::DISABLED;
    }

    return generating.m_default_deferred ? ReportGeneratingPolicy::ON_THE_FLY : ReportGeneratingPolicy::USE_STORAGE;
}

ReportGeneratingPolicy getEffectiveReportGeneratingPolicy(const TR::SubjectReportGenerating& generating)
{
    auto policy = getReportGeneratingPolicy(generating);
    if (policy == ReportGeneratingPolicy::DEFAULT)
    {
        policy = getDefaultReportGeneratingPolicy(generating);
    }
    return policy;  
}

void setReportGeneratingPolicy(TR::ReportGeneratingOptions& generating, ReportGeneratingPolicy policy)
{
    switch (policy)
    {
    case ReportGeneratingPolicy::DEFAULT:
        generating.m_report_visibility = TR::ReportVisibility::DEFAULT_VISIBILITY;
        generating.m_defer_report_policy = TR::DeferReportPolicy::DEFAULT_POLICY;
        break;

    case ReportGeneratingPolicy::DISABLED:
        generating.m_report_visibility = TR::ReportVisibility::INVISIBLE_REPORT;
        generating.m_defer_report_policy = TR::DeferReportPolicy::DEFAULT_POLICY;
        break;

    case ReportGeneratingPolicy::ON_THE_FLY:
        generating.m_report_visibility = TR::ReportVisibility::VISIBLE_REPORT;
        generating.m_defer_report_policy = TR::DeferReportPolicy::DEFER_REPORT;
        break;

    case ReportGeneratingPolicy::USE_STORAGE:
        generating.m_report_visibility = TR::ReportVisibility::VISIBLE_REPORT;
        generating.m_defer_report_policy = TR::DeferReportPolicy::NOT_DEFER_REPORT;
        break;

    default:
        _ASSERT(false);
    }
}

std::wstring toString(ReportGeneratingPolicy policy)
{
    switch (policy)
    {
    case ReportGeneratingPolicy::DEFAULT:
        return L"Default";

    case ReportGeneratingPolicy::DISABLED:
        return L"Not required";

    case ReportGeneratingPolicy::ON_THE_FLY:
        return L"On the fly";

    case ReportGeneratingPolicy::USE_STORAGE:
        return L"Use storage";

    default:
        _ASSERT(false);
    }

    return L"";
}

SelectorProperty ReportGeneratingProperty(TR::SubjectReportOptions& options)
{
    std::vector<std::wstring> items{
    (boost::wformat(L"Default (%1%)") % toString(getDefaultReportGeneratingPolicy(options.m_report_generating))).str(),
    toString(ReportGeneratingPolicy::USE_STORAGE),
    toString(ReportGeneratingPolicy::ON_THE_FLY),
    toString(ReportGeneratingPolicy::DISABLED)  
     }; 

    return SelectorProperty(L"", L"Setup how report will be generated", std::move(items), customValue<int>(
        [&options] {
            return int(getReportGeneratingPolicy(options.m_report_generating)); 
        },
        [&options] (int value){
            return setReportGeneratingPolicy(options.m_report_generating, ReportGeneratingPolicy(value)); 
        }
    ));
}

} //namespace {

class ReportOptionsEditor::GeneralPage: public CommonPropertyPage
{
public:
    enum class Editor { DEFAULT_PATTERN_VIEW = 0, PATTERN_EDITOR, COMPLIANCE_EDITOR, VALIDATION_EDITOR, UNCOMPATIBLE_OPTIONS};

    GeneralPage(ReportOptionsEditor* optionsEditor):
        CommonPropertyPage(L"General", L"General report options"),
        m_optionsEditor(optionsEditor),
        m_options(optionsEditor->m_subjectReportOptions),
        m_showUncompatibleOptions(false)
    {
        auto& client = Application::instance()->getClient();
        m_schedulesInfo = client.getSchedulesInfo();
        
        CommonProperty properties(L"", L"", CommonProperty(L"Report generating", L"",
            ReportGeneratingProperty(m_options) 
        ));     
        
        properties.addProperties(
            OptionalProperty(
                [this] {
                    return m_showUncompatibleOptions == true || 
                        getEffectiveReportGeneratingPolicy(m_options.m_report_generating) == ReportGeneratingPolicy::USE_STORAGE;
                },          
                CommonProperty(L"Report loading", L"",
                    ScheduleProperty(L"", L"Select schedule to load report from source", m_schedulesInfo,
                        m_options.m_report_generating.m_default_schedule_key,
                        m_options.m_report_generating.m_schedule_key
                    )
                )
        ));

        if (m_options.m_compliance_checking)
        {
            properties.addProperties(OptionalProperty(
                [this] {
                    return m_showUncompatibleOptions == true ||
                        getEffectiveReportGeneratingPolicy(m_options.m_report_generating) == ReportGeneratingPolicy::USE_STORAGE;
                }, 
                CommonProperty(L"Compliane", L"",
                    PolicyProperty(L"", L"Enable compliance checking", 
                        {L"Enabled", L"Disabled"}, m_options.m_compliance_checking->m_default_checking_on,
                        m_options.m_compliance_checking->m_compliance_checking_policy
                    ),
                    AnyProperty(L"", L"Setup compliance rules", L"Edit", 
                        staticValue(boost::any(Editor::COMPLIANCE_EDITOR))
                    ),
                    PolicyProperty(L"", L"Send email if compliance checking failed", {L"Yes", L"No"},
                        m_options.m_compliance_checking->m_default_notification_enabled,
                        m_options.m_compliance_checking->m_notification_policy
                    )
                )
            ));

            properties.addProperties(OptionalProperty(
                [this] {
                    return m_showUncompatibleOptions == false ||
                        getEffectiveReportGeneratingPolicy(m_options.m_report_generating) == ReportGeneratingPolicy::ON_THE_FLY;
                }, 
                CommonProperty(L"Compliane", L"",                   
                    AnyProperty(L"", L"Setup compliance rules", L"Edit", 
                        staticValue(boost::any(Editor::COMPLIANCE_EDITOR))
                    )
                )
            ));
        }

        if (m_options.m_validating)
        {
            properties.addProperties(OptionalProperty(
                [this] {
                    return m_showUncompatibleOptions == true ||
                        getEffectiveReportGeneratingPolicy(m_options.m_report_generating) == ReportGeneratingPolicy::USE_STORAGE;
                },
                CommonProperty(L"Validating", L"",
                    PolicyProperty(L"", L"Select validating policy", 
                        {L"Enabled", L"Disabled"}, m_options.m_validating->m_default_validation_on,
                        m_options.m_validating->m_validation_policy
                    ),
                    AnyProperty(L"", L"Edit validating rules", L"Edit", 
                        staticValue(boost::any(Editor::VALIDATION_EDITOR))
                    ),
                    PolicyProperty(L"", L"Send email if validation checking failed", 
                        {L"Yes", L"No"}, m_options.m_validating->m_default_notification_enabled,
                        m_options.m_validating->m_notification_policy
                    )
                )
            ));
        }

        if (m_options.m_integrity_checking)
        {
            properties.addProperties(OptionalProperty(
                [this] {
                    return m_showUncompatibleOptions == true ||
                        getEffectiveReportGeneratingPolicy(m_options.m_report_generating) == ReportGeneratingPolicy::USE_STORAGE;
                },
                CommonProperty(L"IntegrityChecking", L"",
                    PolicyProperty(L"", L"Select integrity checking policy", 
                        {L"Enabled", L"Disabled"}, m_options.m_integrity_checking->m_default_checking_on,
                        m_options.m_integrity_checking->m_integrity_checking_policy
                    ),
                    SwitchPropertyEx(L"", L"Specify pattern to match", 
                        { 
                            {L"Use default pattern", L"Show", boost::any(Editor::DEFAULT_PATTERN_VIEW)},
                            {L"Use custom pattern", boost::any(Editor::PATTERN_EDITOR)}
                        }, refValue(m_options.m_integrity_checking->m_pattern_choise)
                    ),
                    PolicyProperty(L"", L"Send email if integrity checking failed", 
                        {L"Yes", L"No"}, m_options.m_integrity_checking->m_default_mailing_enabled,
                        m_options.m_integrity_checking->m_notification_policy
                    )
                )
            ));
        }

        if (m_options.m_archiving)
        {
            properties.addProperties(OptionalProperty(
                [this] {
                    return m_showUncompatibleOptions == true ||
                        getEffectiveReportGeneratingPolicy(m_options.m_report_generating) == ReportGeneratingPolicy::USE_STORAGE;
                },
                CommonProperty(L"Archiving", L"",
                    PolicyProperty(L"", L"Select archiving policy",
                        { L"Disabled", L"Enabled" }, m_options.m_archiving->m_default_archiving_policy,
                        m_options.m_archiving->m_archiving_policy
                    ),
                    PolicyProperty(L"", L"Select archived reports expiration time",
                        { L"Never", L"Hour", L"Day", L"Month", L"Year" }, m_options.m_archiving->m_default_archiving_expiration,
                        m_options.m_archiving->m_archiving_expiration
                    )
                )
            ));
        }

        properties.addProperties(
            OptionalProperty(
                [this] {
                    return m_showUncompatibleOptions == false &&
                        getEffectiveReportGeneratingPolicy(m_options.m_report_generating) != ReportGeneratingPolicy::USE_STORAGE;
                },          
                CommonProperty(L"Uncompatible features", L"",
                    AnyProperty(L"", L"Some features are uncompatible with generation mode. Click 'Show' to see them.", L"Show",
                        staticValue(boost::any(Editor::UNCOMPATIBLE_OPTIONS)))
                )
        ));

        show(std::move(properties));
    }

    virtual void handleEvent(boost::any any) override
    {
        switch (boost::any_cast<Editor>(any))
        {
        case Editor::DEFAULT_PATTERN_VIEW:
            m_propertySheet->pushPage(m_optionsEditor->createDefaultPatternPage());
            break;

        case Editor::PATTERN_EDITOR:
            m_propertySheet->pushPage(m_optionsEditor->createPatternPage());
            break;

        case Editor::COMPLIANCE_EDITOR:
            m_propertySheet->pushPage(m_optionsEditor->createCompliancePage());
            break;

        case Editor::VALIDATION_EDITOR:
            m_propertySheet->pushPage(m_optionsEditor->createValidationPage());
            break;

        case Editor::UNCOMPATIBLE_OPTIONS:
            m_showUncompatibleOptions = true;
            refresh();
            break;

        default:
            _ASSERT(false);
        }
    }

private:
    ReportOptionsEditor* m_optionsEditor;
    TR::SubjectReportOptions& m_options;
    std::vector<TR::ScheduleInfo> m_schedulesInfo;
    bool m_showUncompatibleOptions;
};

ReportOptionsEditor::ReportOptionsEditor(TR::SubjectReportOptions subjectReportOptions):
    m_subjectReportOptions(std::move(subjectReportOptions))
{
    auto& client = Application::instance()->getClient();
    m_reportTypeInfo = client.getReportTypeInfo(m_subjectReportOptions.m_report_type_uuid);
    pushPage(createGeneralPage());
}

ReportOptionsEditor::~ReportOptionsEditor()
{
    clear();
}

bool ReportOptionsEditor::apply()
{
    if (!PropertySheetWidget::apply())
    {
        return false;
    }

    auto& client = Application::instance()->getClient();
    client.setOptions(m_subjectReportOptions.m_subject_key, m_subjectReportOptions);

    if (m_patternEditor)
    {
        m_patternEditor->apply();
    }

    if (m_complianceEditor)
    {
        m_complianceEditor->apply();
    }

    if (m_validatorEditor)
    {
        m_validatorEditor->apply();
    }

    return true;
}

std::unique_ptr<PropertyPageWidget> ReportOptionsEditor::createGeneralPage()
{
    return std::make_unique<GeneralPage>(this);
}

std::unique_ptr<PropertyPageWidget> ReportOptionsEditor::createDefaultPatternPage()
{
    if (!m_defaultPatternView)
    {
        auto app = Application::instance();
        auto& client = app->getClient();
        TR::RoleKey parentKey(0);
        if (auto source_key = boost::get<TR::SourceKey>(&m_subjectReportOptions.m_subject_key))
        {
            parentKey = client.getSourceInfo(*source_key).m_role_key;
        }
        else if (auto role_key = boost::get<TR::RoleKey>(&m_subjectReportOptions.m_subject_key))
        {
            parentKey = client.getRoleInfo(*role_key).m_parent_key;
        }

        std::shared_ptr<TR::Content> content;
        if (parentKey != TR::RoleKey(0))
        {
            content = client.getEffectivePattern(parentKey, m_subjectReportOptions.m_report_type_uuid);
        }
        m_defaultPatternView = createContentView(content);
    }

    return std::make_unique<ContainerPage>(L"Default pattern", L"", std::make_unique<WidgetHolder>(m_defaultPatternView.get()));
}

std::unique_ptr<PropertyPageWidget> ReportOptionsEditor::createPatternPage()
{
    if (!m_patternEditor)
    {
        m_patternEditor.reset(new PatternEditor(m_subjectReportOptions.m_subject_key, m_subjectReportOptions.m_report_type_uuid));
    }

    return std::make_unique<ContainerPage>(L"Pattern", L"Edit pattern to compare", std::make_unique<WidgetHolder>(m_patternEditor.get()));
}

std::unique_ptr<PropertyPageWidget> ReportOptionsEditor::createCompliancePage()
{
    if (!m_complianceEditor)
    {
        m_complianceEditor.reset(new ComplianceEditor(m_subjectReportOptions.m_subject_key, m_reportTypeInfo->m_compliance_key));
    }

    return std::make_unique<ContainerPage>(L"Compliance Settings", L"Setup rules for compliance", std::make_unique<WidgetHolder>(m_complianceEditor.get()));
}

std::unique_ptr<PropertyPageWidget> ReportOptionsEditor::createValidationPage()
{
    if (!m_validatorEditor)
    {
        m_validatorEditor.reset(new ValidatorEditor(m_subjectReportOptions.m_subject_key, m_reportTypeInfo->m_validation_key));
    }
    return std::make_unique<ContainerPage>(L"Validation settings", L"Setup validation conditions", std::make_unique<WidgetHolder>(m_validatorEditor.get()));
}