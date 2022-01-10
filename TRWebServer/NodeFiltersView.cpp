#include "stdafx.h"
#include "NodeFiltersView.h"
#include "PropertyWizard.h"
#include "PropertyDialog.h"
#include "CommonPropertyPage.h"
#include "BrowseNodeDlg.h"
#include "CustomValue.h"
#include "StaticValue.h"
#include "InventoryTable.h"
#include "Application.h"

class CreateNodeFilterWizard: public PropertyWizard
{
public:
    enum PageID {NODE_PAGE, CONDITIONS_PAGE};
    
    class SelectNodePage: public CommonPropertyPage
    {
    public:
        SelectNodePage(const TR::XmlDefDoc& filterDef, std::string& nodeXPath):
            CommonPropertyPage(L"Node", L"Enter path to trivial node for filtration", NODE_PAGE),
            m_filterDef(filterDef),
            m_nodeXPath(nodeXPath)
        {
            show({L"", L"",
                BrowseProperty(L"", L"", L"Browse...", boost::any(), [&](boost::any){
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
            browseDlg->show([this](const TR::XML::XmlDefNode* defNode){
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

    CreateNodeFilterWizard(const TR::XmlDefDoc& filterDef):
        PropertyWizard(L"Create condition"),
        m_filterDef(filterDef)
    {
        init(std::make_unique<SelectNodePage>(m_filterDef, m_nodeFilter.m_xpath));
    }

    void show(std::function<void(TR::XmlNodeFilter nodeFilter)> onApply)
    {
        m_onApply = std::move(onApply);
        PropertyWizard::show();
    }

    virtual std::unique_ptr<PropertyPageWidget> nextPage(int currentPageIdentity) override
    {
        switch (currentPageIdentity)
        {
        case NODE_PAGE:
            return createConditionsPage();

        default:
            _ASSERT(false);
        }
        return nullptr;
    }

    virtual bool isFinal(int currentPageIdentity) override
    {
        return currentPageIdentity == CONDITIONS_PAGE;
    }

    virtual void onFinish() override
    {
        boost::split(m_nodeFilter.m_patterns, m_patterns, boost::is_any_of(L"\r\n"), boost::algorithm::token_compress_on);
        m_onApply(std::move(m_nodeFilter));
    }

    std::unique_ptr<PropertyPageWidget> createConditionsPage()
    {
        return std::make_unique<CommonPropertyPage>(CommonProperty{L"Parameters", L"Enter condition parameters", 
            SelectorProperty(L"Action", L"", {L"Accept", L"Reject"}, refValue(m_nodeFilter.m_action)),
            SelectorProperty(L"Predicate", L"", {L"Equal", L"Match", L"Less", L"More"}, refValue(m_nodeFilter.m_predicate)),
            TextProperty(L"Patterns", L"", 4, refValue(m_patterns))
        }, CONDITIONS_PAGE);
    }

private:
    TR::XmlNodeFilter m_nodeFilter;
    const TR::XmlDefDoc& m_filterDef;
    std::wstring m_patterns;
    std::function<void(TR::XmlNodeFilter m_nodeFilter)> m_onApply;
};

class EditNodeFilter: public PropertyDialog
{
public:
    EditNodeFilter(TR::XmlNodeFilter nodeFilter):
        PropertyDialog(L"Edit condition"),
        m_nodeFilter(std::move(nodeFilter))
    {
        setWidth(300);
        setHeight(480);

        for (auto& pattern : m_nodeFilter.m_patterns)
        {
            if (! m_patterns.empty())
            {
                m_patterns.append(L"\r\n");
            }
            m_patterns.append(pattern);
        }
        init( std::make_unique<CommonPropertyPage>(CommonProperty{L"Parameters", L"Enter condition parameters",
            SelectorProperty(L"Action", L"", {L"Accept", L"Reject"}, refValue(m_nodeFilter.m_action)),
            SelectorProperty(L"Predicate", L"", {L"Equal", L"Match", L"Less", L"More"}, refValue(m_nodeFilter.m_predicate)),
            TextProperty(L"Patterns", L"", 4, refValue(m_patterns))
        }, 0));
    }

    void show(std::function<void(TR::XmlNodeFilter nodeFilter)> onApply)
    {
        m_onApply = std::move(onApply);
        PropertyDialog::show();
    }

    virtual void apply() override
    {
        boost::split(m_nodeFilter.m_patterns, m_patterns, boost::is_any_of(L"\r\n"), boost::algorithm::token_compress_on);
        m_onApply(std::move(m_nodeFilter));
    }

private:
    TR::XmlNodeFilter m_nodeFilter;
    std::wstring m_patterns;
    std::function<void(TR::XmlNodeFilter m_nodeFilter)> m_onApply;
};

NodeFiltersView::NodeFiltersView(std::vector<TR::XmlNodeFilter> xmlNodeFilters, const TR::XmlDefDoc& filterDef, const TR::XmlRegularDoc* browseDoc):
    m_xmlNodeFilters(std::move(xmlNodeFilters)),
    m_filterDef(filterDef),
    m_browseDoc(browseDoc),
    m_conditions(nullptr)
{
    setPadding(5);
    setOverflow(Wt::Overflow::Auto);    

    m_conditions = addWidget(std::make_unique<InventoryTable>());
    m_conditions->setEmptyMessage(L"Click 'Add...' button to add first condition to filter");

    m_conditions->addTool("Move", [this](int index){
        m_conditions->setMoveMode([this, index](int target){
            if (index == target)
            {
                return;
            }
            moveNodeFilter(index, target);
        });
    });

    m_conditions->addTool("Edit", [this](int index){
        editNodeFilter(index);
    }, true);

    m_conditions->addTool("Delete", [this](int index){
        deleteNodeFilter(index);
    });

    size_t position = 0;
    for (auto& nodeFilter : m_xmlNodeFilters)
    {
        m_conditions->setItem(position++, formatXPath(nodeFilter, m_filterDef), formatCondition(nodeFilter));
    }
}

std::unique_ptr<Wt::WToolBar> NodeFiltersView::getViewBar()
{
    auto toolbar = std::make_unique<Wt::WToolBar>();
    auto addBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Add...");
    addBtn->clicked().connect([this](Wt::WMouseEvent) {
        addNodeFilter();
    });

    return toolbar;
}

std::vector<TR::XmlNodeFilter> NodeFiltersView::getNodeFilters() const
{
    return m_xmlNodeFilters;    
}

void NodeFiltersView::addNodeFilter()
{
    auto wizard = new CreateNodeFilterWizard(m_filterDef);
    wizard->show([this](TR::XmlNodeFilter nodeFilter){      
        m_xmlNodeFilters.insert(m_xmlNodeFilters.begin(), nodeFilter);
        m_conditions->insertItem(0, formatXPath(nodeFilter, m_filterDef), formatCondition(nodeFilter));
    });
}

void NodeFiltersView::moveNodeFilter(int from, int to)
{
    std::swap(m_xmlNodeFilters[from], m_xmlNodeFilters[to]);
    m_conditions->setItem(from, formatXPath(m_xmlNodeFilters[from], m_filterDef), formatCondition(m_xmlNodeFilters[from]));
    m_conditions->setItem(to, formatXPath(m_xmlNodeFilters[to], m_filterDef), formatCondition(m_xmlNodeFilters[to]));
}

void NodeFiltersView::editNodeFilter(int index)
{
    auto dlg = new EditNodeFilter(m_xmlNodeFilters[index]);
    dlg->show([this, index](TR::XmlNodeFilter nodeFilter){
        m_xmlNodeFilters[index] = nodeFilter;
        m_conditions->setItem(index, formatXPath(nodeFilter, m_filterDef), formatCondition(nodeFilter));
    });
}

void NodeFiltersView::deleteNodeFilter(int index)
{
    m_xmlNodeFilters.erase(m_xmlNodeFilters.begin() + index);
    m_conditions->deleteItem(index);
}