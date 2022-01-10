#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
class InventoryTable;

class NodeFiltersView: public Wt::WContainerWidget
{
public:
    NodeFiltersView(std::vector<TR::XmlNodeFilter> xmlNodeFilters, const TR::XmlDefDoc& filterDef, const TR::XmlRegularDoc* browseDoc);
    
    std::unique_ptr<Wt::WToolBar>   getViewBar();
    std::vector<TR::XmlNodeFilter>  getNodeFilters() const;

private:
    void addNodeFilter();
    void moveNodeFilter(int from, int to);
    void editNodeFilter(int index);
    void deleteNodeFilter(int index);

    std::vector<TR::XmlNodeFilter> m_xmlNodeFilters;
    const TR::XmlDefDoc& m_filterDef;
    const TR::XmlRegularDoc* m_browseDoc;
    InventoryTable* m_conditions;
    Wt::WText* m_emptyMessage;
};