#include "stdafx.h"
#include "CommonPropertyView.h"
#include "CommonPropertyArrayView.h"
#include "StandardTable.h"
#include "Application.h"

static const int EDIT_WIDTH = 250;
static const int TABLE_WIDTH = 300;
static const int TABLE_HEIGHT = 240;
static const int BUTTON_WIDTH = 80;

PropertyView::PropertyView()
{
}

DetailedPropertyView::DetailedPropertyView():
    m_layout(nullptr),
    m_nameLabel(nullptr)
{
    m_layout = setLayout(std::make_unique<Wt::WVBoxLayout>());
    m_layout->setSpacing(5);
    m_layout->setContentsMargins(0, 0, 0, 0);
}

void DetailedPropertyView::showTitle(const Wt::WString& name, const Wt::WString& description)
{
    if (!name.empty())
    {
        m_nameLabel = m_layout->addWidget(std::make_unique<Wt::WLabel>(name));
        m_nameLabel->setMargin(0);
    }

    if (!description.empty())
    {
        m_layout->addWidget(std::make_unique<Wt::WText>(description));
    }
}

CheckPropertyView::CheckPropertyView(const CheckProperty& checkProperty):
    m_checkProperty(checkProperty)
{
    showTitle(m_checkProperty.getName(), L"");

    auto checkBox = m_layout->addWidget(std::make_unique<Wt::WCheckBox>(m_checkProperty.getDescription())); 
    if (m_nameLabel)
    {
        m_nameLabel->setBuddy(checkBox);
    }

    checkBox->setChecked(m_checkProperty.getChecked());
    checkBox->setEnabled(m_checkProperty.isReadOnly() == false);
    checkBox->changed().connect(std::bind([this, checkBox]{
        m_checkProperty.setChecked(checkBox->isChecked());
        m_changed.emit();
    }));
}

CheckPropertyExView::CheckPropertyExView(const CheckPropertyEx& checkPropertyEx):   
    m_checkPropertyEx(checkPropertyEx),
    m_extensionView(nullptr)
{
    showTitle(m_checkPropertyEx.getName(), L"");

    auto checkBox = m_layout->addWidget(std::make_unique<Wt::WCheckBox>(m_checkPropertyEx.getDescription()));   
    if (m_nameLabel)
    {
        m_nameLabel->setBuddy(checkBox);
    }

    checkBox->setChecked(m_checkPropertyEx.getChecked());
    checkBox->setEnabled(m_checkPropertyEx.isReadOnly() == false);
    checkBox->changed().connect(std::bind([this, checkBox]{
        m_checkPropertyEx.setChecked(checkBox->isChecked());        
        m_changed.emit();
        showExtension();
    }));

    showExtension();
}

void CheckPropertyExView::showExtension()
{
    if (m_extensionView)
    {
        delete m_extensionView;
        m_extensionView = nullptr;
    }

    auto extension = m_checkPropertyEx.getExtension();
    if (auto extensionProperty = boost::any_cast<CommonProperty>(&extension))
    {
        m_extensionView = m_layout->addWidget(std::make_unique<CommonPropertyView>(
            *extensionProperty, CommonPropertyView::DEFAULT_ATTRIBUTES ^ CommonPropertyView::SHOW_HEADER));
    }
    else
    {
        _ASSERT(false);
    }
}

NumberPropertyView::NumberPropertyView(const NumberProperty& numberProperty):
    m_numberProperty(numberProperty)
{
    showTitle(m_numberProperty.getName(), m_numberProperty.getDescription());

    auto hbox = m_layout->addLayout(std::make_unique<Wt::WHBoxLayout>());

    auto valueEdit = hbox->addWidget(std::make_unique<Wt::WSpinBox>());
    if (m_nameLabel)
    {
        m_nameLabel->setBuddy(valueEdit);
    }

    valueEdit->setWidth(EDIT_WIDTH);
    valueEdit->setMinimumSize(EDIT_WIDTH, 0);
    valueEdit->setValue(m_numberProperty.getNumber());
    
    valueEdit->changed().connect([this, valueEdit](Wt::NoClass){
        m_numberProperty.setNumber(valueEdit->value());
        m_changed.emit();
    });

    hbox->addStretch(1);
}

StringPropertyView::StringPropertyView(const StringProperty& stringProperty):   
    m_stringProperty(stringProperty)
{
    showTitle(m_stringProperty.getName(), m_stringProperty.getDescription());

    auto hbox = m_layout->addLayout(std::make_unique<Wt::WHBoxLayout>());

    auto valueEdit = hbox->addWidget(std::make_unique<Wt::WLineEdit>(m_stringProperty.getString()));
    if (m_nameLabel)
    {
        m_nameLabel->setBuddy(valueEdit);
    }

    valueEdit->setWidth(EDIT_WIDTH);
    valueEdit->setMinimumSize(EDIT_WIDTH, 0);
    valueEdit->setEnabled(!m_stringProperty.isReadOnly());
    valueEdit->setEchoMode(m_stringProperty.isSecret() ? Wt::EchoMode::Password : Wt::EchoMode::Normal);
    valueEdit->setPlaceholderText(m_stringProperty.getPrompt());

    if (m_stringProperty.NotEmpty())
    {
        valueEdit->setValidator(std::make_shared<Wt::WValidator>());
    }

    valueEdit->textInput().connect([this, valueEdit](Wt::NoClass){
        m_stringProperty.setString(valueEdit->text());
        m_changed.emit();
    });

    hbox->addStretch(1);
}

bool StringPropertyView::validate(Wt::WString& message)
{
    if (m_stringProperty.NotEmpty() && m_stringProperty.getString().empty())
    {
        message = (boost::wformat(L"Field '%1%' can not be empty") % m_stringProperty.getName()).str();
        return false;
    }
    return true;
}

TextPropertyView::TextPropertyView(const TextProperty& textProperty):   
    m_textProperty(textProperty)
{
    showTitle(textProperty.getName(), textProperty.getDescription());

    auto hbox = m_layout->addLayout(std::make_unique<Wt::WHBoxLayout>());   
    hbox->setSpacing(0);

    auto textEdit = hbox->addWidget(std::make_unique<Wt::WTextArea>(m_textProperty.getText())); 

    if (m_nameLabel)
    {
        m_nameLabel->setBuddy(textEdit);
    }

    textEdit->setWidth(EDIT_WIDTH);
    textEdit->setMinimumSize(EDIT_WIDTH, 0);
    textEdit->setRows(textProperty.getLineCount());
    textEdit->setReadOnly(textProperty.isReadOnly());

    textEdit->changed().connect([this, textEdit](Wt::NoClass){
        m_textProperty.setText(textEdit->text());
        m_changed.emit();
    });

    hbox->addStretch(1);
}

SelectorPropertyView::SelectorPropertyView(const SelectorProperty& selectorProperty):
    m_selectorProperty(selectorProperty)
{
    showTitle(m_selectorProperty.getName(), m_selectorProperty.getDescription());

    auto hbox = m_layout->addLayout(std::make_unique<Wt::WHBoxLayout>());

    auto valueCombo = hbox->addWidget(std::make_unique<Wt::WComboBox>());
    if (m_nameLabel)
    {
        m_nameLabel->setBuddy(valueCombo);
    }

    for (auto& item : selectorProperty.getItems())
    {
        valueCombo->addItem(item);
    }
    valueCombo->setCurrentIndex(m_selectorProperty.getSelection());
    valueCombo->setWidth(EDIT_WIDTH);
    valueCombo->setMinimumSize(EDIT_WIDTH, 0);
    valueCombo->changed().connect([this, valueCombo](Wt::NoClass){
        m_selectorProperty.setSelection(valueCombo->currentIndex());
        m_changed.emit();
    });
    valueCombo->setEnabled(m_selectorProperty.isReadOnly() == false);
    hbox->addStretch(1);
}

SelectorPropertyExView::SelectorPropertyExView(const SelectorPropertyEx& selectorPropertyEx):
    m_selectorPropertyEx(selectorPropertyEx),
    m_expandButton(nullptr)
{
    showTitle(m_selectorPropertyEx.getName(), m_selectorPropertyEx.getDescription());

    auto hbox = m_layout->addLayout(std::make_unique<Wt::WHBoxLayout>());

    auto valueCombo = hbox->addWidget(std::make_unique<Wt::WComboBox>());
    if (m_nameLabel)
    {
        m_nameLabel->setBuddy(valueCombo);
    }

    for (auto& item : m_selectorPropertyEx.getItems())
    {
        valueCombo->addItem(item);
    }
    valueCombo->setCurrentIndex(m_selectorPropertyEx.getSelection());
    valueCombo->setWidth(EDIT_WIDTH);
    valueCombo->setMinimumSize(EDIT_WIDTH, 0);
    valueCombo->changed().connect([this, valueCombo](Wt::NoClass){
        m_selectorPropertyEx.setSelection(valueCombo->currentIndex());
        m_changed.emit();
    });
    valueCombo->setEnabled(m_selectorPropertyEx.isReadOnly() == false);

    m_expandButton = hbox->addWidget(std::make_unique<Wt::WPushButton>(m_selectorPropertyEx.getExpandCaption()));
    m_expandButton->setWidth(BUTTON_WIDTH);
    m_expandButton->setMinimumSize(BUTTON_WIDTH, 0);
    m_expandButton->clicked().connect([this](Wt::WMouseEvent){
        m_event.emit(m_selectorPropertyEx.expand());
    });
    hbox->addStretch(1);
}

SwitchPropertyView::SwitchPropertyView(const SwitchProperty& switchProperty):
    m_switchProperty(switchProperty)
{
    showTitle(m_switchProperty.getName(), m_switchProperty.getDescription());

    auto group = std::make_shared<Wt::WButtonGroup>();

    for (auto& case_ : switchProperty.getCases())
    {
        auto caseBtn = m_layout->addWidget(std::make_unique<Wt::WRadioButton>(case_));
        caseBtn->setMargin(6, Wt::WFlags<Wt::Side>() | Wt::Side::Top | Wt::Side::Bottom);
        group->addButton(caseBtn);
    }

    group->setSelectedButtonIndex(switchProperty.getCase());
    group->checkedChanged().connect(std::bind([this, group](){
        m_switchProperty.setCase(group->selectedButtonIndex());
        m_changed.emit();
    }));    
}

SwitchPropertyExView::SwitchPropertyExView(const SwitchPropertyEx& switchProperty): 
    m_switchProperty(switchProperty)
{
    showTitle(m_switchProperty.getName(), m_switchProperty.getDescription());

    auto group = std::make_shared<Wt::WButtonGroup>();

    for (auto& case_ : switchProperty.getCases())
    {
        auto hbox = m_layout->addLayout(std::make_unique<Wt::WHBoxLayout>());
        hbox->setContentsMargins(0, 0, 0, 0);

        auto caseBtn = hbox->addWidget(std::make_unique<Wt::WRadioButton>(case_.m_name));
        caseBtn->setMargin(6/*, Wt::WFlags<Wt::Side>() | Wt::Side::Top | Wt::Side::Bottom*/);
        group->addButton(caseBtn);

        if (!boost::any_cast<boost::none_t>(&case_.m_eventId))
        {
            auto button = hbox->addWidget(std::make_unique<Wt::WPushButton>(case_.m_eventName));            

            //button->setWidth(BUTTON_WIDTH);
            //button->setMinimumSize(BUTTON_WIDTH, 0);
            button->clicked().connect([this, case_](Wt::WMouseEvent){
                m_event.emit(case_.m_eventId);
            });         
        }
        hbox->addStretch(1);
    }

    group->setSelectedButtonIndex(switchProperty.getCase());
    group->checkedChanged().connect(std::bind([this, group](){
        m_switchProperty.setCase(group->selectedButtonIndex());
        m_changed.emit();
    }));
}

SwitchCommonPropertyView::SwitchCommonPropertyView(const SwitchCommonProperty& switchProperty):
    m_switchProperty(switchProperty)
{
    showTitle(m_switchProperty.getName(), m_switchProperty.getDescription());

    auto group = std::make_shared<Wt::WButtonGroup>();

    for (auto& case_ : m_switchProperty.getCases())
    {
        auto caseBtn = m_layout->addWidget(std::make_unique<Wt::WRadioButton>(std::get<0>(case_)));
        caseBtn->setMargin(6, Wt::WFlags<Wt::Side>() | Wt::Side::Top | Wt::Side::Bottom);
        group->addButton(caseBtn);      
        
        auto caseContainer = m_layout->addWidget(std::make_unique<Wt::WContainerWidget>());
        m_caseContainers.push_back(caseContainer);
    
        showCaseView(m_caseContainers.size() - 1);
    }

    group->setSelectedButtonIndex(switchProperty.getCase());
    group->checkedChanged().connect([this, group](Wt::WRadioButton* caseBtn){
        int prevCase = m_switchProperty.getCase();
        int nextCase = group->id(caseBtn);
        m_switchProperty.setCase(nextCase);
        showCaseView(prevCase);
        showCaseView(nextCase);
        m_changed.emit();
    });
}

void SwitchCommonPropertyView::showCaseView(int position)
{
    auto case_ = m_switchProperty.getCases()[position];
    auto commonProperty = m_switchProperty.getCase() == position ? std::get<1>(case_) : std::get<2>(case_);

    auto caseView = std::make_unique<CommonPropertyView>(commonProperty);
    caseView->connectEvent([this](boost::any any){
        m_event.emit(any);
    });

    m_caseContainers[position]->clear();
    m_caseContainers[position]->addWidget(std::move(caseView));
}

class CheckListColumn: public StandardTableColumn<size_t>
{
public:
    CheckListColumn(CheckListProperty& checkListProperty):
        m_checkListProperty(checkListProperty)
    {
    }

    virtual std::wstring getName() const override
    {
        return m_checkListProperty.getName();
    }

    virtual std::wstring getString(const StandardModelRow<size_t>& row) const override
    {
        return m_checkListProperty.getItems()[row.getData()];
    }

    virtual std::unique_ptr<Wt::WWidget> create(const StandardModelRow<size_t>& row, Wt::WFlags<Wt::ViewItemRenderFlag> flags) override
    {
        auto index = row.getData();
        auto checkBox = std::make_unique<Wt::WCheckBox>(getString(row));
        checkBox->addStyleClass("tr-inplace-checkbox");
        checkBox->setChecked(m_checkListProperty.isChecked(index));

        checkBox->checked().connect([this, index](Wt::NoClass){
            m_checkListProperty.setChecked(index, true);
        });

        checkBox->unChecked().connect([this, index](Wt::NoClass){
            m_checkListProperty.setChecked(index, false);
        });

        return checkBox;
    }

private:
    CheckListProperty& m_checkListProperty;
};

CheckListPropertyView::CheckListPropertyView(const CheckListProperty& checkListProperty):
    m_checkListProperty(checkListProperty)
{
    auto hbox = m_layout->addLayout(std::make_unique<Wt::WHBoxLayout>());   

    auto checkList = hbox->addWidget(std::make_unique<StandardTable<size_t>>());
    
    checkList->setWidth(TABLE_WIDTH);
    checkList->setMinimumSize(TABLE_WIDTH, TABLE_HEIGHT);
    checkList->setHeight(TABLE_HEIGHT);
    checkList->addColumn(std::make_unique<CheckListColumn>(m_checkListProperty));
    checkList->setColumnWidth(0, 210);

    for (size_t i = 0; i < m_checkListProperty.getItems().size(); ++i)
    {
        checkList->addRow(i);
    }

    hbox->addStretch(1);
}

bool CheckListPropertyView::validate(Wt::WString& message)
{
    if (m_checkListProperty.checkRequired())
    {
        auto size = m_checkListProperty.getItems().size();
        for (size_t position = 0; position < size; ++position)
        {
            if (m_checkListProperty.isChecked(position))
            {               
                return true;
            }
        }
        message = L"No one item of '" + m_checkListProperty.getName() + L"' is checked";
        return false;
    }
    return true;
}

class SelectionListColumn: public StandardTableColumn<size_t>
{
public:
    SelectionListColumn(SelectionListProperty& selectionListProperty):
        m_selectionListProperty(selectionListProperty)
    {
    }

    virtual std::wstring getName() const override
    {
        return m_selectionListProperty.getName();
    }

    virtual std::wstring getString(const StandardModelRow<size_t>& row) const override
    {
        return m_selectionListProperty.getItems()[row.getData()];
    }

private:
    SelectionListProperty& m_selectionListProperty;
};

SelectionListPropertyView::SelectionListPropertyView(const SelectionListProperty& selectionListProperty):
    m_selectionListProperty(selectionListProperty)
{
    auto hbox = m_layout->addLayout(std::make_unique<Wt::WHBoxLayout>());

    auto selectionList = hbox->addWidget(std::make_unique<StandardTable<size_t>>());
    selectionList->setSelectionMode(Wt::SelectionMode::Single);
    selectionList->setWidth(TABLE_WIDTH);
    selectionList->setMinimumSize(TABLE_WIDTH, TABLE_HEIGHT);
    selectionList->setHeight(TABLE_HEIGHT);

    selectionList->selectionChanged().connect(std::bind([this, selectionList]{
        for (auto index : selectionList->selectedIndexes())
        {
            m_selectionListProperty.select(index.row());
        }
    }));

    selectionList->addColumn(std::make_unique<SelectionListColumn>(m_selectionListProperty));
    selectionList->setColumnWidth(0, 210);
    for (size_t i = 0; i < m_selectionListProperty.getItems().size(); ++i)
    {
        selectionList->addRow(i);
    }

    auto selected = m_selectionListProperty.getSelection();
    if (selected != SelectionListProperty::INVALID_SELECTION)
    {
        auto index = selectionList->model()->index(selected, 0);
        selectionList->select(index, Wt::SelectionFlag::Select);
    }

    hbox->addStretch(1);
}

bool SelectionListPropertyView::validate(Wt::WString& message)
{
    if (m_selectionListProperty.isSelectionRequired() && m_selectionListProperty.getSelection() == SelectionListProperty::INVALID_SELECTION)
    {
        message = L"One of items must be selected";
        return false;
    }
    return true;
}

DatePropertyView::DatePropertyView(const DateProperty& dateProperty):   
    m_dateProperty(dateProperty)
{
    showTitle(m_dateProperty.getName(), m_dateProperty.getDescription());

    auto hbox = m_layout->addLayout(std::make_unique<Wt::WHBoxLayout>());
    auto dateEdit = hbox->addWidget(std::make_unique<Wt::WDateEdit>());

    dateEdit->setWidth(EDIT_WIDTH);
    dateEdit->setMinimumSize(EDIT_WIDTH, 0);

    auto date = m_dateProperty.getDate();
    dateEdit->setDate(Wt::WDate(std::chrono::system_clock::from_time_t(mktime(&date))));

    dateEdit->changed().connect([this, dateEdit](Wt::NoClass){
        auto t = std::chrono::system_clock::to_time_t(dateEdit->date().toTimePoint());
        auto tm = *localtime(&t);
        m_dateProperty.setDate(tm);
        m_changed.emit();
    });

    hbox->addStretch(1);
}

class FixedTimeEdit: public Wt::WTimeEdit
{
protected:
    /*virtual void setFromTimePicker() override
    {
        Wt::WTimeEdit::setFromTimePicker();
        changed().emit(Wt::NoClass());
    }*/
};

TimePropertyView::TimePropertyView(const TimeProperty& timeProperty):
    m_timeProperty(timeProperty)
{
    showTitle(m_timeProperty.getName(), m_timeProperty.getDescription());

    auto hbox = m_layout->addLayout(std::make_unique<Wt::WHBoxLayout>());
    
    auto timeEdit = hbox->addWidget(std::make_unique<FixedTimeEdit>());
    timeEdit->setWidth(EDIT_WIDTH);
    timeEdit->setMinimumSize(EDIT_WIDTH, 0);

    auto tm = m_timeProperty.getTime();
    timeEdit->setTime(Wt::WTime(tm.tm_hour, tm.tm_min, tm.tm_sec));

    timeEdit->changed().connect([this, timeEdit](Wt::NoClass){
        auto wtime = timeEdit->time();
        auto tm = m_timeProperty.getTime();
        tm.tm_hour = wtime.hour();
        tm.tm_min = wtime.minute();
        tm.tm_sec = wtime.second();
        m_timeProperty.setTime(tm);
    });

    hbox->addStretch(1);
}

AnyPropertyView::AnyPropertyView(const AnyProperty& anyProperty):
    m_anyProperty(anyProperty)
{
    showTitle(m_anyProperty.getName(), m_anyProperty.getDescription());

    auto hbox = m_layout->addLayout(std::make_unique<Wt::WHBoxLayout>());

    auto expandBtn = hbox->addWidget(std::make_unique<Wt::WPushButton>(m_anyProperty.getCaption()));

    if (m_nameLabel)
    {
        m_nameLabel->setBuddy(expandBtn);
    }

    expandBtn->clicked().connect([this](Wt::WMouseEvent){
        m_event.emit(m_anyProperty.getAny());
    });

    hbox->addStretch(1);
}

BrowsePropertyView::BrowsePropertyView(const BrowseProperty& browseProperty):
    m_browseProperty(browseProperty),
    m_edit(nullptr)
{
    showTitle(m_browseProperty.getName(), m_browseProperty.getDescription());

    auto hbox = m_layout->addLayout(std::make_unique<Wt::WHBoxLayout>());

    m_edit = hbox->addWidget(std::make_unique<Wt::WLineEdit>(m_browseProperty.formatString()));
    m_edit->setEnabled(false);
    m_edit->setWidth(EDIT_WIDTH);
    m_edit->setMinimumSize(EDIT_WIDTH, 0);

    if (m_nameLabel)
    {
        m_nameLabel->setBuddy(m_edit);
    }

    auto browseButton = hbox->addWidget(std::make_unique<Wt::WPushButton>(m_browseProperty.getCaption()));  
    browseButton->setEnabled(browseProperty.isReadOnly() == false);
    browseButton->setWidth(BUTTON_WIDTH);
    browseButton->setMinimumSize(BUTTON_WIDTH, 0);
    browseButton->clicked().connect([this](Wt::WMouseEvent){
        m_event.emit(m_browseProperty.getSubject());
    });
    hbox->addStretch(1);
}

void BrowsePropertyView::refresh()
{
    m_edit->setText(m_browseProperty.formatString());
}

OptionalPropertyView::OptionalPropertyView(const OptionalProperty& optionalProperty):
    m_optionalProperty(optionalProperty),
    m_optionalView(nullptr)
{
    if (m_optionalProperty.isReady())
    {
        m_optionalView = addWidget(std::make_unique<CommonPropertyView>(m_optionalProperty.getInnerProperty()));
        m_optionalView->connectChanged([this] {
            m_changed.emit();
        });

        m_optionalView->connectEvent([this](boost::any any) {
            m_event.emit(any);
        });
    }
}
void OptionalPropertyView::refresh()
{
    if (m_optionalProperty.isReady())
    {
        if (!m_optionalView)
        {
            m_optionalView = addWidget(std::make_unique<CommonPropertyView>(m_optionalProperty.getInnerProperty()));
            m_optionalView->connectChanged([this] {
                m_changed.emit();
            });

            m_optionalView->connectEvent([this](boost::any any) {
                m_event.emit(any);
            });
        }
    }
    else
    {
        if (m_optionalView)
        {
            removeWidget(m_optionalView);
            m_optionalView = nullptr;
        }
    }

    if (m_optionalView)
    {
        m_optionalView->refresh();
    }
}

const CommonPropertyView::Attributes CommonPropertyView::DEFAULT_ATTRIBUTES = Attributes() | SHOW_HEADER | EXPAND_SUBPROPS;

struct CreatePropertyView: boost::static_visitor<std::unique_ptr<PropertyView>>
{
    CreatePropertyView(CommonPropertyView::Attributes parentAttributes):
        m_parentAttributes(parentAttributes)
    {
    }

    std::unique_ptr<PropertyView> operator()(const CheckProperty& checkProperty) const
    {
        return std::make_unique<CheckPropertyView>(checkProperty);
    }

    std::unique_ptr<PropertyView> operator()(const CheckPropertyEx& checkPropertyEx) const
    {
        return std::make_unique<CheckPropertyExView>(checkPropertyEx);
    }

    std::unique_ptr<PropertyView> operator()(const NumberProperty& numberProperty) const
    {
        return std::make_unique<NumberPropertyView>(numberProperty);
    }

    std::unique_ptr<PropertyView> operator()(const StringProperty& stringProperty) const
    {
        return std::make_unique<StringPropertyView>(stringProperty);
    }

    std::unique_ptr<PropertyView> operator()(const TextProperty& textProperty) const
    {
        return std::make_unique<TextPropertyView>(textProperty);
    }
    
    std::unique_ptr<PropertyView> operator()(const SelectorProperty& selectorProperty) const
    {
        return std::make_unique<SelectorPropertyView>(selectorProperty);
    }

    std::unique_ptr<PropertyView> operator()(const SelectorPropertyEx& selectorPropertyEx) const
    {
        return std::make_unique<SelectorPropertyExView>(selectorPropertyEx);
    }

    std::unique_ptr<PropertyView> operator()(const SwitchProperty& switchProperty) const
    {
        return std::make_unique<SwitchPropertyView>(switchProperty);
    }

    std::unique_ptr<PropertyView> operator()(const SwitchPropertyEx& switchPropertyEx) const
    {
        return std::make_unique<SwitchPropertyExView>(switchPropertyEx);
    }

    std::unique_ptr<PropertyView> operator()(const SwitchCommonProperty& switchCommonProperty) const
    {
        return std::make_unique<SwitchCommonPropertyView>(switchCommonProperty);
    }

    std::unique_ptr<PropertyView> operator()(const CheckListProperty& checkListProperty) const
    {
        return std::make_unique<CheckListPropertyView>(checkListProperty);
    }
    
    std::unique_ptr<PropertyView> operator()(const SelectionListProperty& selectionListProperty) const
    {
        return std::make_unique<SelectionListPropertyView>(selectionListProperty);
    }

    std::unique_ptr<PropertyView> operator()(const DateProperty& dateProperty) const
    {
        return std::make_unique<DatePropertyView>(dateProperty);
    }

    std::unique_ptr<PropertyView> operator()(const TimeProperty& timeProperty) const
    {
        return std::make_unique<TimePropertyView>(timeProperty);
    }

    std::unique_ptr<PropertyView> operator()(const BrowseProperty& browseProperty) const
    {
        return std::make_unique<BrowsePropertyView>(browseProperty);
    }

    std::unique_ptr<PropertyView> operator()(const OptionalProperty& optionalProperty) const
    {
        return std::make_unique<OptionalPropertyView>(optionalProperty);
    }

    std::unique_ptr<PropertyView> operator()(const CommonProperty& commonProperty) const
    {
        if(!m_parentAttributes.test(CommonPropertyView::EXPAND_SUBPROPS))
        {
            return std::make_unique<AnyPropertyView>(AnyProperty(commonProperty.getName(), commonProperty.getDescription(), L"expand",
                staticValue(boost::any(commonProperty))));          
        }
        return std::make_unique<CommonPropertyView>(commonProperty, CommonPropertyView::DEFAULT_ATTRIBUTES ^ CommonPropertyView::EXPAND_SUBPROPS);
    }

    std::unique_ptr<PropertyView> operator()(const AnyProperty& anyProperty) const
    {
        return std::make_unique<AnyPropertyView>(anyProperty);
    }

    std::unique_ptr<PropertyView> operator()(const PropertyArray<CommonPropertyFactory>& propertyArray) const
    {
        return std::make_unique<CommonPropertyArrayView>(propertyArray);
    }

    template<class T>
    std::unique_ptr<PropertyView> operator()(T&) const
    {
        _ASSERT(false);
        return nullptr;
    }

    CommonPropertyView::Attributes m_parentAttributes;
};

CommonPropertyView::CommonPropertyView(const CommonProperty& commonProperty, Attributes attributes)
{
    setOverflow(Wt::Overflow::Auto);

    if (attributes.test(SHOW_HEADER))
    {
        auto caption = commonProperty.getName();
        if (!caption.empty())
        {
            auto captionLabel = addWidget(std::make_unique<Wt::WLabel>(caption));
            captionLabel->setInline(false);
            auto& ds = captionLabel->decorationStyle();
            auto font = ds.font();
            font.setWeight(Wt::FontWeight::Bold);
            ds.setFont(font);
        }   

        auto description = commonProperty.getDescription();
        if (!description.empty())
        {
            auto descriptionText = addWidget(std::make_unique<Wt::WText>(description));
            descriptionText->setInline(false);
        }
    }

    for (size_t position = 0; position < commonProperty.size(); ++position)
    {
        auto propertyView = addWidget(visitMemberProperty(CreatePropertyView(attributes), commonProperty, position));
        propertyView->setMargin(10);

        propertyView->connectChanged([this]{
            m_changed.emit();
        });

        propertyView->connectEvent([this](boost::any any){
            m_event.emit(any);
        });
    }
}

void CommonPropertyView::refresh()
{
    for (auto widget : children())
    {
        widget->refresh();
    }
}

bool CommonPropertyView::validate(Wt::WString& message)
{
    for (auto widget : children())
    {
        if (auto propertyView = dynamic_cast<PropertyView*>(widget))
        {
            if ( ! propertyView->validate(message))
            {
                return false;
            }           
        }       
    }
    return true;
}