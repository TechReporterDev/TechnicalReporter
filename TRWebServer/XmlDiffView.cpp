#include "stdafx.h"
#include "XmlDiffView.h"
#include "XmlDiffTreeView.h"
#include "XmlDiffModel.h"
#include "Application.h"

XmlDiffView::XmlDiffView(std::shared_ptr<TR::XmlContent> old_content, std::shared_ptr<TR::XmlContent> new_content, std::shared_ptr<TR::PlainXmlDiff> diff):
    m_old_content(old_content),
    m_new_content(new_content),
    m_diff(diff)
{
    _ASSERT(old_content && new_content && diff);

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);

    auto model = std::make_unique<XmlDiffModel>(new_content->as_xml_doc(), old_content->as_xml_doc(), diff->get_diff_points());
    model->addColumn(std::make_unique<XmlDiffNameColumn>());
    model->addColumn(std::make_unique<XmlDiffNewValueColumn>());
    model->addColumn(std::make_unique<XmlDiffOldValueColumn>());

    auto treeView = vbox->addWidget(std::make_unique<XmlDiffTreeView>(std::move(model)));
    treeView->setColumnWidth(0, 350);
    treeView->setColumnWidth(1, 250);
    treeView->setColumnWidth(2, 250);   
}