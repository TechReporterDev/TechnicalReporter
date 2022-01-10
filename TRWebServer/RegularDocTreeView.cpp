#include "stdafx.h"
#include "RegularDocTreeView.h"
#include "RegularDocModel.h"
#include "Application.h"

class RegularDocTreeView::RegularNodeRowDelegate: public Wt::WAbstractItemDelegate
{
public:
    RegularNodeRowDelegate(RegularDocTreeView* parentView):
        m_parentView(parentView)
    {
    }

    virtual std::unique_ptr<Wt::WWidget> update(Wt::WWidget *widget, const Wt::WModelIndex& index, Wt::WFlags<Wt::ViewItemRenderFlag> flags)
    {
        if (widget)
        {
            delete widget;
        }

        auto& baseRow = getRowFromIndex(index);
        auto& regNodeRow = dynamicDowncast<RegularNodeRow>(baseRow);
        auto model = regNodeRow.getRegularDocModel();
        auto& column = model->getColumn(index.column());
        
        auto container = std::make_unique<Wt::WContainerWidget>();
        container->setStyleClass(m_parentView->getRowStyle(regNodeRow));
        container->addWidget(std::make_unique<Wt::WText>(column.getString(regNodeRow.getRegNode())));

        if (dynamic_cast<const RegularActionColumn*>(&column))
        {
            using namespace TR::XML::META;
            auto contextNode = &regNodeRow.getRegNode();
            auto& defNode = contextNode->get_bound_def();
            for (auto& metaNode : defNode.get_meta_nodes())
            {
                if (auto actionNode = cast_meta_node<XmlActionNode>(&metaNode))
                {
                    auto caption = actionNode->get_caption();
                    if (caption.empty())
                    {
                        continue;
                    }

                    auto actionBtn = container->addWidget(std::make_unique<Wt::WPushButton>(caption));
                    actionBtn->setStyleClass("tr-inplace-button");
                    actionBtn->clicked().connect([this, contextNode, actionNode](Wt::WMouseEvent){
                        m_parentView->onActionClick(contextNode, actionNode);
                    });
                }
            }
        }
        return container;
    }

private:
    RegularDocTreeView* m_parentView;
};

RegularDocTreeView::RegularDocTreeView(std::unique_ptr<RegularDocModel> model):
    XmlDocTreeView(STATIC_POINTER_CAST(XmlDocModel, std::move(model)))
{
    setItemDelegate(std::make_shared<RegularNodeRowDelegate>(this));
}

RegularDocTreeView::~RegularDocTreeView()
{
}

RegularDocModel* RegularDocTreeView::getRegularDocModel()
{
    return static_cast<RegularDocModel*>(getXmlDocModel());
}

void RegularDocTreeView::connectActionClick(OnActionClick onActionClick)
{
    m_onActionClick = onActionClick;
}

std::wstring RegularDocTreeView::getRowStyle(RegularNodeRow& regNodeRow) const
{
    return L"";
}

void RegularDocTreeView::onActionClick(const TR::XML::XmlRegularNode* context_node, const TR::XML::META::XmlActionNode* action_node)
{
    m_onActionClick(context_node, action_node);
}
