#include "stdafx.h"
#include "XmlDiffModel.h"
#include "XmlDiffTreeView.h"
#include "Application.h"

class XmlDiffTreeDelegate: public Wt::WAbstractItemDelegate
{
public:
    virtual std::unique_ptr<Wt::WWidget> update(Wt::WWidget *widget, const Wt::WModelIndex& index, Wt::WFlags<Wt::ViewItemRenderFlag> flags)
    {
        if (widget)
        {
            delete widget;
        }

        auto& baseRow = getRowFromIndex(index);
        const auto& diffNodeRow = dynamicDowncast<XmlDiffNodeRow>(baseRow);

        auto nodeWidget = std::make_unique<Wt::WContainerWidget>();
        switch (diffNodeRow.getDiffState())
        {
            case XmlDiffNodeRow::STATE_NONE:
                break;

            case XmlDiffNodeRow::STATE_INSERTED:
                nodeWidget->setStyleClass("tr-inserted-cell");
                break;

            case XmlDiffNodeRow::STATE_REMOVED:
                nodeWidget->setStyleClass("tr-removed-cell");
                break;

            case XmlDiffNodeRow::STATE_MODIFIED:
                nodeWidget->setStyleClass("tr-modified-cell");
                break;

            default:
                _ASSERT(false);
        }

        auto model = diffNodeRow.getXmlDiffModel();
        auto& column = model->getColumn(index.column());
        nodeWidget->addWidget(std::make_unique<Wt::WText>(column.getString(diffNodeRow)));
        return nodeWidget;
    }
};

XmlDiffTreeView::XmlDiffTreeView(std::shared_ptr<XmlDiffModel> model):
    m_model(model)
{
    setModel(std::shared_ptr<Wt::WAbstractItemModel>(m_model, m_model.get()));  
    setItemDelegate(std::make_shared<XmlDiffTreeDelegate>());
    expandToDepth(3);
}

XmlDiffTreeView::~XmlDiffTreeView()
{
}