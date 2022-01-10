#include "stdafx.h"
#include "RegularDiffView.h"
#include "RegularDiffTreeView.h"
#include "RegularDiffModel.h"
#include "Application.h"

RegularDiffView::RegularDiffView(std::shared_ptr<TR::RegularContent> old_content, std::shared_ptr<TR::RegularContent> new_content, std::shared_ptr<TR::RegularDiff> diff):
    m_old_content(old_content),
    m_new_content(new_content),
    m_diff(diff)
{
    _ASSERT(old_content && new_content && diff);

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);

    auto model = std::make_unique<RegularDiffModel>(new_content->as_regular_doc(), old_content->as_regular_doc(), diff->get_diff_points());
    model->addColumn(std::make_unique<RegularDiffNameColumn>());
    model->addColumn(std::make_unique<RegularDiffNewValueColumn>());
    model->addColumn(std::make_unique<RegularDiffOldValueColumn>());
    model->addColumn(std::make_unique<RegularDiffDescriptionColumn>());

    auto treeView = vbox->addWidget(std::make_unique<RegularDiffTreeView>(std::move(model)));   
    treeView->setColumnWidth(0, 350);
    treeView->setColumnWidth(1, 250);
    treeView->setColumnWidth(2, 250);
    treeView->setColumnWidth(3, 350);
}