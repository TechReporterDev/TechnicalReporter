#include "stdafx.h"
#include "RegularDocEditor.h"
#include "RegularDocModel.h"
#include "CommonPropertyPage.h"
#include "PropertyDialog.h"
#include "RefValue.h"
#include "Application.h"

namespace {
class EditValueDlg: public PropertyDialog
{
public:
    EditValueDlg(RegularTrivialNodeRow& nodeRow):
        PropertyDialog("Edit Value"),
        m_nodeRow(nodeRow),
        m_string(nodeRow.getValue())
    {
        setWidth(300);
        setButtons(stl_tools::flag | BTN_OK | BTN_CANCEL);

        auto& regNode = nodeRow.getRegNode();
        auto& defNode = regNode.get_bound_def();        

        init(std::make_unique<CommonPropertyPage>(CommonProperty{L"", L"", 
            StringProperty(defNode.get_caption(), defNode.get_description(), refValue(m_string))
        }, 0));
    }

protected:
    //override Dialog
    virtual void apply()
    {
        m_nodeRow.setValue(m_string);
    }

private:
    RegularTrivialNodeRow& m_nodeRow;
    std::wstring m_string;
};

class RegularDocEditorDelegate: public Wt::WAbstractItemDelegate
{
public:
    RegularDocEditorDelegate()
    {
    }

    std::unique_ptr<Wt::WWidget> update(Wt::WWidget* widget, const Wt::WModelIndex& index, Wt::WFlags<Wt::ViewItemRenderFlag> flags)
    {
        if (widget)
        {
            //delete widget;
        }

        auto& baseRow = getRowFromIndex(index);
        auto& regNodeRow = dynamicDowncast<RegularNodeRow>(baseRow);
        auto regModel = regNodeRow.getRegularDocModel();
        auto& column = regModel->getColumn(index.column());
        
        auto itemWidget = std::make_unique<Wt::WContainerWidget>();
        itemWidget->addWidget(std::make_unique<Wt::WText>(column.getString(regNodeRow.getRegNode())));

        if (dynamic_cast<const RegularNameColumn*>(&column))
        {
            if (auto listNodeRow = dynamic_cast<RegularListNodeRow*>(&regNodeRow))
            {
                auto addBtn = itemWidget->addWidget(std::make_unique<Wt::WPushButton>("Add"));
                addBtn->setFloatSide(Wt::Side::Right);
                addBtn->setStyleClass("btn-xs");
                addBtn->clicked().connect([this, listNodeRow](Wt::WMouseEvent){
                    listNodeRow->addItemRow();
                });
            }

            if (auto parentRow = regNodeRow.getParentRow())
            {
                if (auto listNodeRow = dynamic_cast<RegularListNodeRow*>(parentRow))
                {
                    auto deleteBtn = itemWidget->addWidget(std::make_unique<Wt::WPushButton>("Delete"));
                    deleteBtn->setFloatSide(Wt::Side::Right);
                    deleteBtn->setStyleClass("btn-xs");
                    deleteBtn->clicked().connect([this, listNodeRow, &regNodeRow](Wt::WMouseEvent){
                        auto position = regNodeRow.getPosition();
                        listNodeRow->removeItemRow(position);
                    });

                    auto downBtn = itemWidget->addWidget(std::make_unique<Wt::WPushButton>("Down"));
                    downBtn->setFloatSide(Wt::Side::Right);
                    downBtn->setStyleClass("btn-xs");
                    downBtn->clicked().connect([this, listNodeRow, &regNodeRow](Wt::WMouseEvent){
                        auto position = regNodeRow.getPosition();
                        listNodeRow->moveItemRowDown(position);
                    });

                    auto upBtn = itemWidget->addWidget(std::make_unique<Wt::WPushButton>("Up"));
                    upBtn->setFloatSide(Wt::Side::Right);
                    upBtn->setStyleClass("btn-xs");
                    upBtn->clicked().connect([this, listNodeRow, &regNodeRow](Wt::WMouseEvent){
                        auto position = regNodeRow.getPosition();
                        listNodeRow->moveItemRowUp(position);
                    });                 
                }
            }
        }

        else if (dynamic_cast<const RegularValueColumn*>(&column))
        {
            if (auto trivialNodeRow = dynamic_cast<RegularTrivialNodeRow*>(&regNodeRow))
            {
                auto editBtn = itemWidget->addWidget(std::make_unique<Wt::WPushButton>("Edit"));
                editBtn->setFloatSide(Wt::Side::Right);
                editBtn->setStyleClass("btn-xs");
                editBtn->clicked().connect([this, trivialNodeRow](Wt::WMouseEvent){
                    auto dlg = new EditValueDlg(*trivialNodeRow);
                    dlg->show();
                });
            }
        }       

        return itemWidget;
    }
};

std::unique_ptr<RegularDocModel> createEditorModel(TR::XML::XmlRegularDoc& doc)
{
    auto model = std::make_unique<RegularDocModel>(doc);
    model->addColumn(std::make_unique<RegularNameColumn>());
    model->addColumn(std::make_unique<RegularValueColumn>());
    model->addColumn(std::make_unique<RegularDescriptionColumn>());
    return model;
}
} // namespace {

RegularDocEditor::RegularDocEditor(TR::XML::XmlRegularDoc& doc):
    RegularDocTreeView(createEditorModel(doc))
{
    setItemDelegate(std::make_shared<RegularDocEditorDelegate>());
    setColumnWidth(0, 400);
    setColumnWidth(1, 400);
    setColumnWidth(2, 400);
    setRowHeight(23);
    setLineHeight(23);
}