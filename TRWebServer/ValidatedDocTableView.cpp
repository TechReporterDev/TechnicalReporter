#include "stdafx.h"
#include "ValidatedDocTableView.h"
#include "Application.h"

ValidatedNodeTableView::ValidatedNodeTableView(ValidatedDocTableView& tableView):
    m_tableView(tableView)
{
}

std::unique_ptr<Wt::WWidget> ValidatedNodeTableView::update(Wt::WWidget* widget, const Wt::WModelIndex& index, Wt::WFlags<Wt::ViewItemRenderFlag> flags)
{
    if (widget)
    {
        //delete widget;
    }
    auto nodeWidget = std::make_unique<Wt::WContainerWidget>();

    auto validatedNode = static_cast<TR::XML::XmlValidatedNode*>(m_tableView.m_regularTable.m_rows[index.row()][index.column()]);
    if (validatedNode)
    {
        if (!validatedNode->is_valid())
        {
            nodeWidget->setStyleClass("tr-invalid-cell");
        }

        nodeWidget->addWidget(std::make_unique<Wt::WText>(xml_tools::get_node_text(*validatedNode)));
    }   

    return nodeWidget;
}

ValidatedDocTableView::ValidatedDocTableView(const TR::XML::XmlRegularDoc& doc):
    m_doc(doc),
    m_regularTable(TR::XML::make_regular_table(const_cast<TR::XML::XmlRegularDoc&>(doc))),
    m_validatedNodeView(std::make_shared<ValidatedNodeTableView>(*this))
{
    setItemDelegate(m_validatedNodeView);

    m_model = std::make_shared<Wt::WStandardItemModel>(m_regularTable.m_rows.size(), m_regularTable.m_header.size());
    for (size_t col = 0; col < m_regularTable.m_header.size(); col++)
    {
        m_model->setHeaderData(col, Wt::WString(m_regularTable.m_header[col]->get_caption()));
    }

    for (size_t row = 0; row < m_regularTable.m_rows.size(); row++)
    {
        for (size_t col = 0; col < m_regularTable.m_header.size(); col++)
        {
            auto regular_node = m_regularTable.m_rows[row][col];
            auto item = std::make_unique<Wt::WStandardItem>();
            item->setText(regular_node ? xml_tools::get_node_text(*regular_node) : "-");
            m_model->setItem(row, col, std::move(item));
        }
    }

    setModel(m_model);
}

ValidatedDocTableView::~ValidatedDocTableView()
{
}