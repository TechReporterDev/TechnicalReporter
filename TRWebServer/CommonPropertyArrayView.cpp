#include "stdafx.h"
#include "CommonPropertyArrayView.h"
#include "PropertyDialog.h"
#include "CommonPropertyPage.h"
#include "SelfArray.h"
#include "Application.h"

struct GetPreviewString: boost::static_visitor<std::wstring>
{
    std::wstring operator()(const StringProperty& ppt) const
    {
        return ppt.getString();
    }

    std::wstring operator()(...) const
    {
        return L"";
    }
};

class EditItemDialog: public PropertyDialog
{
public:
    EditItemDialog(const Wt::WString& title, CommonProperty ppt, std::function<void(CommonProperty)> onApply, Wt::WObject* parent) :
        PropertyDialog(title, parent),
        m_ppt(std::move(ppt)),
        m_onApply(onApply)
    {
        init(std::make_unique<CommonPropertyPage>(m_ppt));
    }

    virtual void apply() override
    {
        m_onApply(m_ppt);
    }

private:
    CommonProperty m_ppt;
    std::function<void(CommonProperty)> m_onApply;  
};

class PropertyPreviewColumn: public StandardTableColumn<nullptr_t>
{
public:
    PropertyPreviewColumn(PropertyArray<CommonPropertyFactory>& propertyArray, size_t propertyPosition):
        m_propertyArray(propertyArray),
        m_propertyPosition(propertyPosition)
    {
    }

    virtual std::wstring getName() const
    {
        return m_propertyArray.getFactory().getFactory(m_propertyPosition).m_name;
    }

    virtual std::wstring getString(const StandardModelRow<nullptr_t>& row) const override
    {       
        auto rowProperty = m_propertyArray.getProperty(row.getPosition());
        return visitMemberProperty(GetPreviewString(), rowProperty, m_propertyPosition);
    }

private:
    PropertyArray<CommonPropertyFactory>& m_propertyArray;
    size_t m_propertyPosition;
};

CommonPropertyArrayView::CommonPropertyArrayView(const PropertyArray<CommonPropertyFactory>& propertyArray):
    m_propertyArray(propertyArray),
    m_previewTable(nullptr)
{
    setWidth(350);

    auto hbox = m_layout->addLayout(std::make_unique<Wt::WHBoxLayout>());

    hbox->addWidget(std::make_unique<Wt::WText>(propertyArray.getName()), 1);
    auto buttons = hbox->addWidget(std::make_unique<Wt::WToolBar>());   

    auto addBtn = Wt::addButton<Wt::WPushButton>(*buttons, "add");
    addBtn->setStyleClass("btn-xs");
    addBtn->clicked().connect([&](Wt::WMouseEvent){
        auto dlg = new EditItemDialog(L"Add Item", m_propertyArray.createProperty(), [this](CommonProperty ppt){
            auto position = m_propertyArray.size();
            m_propertyArray.addProperty(*ppt.shareValue(), position);
            m_previewTable->addRow(nullptr);
        }, this);
        dlg->show();
    });

    auto editBtn = Wt::addButton<Wt::WPushButton>(*buttons, "edit");
    editBtn->setStyleClass("btn-xs");
    editBtn->clicked().connect([&](Wt::WMouseEvent){
        auto selectedRows = m_previewTable->getSelectedRows();
        if (selectedRows.size() == 1)
        {
            showEditDlg(selectedRows[0]);
        }               
    });

    auto removeBtn = Wt::addButton<Wt::WPushButton>(*buttons, "remove");
    removeBtn->setStyleClass("btn-xs");
    removeBtn->clicked().connect([&](Wt::WMouseEvent){
        auto selectedRows = m_previewTable->getSelectedRows();
        for (size_t index = 0; index < selectedRows.size(); ++index)
        {
            auto position = selectedRows[index] - index;
            m_propertyArray.removeProperty(position);
            m_previewTable->removeRow(position);
        }       
    });

    m_previewTable = m_layout->addWidget(std::make_unique<StandardTable<nullptr_t>>());
    m_previewTable->setSelectionMode(Wt::SelectionMode::Extended);
    m_previewTable->setSortingEnabled(false);
    m_previewTable->setHeight(350);
    m_previewTable->doubleClicked().connect(std::bind([&](Wt::WModelIndex index){
        if (!index.isValid())
        {
            return;
        }

        showEditDlg(index.row());
    }, std::placeholders::_1));

    auto& factory = m_propertyArray.getFactory();
    for (size_t column = 0; column < factory.getFactoryCount(); ++column)
    {
        if (factory.getFactory(column).m_attributes.contains(PropertyAttribute::NO_PREVIEW) == false)
        {
            m_previewTable->addColumn(std::make_unique<PropertyPreviewColumn>(m_propertyArray, column));
        }       
    }

    auto array = m_propertyArray.shareArray();
    for (size_t row = 0; row < m_propertyArray.size(); ++row)
    {
        m_previewTable->addRow(nullptr);
    }
}

void CommonPropertyArrayView::showEditDlg(size_t row)
{
    auto target = m_propertyArray.getProperty(row);
    auto ppt = m_propertyArray.getFactory().createProperty(std::dynamic_pointer_cast<ValueCollection>(target.shareValue()->clone()));
    auto dlg = new EditItemDialog(L"Edit Item", ppt, [this, row](CommonProperty ppt){
        auto target = m_propertyArray.getProperty(row);
        target.shareValue()->assign(*ppt.shareValue());
        m_previewTable->updateRow(row, nullptr);
    }, this);
    dlg->show();
}