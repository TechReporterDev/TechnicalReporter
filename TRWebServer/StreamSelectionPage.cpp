#include "stdafx.h"
#include "StreamSelectionPage.h"
#include "StreamOptionsPage.h"
#include "StreamFilterPage.h"
#include "OptionsPage.h"
#include "PropertyWizard.h"
#include "CommonPropertyPage.h"
#include "RefValue.h"
#include "ContainerArray.h"
#include "InventoryTable.h"
#include "PropertyBox.h"
#include "BrowseNodeDlg.h"
#include "Application.h"

class CreateStreamWizard: public PropertyWizard
{
public:
    enum KindOfStream { FILTER, TRACKING };
    enum PageID { KIND_OF_STREAM, FILTER_INPUT, TRACKING_INPUT, TRACKING_NODE, FILTER_FINISH, TRACKING_FINISH };

    class SelectNodePage : public CommonPropertyPage
    {
    public:
        SelectNodePage(const TR::XmlDefDoc& filterDef, std::string& nodeXPath) :
            CommonPropertyPage(L"Node", L"Enter path to trivial node for tracking", PageID::TRACKING_NODE),
            m_filterDef(filterDef),
            m_nodeXPath(nodeXPath)
        {
            show({ L"", L"",
                BrowseProperty(L"", L"", L"Browse...", boost::any(), [&](boost::any) {
                    if (auto defNode = getDefNode())
                    {
                        return defNode->format_caption();
                    }
                    return std::wstring(L"Click `Browse...` to select node.");
                })
            });
        }

        virtual void handleEvent(boost::any) override
        {
            auto browseDlg = new BrowseNodeDlg(m_filterDef, getDefNode());
            browseDlg->show([this](const TR::XML::XmlDefNode* defNode) {
                if (defNode)
                {
                    m_nodeXPath = xml_tools::make_xpath(*defNode);
                }
                else
                {
                    m_nodeXPath.clear();
                }
                refresh();
            });
        }

        const TR::XML::XmlDefNode* getDefNode() const
        {
            if (m_nodeXPath.empty())
            {
                return nullptr;
            }
            return static_cast<const TR::XML::XmlDefNode*>(xml_tools::find_xpath_node(m_filterDef, m_nodeXPath.c_str()));
        }

    protected:
        virtual bool validate() override
        {
            if (auto defNode = getDefNode())
            {
                return defNode->is_trivial();
            }
            return false;
        }

    private:
        const TR::XmlDefDoc& m_filterDef;
        std::string& m_nodeXPath;
    };

    CreateStreamWizard(StreamSelectionWidget* parentWidget):
        PropertyWizard(L"Create stream"),
        m_kindOfStream(FILTER),
        m_parentWidget(parentWidget),
        m_selectedReport(0),
        m_selectedStream(0)
    {
        auto& client = Application::instance()->getClient();
        for (auto reportTypeInfo : client.getReportTypesInfo())
        {
            if (reportTypeInfo.m_syntax == TR::ReportTypeInfo::Syntax::REGULAR_XML)
            {
                m_inputReports.push_back(reportTypeInfo);
            }
        }

        for (auto streamTypeInfo : client.getStreamTypesInfo())
        {
            m_inputStreams.push_back(streamTypeInfo);
        }

        init(std::make_unique<CommonPropertyPage>(
            CommonProperty(L"Kind of stream", L"Select what kind of stream you want create",
                SelectorProperty(L"", L"",
                    {L"Filter", L"Tracking"},
                    refValue(m_kindOfStream))
            ),
            KIND_OF_STREAM          
        ));     
    }

    virtual std::unique_ptr<PropertyPageWidget> nextPage(int currentPageIdentity) override
    {       
        switch (currentPageIdentity)
        {
        case KIND_OF_STREAM:
            switch (m_kindOfStream)
            {
            case FILTER:
                return createFilterInputPage();

            case TRACKING:
                return createTrackingInputPage();

            default:
                _ASSERT(false);
            }
            break;

        case FILTER_INPUT:
            return createFilterFinishPage();

        case TRACKING_INPUT:
            return createSelectNodePage();

        case TRACKING_NODE:
            return createTrackingFinishPage();

        default:
            _ASSERT(false);
        }
        return nullptr;     
    }

    virtual bool isFinal(int currentPageIdentity) override
    {
        return currentPageIdentity == FILTER_FINISH || currentPageIdentity == TRACKING_FINISH;
    }

    virtual void onFinish() override
    {   
        switch (m_kindOfStream)
        {
        case FILTER:
            createFilter();
            break;

        case TRACKING:
            createTracking();
            break;

        default:
            _ASSERT(false);
        }
    }

    std::unique_ptr<PropertyPageWidget> createFilterInputPage()
    {
        return std::make_unique<CommonPropertyPage>(
            CommonProperty(L"Filter", L"Select stream to filter",
                SelectionListProperty(L"Stream name", L"", stl_tools::copy_vector(m_inputStreams | stl_tools::members(&TR::StreamTypeInfo::m_name)),
                    refValue(m_selectedStream)
                )
            ), FILTER_INPUT
        );
    }

    std::unique_ptr<PropertyPageWidget> createTrackingInputPage()
    {
        return std::make_unique<CommonPropertyPage>(
            CommonProperty(L"Tracking", L"Select report for tracking",
                SelectionListProperty(L"Report name", L"", stl_tools::copy_vector(m_inputReports | stl_tools::members(&TR::ReportTypeInfo::m_name)),
                    refValue(m_selectedReport)
                )
            ), TRACKING_INPUT
        );
    }

    std::unique_ptr<PropertyPageWidget> createSelectNodePage()
    {
        auto& client = Application::instance()->getClient();
        auto inputUUID = m_inputReports[m_selectedReport].m_uuid;
        m_inputDef = std::static_pointer_cast<const TR::XmlDefDoc>(client.getContentDef(inputUUID));
        return std::make_unique<SelectNodePage>(*m_inputDef, m_inputXPath);     
    }

    std::unique_ptr<PropertyPageWidget> createFilterFinishPage()
    {
        return std::make_unique<ContainerPage>(L"Last step", L"Click 'Finish' to open filter editor. Then customize <br/> your filter and click 'Apply' to persist.",
            std::make_unique<Wt::WContainerWidget>(),
            FILTER_FINISH
        );
    }

    std::unique_ptr<PropertyPageWidget> createTrackingFinishPage()
    {
        return std::make_unique<CommonPropertyPage>(CommonProperty{L"Last step", L"Enter tracking stream name and click 'Finish' to persist.",
                StringProperty(L"", L"", refValue(m_streamName))
            }, TRACKING_FINISH
        );
    }

    void createFilter()
    {
        Application::pushPage(std::make_unique<CreateStreamFilterPage>(m_inputStreams[m_selectedStream]));
    }

    void createTracking()
    {
        auto& client = Application::instance()->getClient();
        client.addTrackingStream(m_streamName, m_inputReports[m_selectedReport].m_uuid, m_inputXPath);
        m_parentWidget->refresh();
    }

    StreamSelectionWidget* m_parentWidget;
    KindOfStream m_kindOfStream;
    std::vector<TR::StreamTypeInfo> m_inputStreams;
    std::vector<TR::ReportTypeInfo> m_inputReports;
    int m_selectedReport;   
    int m_selectedStream;
    std::shared_ptr<const TR::XmlDefDoc> m_inputDef;
    std::string m_inputXPath;
    std::wstring m_streamName;  
};

class StreamTypeList: public InventoryTable
{
public:
    StreamTypeList()
    {
        addTool("Setup", [this](int index){
            onSetupClick(index);
        }, true);

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
        m_streamTypesInfo = client.getStreamTypesInfo();

        resize(m_streamTypesInfo.size());
        for (size_t i = 0; i < m_streamTypesInfo.size(); ++i)
        {
            setItem(i, m_streamTypesInfo[i].m_name, "Description");
        }
    }

    void onRemoveClick(int index)
    {
        auto streamTypeInfo = m_streamTypesInfo[index];     
        auto propertyBox = new PropertyBox(Dialog::BTN_CLOSE, L"Invalid operation", L"Only custom streams can be removed this way.", Wt::Icon::Information);
        propertyBox->show();        
    }

    void onSetupClick(int index)
    {
        Application::showPage(std::make_unique<StreamOptionsPage>(m_streamTypesInfo[index]));
    }

private:
    std::vector<TR::StreamTypeInfo> m_streamTypesInfo;
};

StreamSelectionWidget::StreamSelectionWidget()
{
    createToolBar();

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);
    
    m_streamTypeList = vbox->addWidget(std::make_unique<StreamTypeList>());
}

void StreamSelectionWidget::refresh()
{
    m_streamTypeList->refresh();
}

void StreamSelectionWidget::createToolBar()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());   

    auto createBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Create stream...");
    createBtn->clicked().connect(std::bind(&StreamSelectionWidget::onCreateClick, this));
}

void StreamSelectionWidget::onCreateClick()
{
    auto createStreamWizard = new CreateStreamWizard(this);
    createStreamWizard->show();
}

std::unique_ptr<MainFramePage> StreamSelectionPage::createPage(const std::string& url)
{
    if (url == "/options/streams")
    {
        return std::make_unique<StreamSelectionPage>();
    }
    return nullptr;
}

StreamSelectionPage::StreamSelectionPage():
    MainFramePage(L"Streams", "/options/streams")
{
    m_path = OptionsPage().getPath();
    m_path.push_back({m_caption, m_url});
}

MainFrameWidget* StreamSelectionPage::getWidget()
{
    if (!m_widget)
    {
        m_widget.reset(new StreamSelectionWidget());
    }
    return m_widget.get();
}