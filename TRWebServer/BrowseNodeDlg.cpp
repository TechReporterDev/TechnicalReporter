#include "stdafx.h"
#include "BrowseNodeDlg.h"
#include "StandardModel.h"
#include "Application.h"

class DefNodeCaptionColumn : public StandardModelColumn<const TR::XML::XmlDefNode*>
{
    virtual std::wstring getName() const override
    {
        return L"Caption";
    }

    virtual std::wstring getString(const StandardModelRow<const TR::XML::XmlDefNode*>& row) const override
    {
        return row.getData()->get_caption();
    }
};

class DefNodeDescriptionColumn : public StandardModelColumn<const TR::XML::XmlDefNode*>
{
    virtual std::wstring getName() const override
    {
        return L"Description";
    }

    virtual std::wstring getString(const StandardModelRow<const TR::XML::XmlDefNode*>& row) const override
    {
        return row.getData()->get_description();
    }
};

static std::unique_ptr<StandardModelRow<const TR::XML::XmlDefNode*>> createDefNodeRow(const TR::XML::XmlDefNode& defNode)
{
    auto defNodeRow = std::make_unique<StandardModelRow<const TR::XML::XmlDefNode*>>(&defNode);
    for (auto& childDefNode : get_child_defs(defNode))
    {
        defNodeRow->addChildRow(createDefNodeRow(childDefNode));
    }
    return defNodeRow;
}

static std::shared_ptr<StandardModel<const TR::XML::XmlDefNode*>> createDefDocModel(const TR::XML::XmlDefDoc& defDoc)
{
    auto model = std::make_shared<StandardModel<const TR::XML::XmlDefNode*>>();
    model->addColumn(std::make_shared<DefNodeCaptionColumn>());
    model->addColumn(std::make_shared<DefNodeDescriptionColumn>());
    model->addRow(createDefNodeRow(*defDoc.get_root_def()));
    return model;
}

StandardModelRow<const TR::XML::XmlDefNode*>* findRow(StandardModelRow<const TR::XML::XmlDefNode*>& row, const TR::XML::XmlDefNode* defNode)
{
    if (row.getData() == defNode)
    {
        return &row;
    }

    for (size_t position = 0; position < row.getChildCount(); ++position)
    {
        if (auto foundRow = findRow(row.getChildRow(position), defNode))
        {
            return foundRow;
        }
    }

    return nullptr;
}

StandardModelRow<const TR::XML::XmlDefNode*>& findRow(StandardModel<const TR::XML::XmlDefNode*>& model, const TR::XML::XmlDefNode* defNode)
{
    auto foundRow = findRow(model.getRow(0), defNode);
    _ASSERT(foundRow);
    return *foundRow;
}

BrowseNodeDlg::BrowseNodeDlg(const TR::XML::XmlDefDoc& defDoc, const TR::XML::XmlDefNode* selectedNode):
    m_defDoc(defDoc),
    m_selectedNode(selectedNode)
{
    setWindowTitle(L"Report nodes");
    setButtons(stl_tools::flag | BTN_OK | BTN_CANCEL);
    setWidth(400);
    setHeight(480);
    addStyleClass("tr-popup-level-2");

    contents()->setPadding(10);
    contents()->setPadding(0, Wt::Side::Bottom);
    auto vbox = contents()->setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->addWidget(std::make_unique<Wt::WText>(L"Select trivial report node"));

    auto browseTree = vbox->addWidget(std::make_unique<Wt::WTreeView>(), 1);
    browseTree->setSelectionMode(Wt::SelectionMode::Single);
    browseTree->setColumnWidth(0, 250);
    browseTree->setColumnWidth(1, 350);

    auto model = createDefDocModel(m_defDoc);
    browseTree->setModel(model);    
    browseTree->expandToDepth(3);

    if (m_selectedNode)
    {
        auto& selectedRow = findRow(*model, selectedNode);
        auto index = model->getIndexFromRow(selectedRow);
        browseTree->select(index);
        browseTree->scrollTo(index);
    }

    browseTree->selectionChanged().connect(std::bind([this, browseTree, model]{
        m_selectedNode = nullptr;
        for (auto index : browseTree->selectedIndexes())
        {
            m_selectedNode = dynamic_cast<StandardModelRow<const TR::XML::XmlDefNode*>&>(model->getRowFromIndex(index)).getData();
        }
    }));
}

void BrowseNodeDlg::show(std::function<void(const TR::XML::XmlDefNode*)> onApply)
{
    m_onApply = onApply;
    Dialog::show();
}

void BrowseNodeDlg::apply()
{
    if (m_onApply)
    {
        m_onApply(m_selectedNode);
    }
}