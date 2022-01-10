#include "stdafx.h"
#include "ReportSelectionPage.h"
#include "ReportOptionsPage.h"
#include "CreateQueryPage.h"
#include "FilterSetupPage.h"
#include "ValidationSettingsPage.h"
#include "OptionsPage.h"
#include "PropertyWizard.h"
#include "CommonPropertyPage.h"
#include "RefValue.h"
#include "ContainerArray.h"
#include "InventoryTable.h"
#include "PropertyBox.h"
#include "Application.h"

class CreateReportWizard: public PropertyWizard
{
public:
    enum KindOfReport { QUERY, FILTER, GROUPING, VALIDATING };
    enum PageID { KIND_OF_REPORT = 0, QUERY_INPUT, FILTER_INPUT, GROUPING_INPUT, VALIDATING_INPUT, QUERY_FINISH, FILTER_FINISH, GROUPING_FINISH, VALIDATING_FINISH };

    CreateReportWizard(ReportSelectionWidget* parentWidget):
        PropertyWizard(L"Create report"),
        m_parentWidget(parentWidget),
        m_kindOfReport(QUERY),
        m_selectedInput(0)
    {
        auto& client = Application::instance()->getClient();
        for (auto reportTypeInfo : client.getReportTypesInfo())
        {
            if (reportTypeInfo.m_syntax == TR::ReportTypeInfo::Syntax::REGULAR_XML || reportTypeInfo.m_syntax == TR::ReportTypeInfo::Syntax::CHECKLIST_XML)
            {
                m_inputs.push_back(reportTypeInfo);
            }
        }
        m_checkedInputs.resize(m_inputs.size(), false);

        init(std::make_unique<CommonPropertyPage>(
            CommonProperty(L"Kind of report", L"Select what kind of report you want create",
                SelectorProperty(L"", L"",
                    {L"Query", L"Filter", L"Grouping", L"Validating"},
                    refValue(m_kindOfReport))
            ),
            KIND_OF_REPORT          
        ));     
    }

    virtual std::unique_ptr<PropertyPageWidget> nextPage(int currentPageIdentity) override
    {
        switch (currentPageIdentity)
        {
        case KIND_OF_REPORT:
            switch (m_kindOfReport)
            {
            case QUERY:
                return createQueryInputPage();

            case FILTER:
                return createFilterInputPage();

            case GROUPING:
                return createGroupingInputPage();

            case VALIDATING:
                return createValidatingInputPage();

            default:
                _ASSERT(false);
            }
            break;

        case QUERY_INPUT:
            return createQueryFinishPage();

        case FILTER_INPUT:
            return createFilterFinishPage();

        case GROUPING_INPUT:
            return createGroupingFinishPage();

        case VALIDATING_INPUT:
            return createValidatingFinishPage();

        default:
            _ASSERT(false);
        }
        return nullptr;     
    }

    virtual bool isFinal(int currentPageIdentity) override
    {
        return currentPageIdentity == QUERY_FINISH ||
            currentPageIdentity == FILTER_FINISH ||
            currentPageIdentity == GROUPING_FINISH || 
            currentPageIdentity == VALIDATING_FINISH;
    }

    virtual void onFinish() override
    {
        switch (m_kindOfReport)
        {
        case QUERY: 
            createQuery();
            break;

        case FILTER:
            createFilter();
            break;

        case GROUPING:
            createGrouping();
            break;

        case VALIDATING:
            createValidation();
            break;

        default:
            _ASSERT(false);
        }
    }

    std::unique_ptr<PropertyPageWidget> createQueryInputPage()
    {
        return std::make_unique<CommonPropertyPage>(
            CommonProperty(L"Input reports", L"Select input reports for query",
                CheckListProperty(L"Report name", L"", stl_tools::copy_vector(m_inputs | stl_tools::members(&TR::ReportTypeInfo::m_name)),
                    containerArray(m_checkedInputs),
                    CHECKLIST_ATTRIBUTES(CHECK_REQUIRED)
                )
            ), QUERY_INPUT
        );
    }

    std::unique_ptr<PropertyPageWidget> createQueryFinishPage()
    {
        return std::make_unique<ContainerPage>(L"Last step", L"Click 'Finish' to open query editor. Then customize <br/> your query and click 'Apply' to persist.",
            std::make_unique<Wt::WContainerWidget>(),
            QUERY_FINISH
        );
    }

    std::unique_ptr<PropertyPageWidget> createFilterInputPage()
    {
        return std::make_unique<CommonPropertyPage>(
            CommonProperty(L"Filter", L"Select report to filter",
                SelectionListProperty(L"Report name", L"", stl_tools::copy_vector(m_inputs | stl_tools::members(&TR::ReportTypeInfo::m_name)),
                    refValue(m_selectedInput)
                )
            ), FILTER_INPUT
        );
    }

    std::unique_ptr<PropertyPageWidget> createFilterFinishPage()
    {
        return std::make_unique<ContainerPage>(L"Last step", L"Click 'Finish' to open filter editor. Then customize <br/> your filter and click 'Apply' to persist.",
            std::make_unique<Wt::WContainerWidget>(),
            FILTER_FINISH
        );
    }

    std::unique_ptr<PropertyPageWidget> createGroupingInputPage()
    {
        return std::make_unique<CommonPropertyPage>(
            CommonProperty(L"Grouping", L"Select report for grouping",
                SelectionListProperty(L"Report name", L"", stl_tools::copy_vector(m_inputs | stl_tools::members(&TR::ReportTypeInfo::m_name)),
                    refValue(m_selectedInput)
                )
            ), GROUPING_INPUT
        );
    }

    std::unique_ptr<PropertyPageWidget> createGroupingFinishPage()
    {
        return std::make_unique<CommonPropertyPage>(CommonProperty{L"Last step", L"Enter grouping report name and click 'Finish' to persist.",
                StringProperty(L"", L"", refValue(m_reportName))
            }, GROUPING_FINISH
        );
    }

    std::unique_ptr<PropertyPageWidget> createValidatingInputPage()
    {
        return std::make_unique<CommonPropertyPage>(
            CommonProperty(L"Validating", L"Select report to validate",
                SelectionListProperty(L"Report name", L"", stl_tools::copy_vector(m_inputs | stl_tools::members(&TR::ReportTypeInfo::m_name)),
                    refValue(m_selectedInput)
                )
            ), VALIDATING_INPUT
        );
    }

    std::unique_ptr<PropertyPageWidget> createValidatingFinishPage()
    {
        return std::make_unique<ContainerPage>(L"Last step", L"Click 'Finish' to open validation editor. Then setup <br/> conditions and click 'Apply' to persist.",
            std::make_unique<Wt::WContainerWidget>(),
            VALIDATING_FINISH
        );
    }

    void createQuery()
    {
        std::vector<TR::ReportTypeInfo> queryInputs;
        for (size_t i = 0; i < m_checkedInputs.size(); ++i)
        {
            if (m_checkedInputs[i])
            {
                queryInputs.push_back(m_inputs[i]);
            }
        }
            
        Application::pushPage(std::make_unique<CreateQueryPage>(std::move(queryInputs)));
    }

    void createFilter()
    {
        Application::pushPage(std::make_unique<CreateFilterPage>(m_inputs[m_selectedInput]));
    }

    void createGrouping()
    {       
        auto inputUUID = m_inputs[m_selectedInput].m_uuid;
        auto& client = Application::instance()->getClient();
        client.addGrouping(m_reportName, inputUUID);
        m_parentWidget->refresh();
    }

    void createValidation()
    {
        Application::pushPage(std::make_unique<CreateValidationPage>(m_inputs[m_selectedInput]));
    }

    ReportSelectionWidget* m_parentWidget;
    KindOfReport m_kindOfReport;
    std::vector<TR::ReportTypeInfo> m_inputs;
    std::vector<bool> m_checkedInputs;  
    int m_selectedInput;
    std::wstring m_reportName;
};

class ReportTypeList: public InventoryTable
{
public:
    ReportTypeList()
    {
        addTool("Setup", [this](int index){
            onSetupClick(index);
        }, true);

        addTool("Modify", [this](int index){
            onModifyClick(index);
        }, false);

        addTool("Remove", [this](int index){
            onRemoveClick(index);
        }, false);
    }

    virtual void load() override
    {
        refresh();
    }

    void refresh()
    {
        auto& client = Application::instance()->getClient();
        m_reportTypesInfo = client.getReportTypesInfo();

        resize(m_reportTypesInfo.size());
        for (size_t i = 0; i < m_reportTypesInfo.size(); ++i)
        {
            setItem(i, m_reportTypesInfo[i].m_name, "Description");
        }
    }

    void onSetupClick(int index)
    {
        Application::showPage(std::make_unique<ReportOptionsPage>(m_reportTypesInfo[index]));
    }

    void onModifyClick(int index)
    {
        auto reportTypeInfo = m_reportTypesInfo[index];
        if (reportTypeInfo.m_custom_filter_key == 0)
        {
            auto propertyBox = new PropertyBox(Dialog::BTN_CLOSE, L"Invalid operation", L"Only custom filter can be modified this way.", Wt::Icon::Information);
            propertyBox->show();
            return;
        }

        auto& client = Application::instance()->getClient();
        auto customFilterInfo = client.getCustomFilterInfo(reportTypeInfo.m_custom_filter_key);
        Application::pushPage(std::make_unique<ModifyFilterPage>(customFilterInfo));
    }

    void onRemoveClick(int index)
    {
        auto reportTypeInfo = m_reportTypesInfo[index];
        if (reportTypeInfo.m_custom_query_key == 0 && reportTypeInfo.m_custom_filter_key == 0 && reportTypeInfo.m_validation_key == 0 && reportTypeInfo.m_grouping_key == 0)
        {
            auto propertyBox = new PropertyBox(Dialog::BTN_CLOSE, L"Invalid operation", L"Only custom query, filter or <br/>validator can be removed this way.", Wt::Icon::Information);
            propertyBox->show();
            return;
        }

        auto propertyBox = new PropertyBox(stl_tools::flag | Dialog::BTN_OK | Dialog::BTN_CANCEL, L"Remove report", L"Do you really want to remove report?", Wt::Icon::Question);
        propertyBox->addCheck(L"", L"Remove all dependent custom reports");
        propertyBox->show([this, reportTypeInfo, propertyBox]{
            auto& client = Application::instance()->getClient();
            client.removeCustomReport(reportTypeInfo.m_uuid, propertyBox->getValue<bool>(0));
            refresh();
        }); 
    }

private:
    std::vector<TR::ReportTypeInfo> m_reportTypesInfo;
};

ReportSelectionWidget::ReportSelectionWidget()
{
    createToolBar();

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    m_reportTypeList = vbox->addWidget(std::make_unique<ReportTypeList>());
}

void ReportSelectionWidget::refresh()
{
    m_reportTypeList->refresh();
}

void ReportSelectionWidget::createToolBar()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());

    auto createBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Create report...");
    createBtn->clicked().connect(std::bind(&ReportSelectionWidget::onCreateClick, this));
}

void ReportSelectionWidget::onCreateClick()
{
    auto createReportWizard = new CreateReportWizard(this);
    createReportWizard->show();
}

std::unique_ptr<MainFramePage> ReportSelectionPage::createPage(const std::string& url)
{
    if (url == "/options/reports")
    {
        return std::make_unique<ReportSelectionPage>();
    }
    return nullptr;
}

ReportSelectionPage::ReportSelectionPage():
    MainFramePage(L"Reports", "/options/reports")
{
    m_path = OptionsPage().getPath();
    m_path.push_back({m_caption, m_url});
}

MainFrameWidget* ReportSelectionPage::getWidget()
{
    if (!m_widget)
    {
        m_widget.reset(new ReportSelectionWidget());
    }
    return m_widget.get();
}