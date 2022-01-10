#include "stdafx.h"
#include "CommonPropertyTable.h"
#include "Application.h"
#undef min
namespace {

class CheckArrayColumn: public StandardTableColumn<size_t>
{
public:
    CheckArrayColumn(PropertyArray<CheckPropertyFactory> checkArray):
        m_checkArray(std::move(checkArray))
    {
    }

    virtual std::wstring getName() const
    {
        return m_checkArray.getFactory().getName();
    }

    virtual std::wstring getString(const StandardModelRow<size_t>& row) const override
    {
        return boost::lexical_cast<std::wstring>(m_checkArray.getProperty(row.getData()).getChecked());
    }

    virtual void create(const StandardModelRow<size_t>& row, Wt::WContainerWidget& container, Wt::WFlags<Wt::ViewItemRenderFlag> flags)
    {
        container.setLineHeight(30);
        container.setMargin(0);
        container.setPadding(5);

        auto checkProperty = m_checkArray.getProperty(row.getData());

        auto checkBox = container.addWidget(std::make_unique<Wt::WCheckBox>());
        checkBox->setHeight(30);
        checkBox->setChecked(checkProperty.getChecked());

        checkBox->changed().connect([checkBox, checkProperty](Wt::NoClass) mutable {
            checkProperty.setChecked(checkBox->isChecked());
        });
    }

private:
    PropertyArray<CheckPropertyFactory> m_checkArray;
};

class NumberArrayColumn: public StandardTableColumn<size_t>
{
public:
    NumberArrayColumn(PropertyArray<NumberPropertyFactory> numberArray):
        m_numberArray(std::move(numberArray))
    {
    }

    virtual std::wstring getName() const
    {
        return m_numberArray.getFactory().getName();
    }

    virtual std::wstring getString(const StandardModelRow<size_t>& row) const override
    {
        return boost::lexical_cast<std::wstring>(m_numberArray.getProperty(row.getData()).getNumber());
    }

    virtual void create(const StandardModelRow<size_t>& row, Wt::WContainerWidget& container, Wt::WFlags<Wt::ViewItemRenderFlag> flags)
    {
        container.setLineHeight(30);
        container.setMargin(0);
        container.setPadding(5);

        auto numberProperty = m_numberArray.getProperty(row.getData());
        if (numberProperty.isReadOnly())
        {
            container.addWidget(std::make_unique<Wt::WText>(boost::lexical_cast<std::wstring>(numberProperty.getNumber())));
            return;
        }

        auto edit = container.addWidget(std::make_unique<Wt::WLineEdit>(boost::lexical_cast<std::wstring>(numberProperty.getNumber())));
        edit->setValidator(std::make_unique<Wt::WIntValidator>(0, 130));

        edit->changed().connect([edit, numberProperty](Wt::NoClass) mutable {
            numberProperty.setNumber(boost::lexical_cast<int>(edit->text()));
        });
    }

private:
    PropertyArray<NumberPropertyFactory> m_numberArray;
};

class StringArrayColumn: public StandardTableColumn<size_t>
{
public:
    StringArrayColumn(PropertyArray<StringPropertyFactory> stringArray):
        m_stringArray(std::move(stringArray))
    {
    }

    virtual std::wstring getName() const
    {
        return m_stringArray.getFactory().getName();
    }

    virtual std::wstring getString(const StandardModelRow<size_t>& row) const override
    {
        return m_stringArray.getProperty(row.getData()).getString();
    }

    virtual void create(const StandardModelRow<size_t>& row, Wt::WContainerWidget& container, Wt::WFlags<Wt::ViewItemRenderFlag> flags)
    {
        container.setLineHeight(30);
        container.setMargin(0);
        container.setPadding(5);

        auto stringProperty = m_stringArray.getProperty(row.getData());
        if (stringProperty.isReadOnly())
        {
            container.addWidget(std::make_unique<Wt::WText>(stringProperty.getString()));
            return;
        }

        auto edit = container.addWidget(std::make_unique<Wt::WLineEdit>(stringProperty.getString()));
        edit->changed().connect([edit, stringProperty](Wt::NoClass) mutable {
            stringProperty.setString(edit->text());
        });
    }   

private:
    PropertyArray<StringPropertyFactory> m_stringArray;
};

class SelectorArrayColumn: public StandardTableColumn<size_t>
{
public:
    SelectorArrayColumn(PropertyArray<SelectorPropertyFactory> selectorArray):
        m_selectorArray(std::move(selectorArray))
    {
    }

    virtual std::wstring getName() const
    {
        return m_selectorArray.getFactory().getName();
    }

    virtual std::wstring getString(const StandardModelRow<size_t>& row) const override
    {
        auto selectorProperty = m_selectorArray.getProperty(row.getData());
        return selectorProperty.getItems()[selectorProperty.getSelection()];
    }

    virtual void create(const StandardModelRow<size_t>& row, Wt::WContainerWidget& container, Wt::WFlags<Wt::ViewItemRenderFlag> flags)
    {
        container.setLineHeight(30);
        container.setMargin(0);
        container.setPadding(5);

        auto selectorProperty = m_selectorArray.getProperty(row.getData());

        auto comboBox = container.addWidget(std::make_unique<Wt::WComboBox>());
        comboBox->addStyleClass("tr-inplace-combobox");
        comboBox->setHeight(30);
        for (auto& item : selectorProperty.getItems())
        {
            comboBox->addItem(item);
        }
        comboBox->setCurrentIndex(selectorProperty.getSelection());

        comboBox->changed().connect([comboBox, selectorProperty](Wt::NoClass) mutable{
            selectorProperty.setSelection(comboBox->currentIndex());
        });     
    }

private:
    PropertyArray<SelectorPropertyFactory> m_selectorArray;
};

class DynamicSelectorArrayColumn: public StandardTableColumn<size_t>
{
public:
    DynamicSelectorArrayColumn(PropertyArray<DynamicSelectorPropertyFactory> selectorArray):
        m_selectorArray(std::move(selectorArray))
    {
    }

    virtual std::wstring getName() const
    {
        return m_selectorArray.getFactory().getName();
    }

    virtual std::wstring getString(const StandardModelRow<size_t>& row) const override
    {
        auto selectorProperty = m_selectorArray.getProperty(row.getData());
        return selectorProperty.getItems()[selectorProperty.getSelection()];
    }

    virtual void create(const StandardModelRow<size_t>& row, Wt::WContainerWidget& container, Wt::WFlags<Wt::ViewItemRenderFlag> flags)
    {
        container.setLineHeight(30);
        container.setMargin(0);
        container.setPadding(5);

        auto selectorProperty = m_selectorArray.getProperty(row.getData());

        auto comboBox = container.addWidget(std::make_unique<Wt::WComboBox>());
        comboBox->addStyleClass("tr-inplace-combobox");
        comboBox->setHeight(30);
        for (auto& item : selectorProperty.getItems())
        {
            comboBox->addItem(item);
        }
        comboBox->setCurrentIndex(selectorProperty.getSelection());

        comboBox->changed().connect([comboBox, selectorProperty](Wt::NoClass) mutable {
            selectorProperty.setSelection(comboBox->currentIndex());
        });
    }

private:
    PropertyArray<DynamicSelectorPropertyFactory> m_selectorArray;
};

class AnyArrayColumn: public StandardTableColumn<size_t>
{
public:
    AnyArrayColumn(PropertyArray<AnyPropertyFactory> anyArray, CommonPropertyTable::AnyPresenter anyPresenter):
        m_anyArray(std::move(anyArray)),
        m_anyPresenter(anyPresenter)
    {
    }

    virtual std::wstring getName() const
    {
        return m_anyArray.getFactory().getName();
    }

    virtual std::wstring getString(const StandardModelRow<size_t>& row) const override
    {
        return L"";
    }

    virtual void create(const StandardModelRow<size_t>& row, Wt::WContainerWidget& container, Wt::WFlags<Wt::ViewItemRenderFlag> flags)
    {
        container.setLineHeight(30);
        container.setMargin(0);
        container.setPadding(5);

        auto anyProperty = m_anyArray.getProperty(row.getData());
        if (m_anyPresenter)
        {
            container.addWidget(m_anyPresenter(anyProperty));
            return;
        }
        container.addWidget(std::make_unique<Wt::WText>(""));
    }

private:
    PropertyArray<AnyPropertyFactory> m_anyArray;
    CommonPropertyTable::AnyPresenter m_anyPresenter;
};


struct CreateTableColumn: boost::static_visitor<std::unique_ptr<StandardTableColumn<size_t>>>
{
    CreateTableColumn(CommonPropertyTable::AnyPresenter anyPresenter):
        m_anyPresenter(anyPresenter)
    {
    }

    std::unique_ptr<StandardTableColumn<size_t>> operator()(PropertyArray<CheckPropertyFactory>& checkArray) const
    {
        return std::make_unique<CheckArrayColumn>(checkArray);
    }

    std::unique_ptr<StandardTableColumn<size_t>> operator()(PropertyArray<NumberPropertyFactory>& numberArray) const
    {
        return std::make_unique<NumberArrayColumn>(numberArray);
    }

    std::unique_ptr<StandardTableColumn<size_t>> operator()(PropertyArray<StringPropertyFactory>& stringArray) const
    {
        return std::make_unique<StringArrayColumn>(stringArray);
    }

    std::unique_ptr<StandardTableColumn<size_t>> operator()(PropertyArray<SelectorPropertyFactory>& selectorArray) const
    {
        return std::make_unique<SelectorArrayColumn>(selectorArray);
    }

    std::unique_ptr<StandardTableColumn<size_t>> operator()(PropertyArray<DynamicSelectorPropertyFactory>& selectorArray) const
    {
        return std::make_unique<DynamicSelectorArrayColumn>(selectorArray);
    }

    std::unique_ptr<StandardTableColumn<size_t>> operator()(PropertyArray<AnyPropertyFactory>& anyProperty) const
    {
        return std::make_unique<AnyArrayColumn>(anyProperty, m_anyPresenter);
    }

    CommonPropertyTable::AnyPresenter m_anyPresenter;
};

} //namespace {


CommonPropertyTable::CommonPropertyTable(CommonPropertyGrid propertyGrid):
    m_propertyGrid(std::move(propertyGrid))
{
#ifdef _DEBUG
    auto colCount = m_propertyGrid.getColumnCount();
    _ASSERT(colCount > 0);

    auto rowCount = m_propertyGrid.getItemCount(0);
    for (size_t index = 0; index < colCount; ++index)
    {
        _ASSERT(m_propertyGrid.getItemCount(index) == rowCount);
    }
#endif //_DEBUG

    setAlternatingRowColors(true);
    setRowHeight(40);
    setHeaderHeight(28);
    setSelectionMode(Wt::SelectionMode::None);  
}

CommonPropertyTable::CommonPropertyTable(std::initializer_list<CommonPropertyGrid::Column> columns):
    CommonPropertyTable(CommonPropertyGrid(columns))
{
}

void CommonPropertyTable::setAnyPresenter(AnyPresenter anyPresenter)
{
    m_anyPresenter = anyPresenter;
}

CommonPropertyTable::AnyPresenter CommonPropertyTable::getAnyPresenter() const
{
    return m_anyPresenter;
}

void CommonPropertyTable::load()
{
    if (columnCount() > 0)
    {
        return; // must load only once
    }

    auto colCount = m_propertyGrid.getColumnCount();
    auto colWidth = std::min(size_t(200), 1200 / colCount);

    for (size_t index = 0; index < colCount; ++index)
    {
        auto column = m_propertyGrid.getColumn(index);
        addColumn(boost::apply_visitor(CreateTableColumn(m_anyPresenter), column));
        setColumnWidth(index, colWidth);
    }

    setColumnAlignment(0, Wt::AlignmentFlag::Center);
    setHeaderAlignment(0, Wt::AlignmentFlag::Center);

    auto rowCount = m_propertyGrid.getItemCount(0);
    for (size_t row = 0; row < rowCount; ++row)
    {
        addRow(row);
    }
}