#include "stdafx.h"
#include "TextDiffTableView.h"
#include "Application.h"

class TextDiffTableDelegate: public Wt::WAbstractItemDelegate
{
public:
    TextDiffTableDelegate()     
    {
    }

    virtual std::unique_ptr<Wt::WWidget> update(Wt::WWidget *widget, const Wt::WModelIndex& index, Wt::WFlags<Wt::ViewItemRenderFlag> flags)
    {
        if (widget)
        {
            delete widget;
        }

        auto nodeWidget = std::make_unique<Wt::WContainerWidget>();
        auto& diffNodeRow = dynamic_cast<const TextDiffNodeRow&>(getRowFromIndex(index));

        switch (diffNodeRow.getDiffState())
        {
            case TextDiffNodeRow::STATE_NONE:
                break;

            case TextDiffNodeRow::STATE_ADDED:
                nodeWidget->setStyleClass("tr-inserted-cell");
                break;

            case TextDiffNodeRow::STATE_REMOVED:
                nodeWidget->setStyleClass("tr-removed-cell");
                break;

            default:
                _ASSERT(false);
        }

        if (flags.test(Wt::ViewItemRenderFlag::Selected))
        {
            nodeWidget->addStyleClass("active");
        }

        auto model = diffNodeRow.getDiffModel();
        auto& column = model->getColumn(index.column());
        nodeWidget->addWidget(std::make_unique<Wt::WText>(column.getString(diffNodeRow)));
        return nodeWidget;
    }
};

TextDiffTableView::TextDiffTableView(const std::wstring& oldText, const std::vector<TR::Core::DiffLine>& diffLines)
{
    m_model = std::make_shared<TextDiffModel>(oldText, diffLines);

    setModel(m_model);
    setItemDelegate(std::make_shared<TextDiffTableDelegate>());
    setColumnWidth(0, 350);
    setColumnWidth(1, 350);
    setSelectionMode(Wt::SelectionMode::Single);
    addStyleClass("tr-hide-row-splitter");
}

TextDiffTableView::~TextDiffTableView()
{
}

void TextDiffTableView::selectNext()
{
    auto next = MAX_ROW_POS;
    auto selected = getSelected();
    if (selected != MAX_ROW_POS)
    {
        next = m_model->getNextDiff(selected);
    }

    if (next == MAX_ROW_POS)
    {
        next = m_model->getFirstDiff();
    }

    if (next != MAX_ROW_POS)
    {
        auto index = getIndexFromRow(m_model->getDiffRow(next));
        select(index);      
        scrollTo(index/*, Wt::WAbstractItemView::PositionAtCenter*/);
    }
}

void TextDiffTableView::selectPrev()
{
    auto prev = MAX_ROW_POS;
    auto selected = getSelected();
    if (selected != MAX_ROW_POS)
    {
        prev = m_model->getPrevDiff(selected);
    }

    if (prev == MAX_ROW_POS)
    {
        prev = m_model->getLastDiff();
    }

    if (prev != MAX_ROW_POS)
    {
        auto index = getIndexFromRow(m_model->getDiffRow(prev));
        select(index);
        scrollTo(index/*, Wt::WAbstractItemView::PositionAtCenter*/);
    }
}

size_t TextDiffTableView::getSelected() const
{
    auto selected = selectedIndexes();
    if (selected.empty())
    {
        return MAX_ROW_POS;
    }

    return selected.begin()->row();
}