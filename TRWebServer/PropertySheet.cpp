#include "stdafx.h"
#include "PropertySheet.h"
#include "Dialog.h"
#include "Application.h"

static const int INVALID_PAGE_INDEX = -1;

class PropertySheetHeader: public Wt::WContainerWidget
{
public:
    PropertySheetHeader(PropertySheetWidget* parentSheet):
        m_parentSheet(parentSheet),
        m_backButton(nullptr),
        m_captionLabel(nullptr),
        m_descriptionText(nullptr)
    {
        setStyleClass(L"tr-header");
        setMinimumSize(0, 35);

        auto hbox = setLayout(std::make_unique<Wt::WHBoxLayout>());
        hbox->setContentsMargins(10, 0, 0, 0);
        hbox->setSpacing(5);

        m_backButton = hbox->addWidget(std::make_unique<Wt::WPushButton>("Back"));
        
        m_backButton->setMargin(5);
        m_backButton->setHidden(true);
        m_backButton->clicked().connect([this](Wt::WMouseEvent){
            m_parentSheet->prevPage();
        });

        auto vbox = hbox->addLayout(std::make_unique<Wt::WVBoxLayout>(), 1);
        vbox->setContentsMargins(0, 0, 0, 0);
        vbox->setSpacing(3);
        vbox->addStretch(1);

        m_captionLabel = vbox->addWidget(std::make_unique<Wt::WLabel>(""));
        m_captionLabel->addStyleClass("tr-caption");
        m_captionLabel->setHidden(true);

        m_descriptionText = vbox->addWidget(std::make_unique<Wt::WText>(""));
        m_descriptionText->setHidden(true);
        vbox->addStretch(1);
    }

    void setCaption(const Wt::WString& caption)
    {
        m_captionLabel->setText(caption);
        m_captionLabel->setHidden(caption.empty());
    }

    void setDescription(const Wt::WString& description)
    {
        m_descriptionText->setText(description);
        m_descriptionText->setHidden(description.empty());
    }

    void showBackButton(bool show = true)
    {
        m_backButton->setHidden(!show);
    }

private:
    PropertySheetWidget* m_parentSheet;
    Wt::WPushButton* m_backButton;
    Wt::WLabel* m_captionLabel;
    Wt::WText* m_descriptionText;
};

class PropertySheetBody: public Wt::WContainerWidget
{
public:
    PropertySheetBody(PropertySheetWidget* parentSheet):
        m_parentSheet(parentSheet),
        m_layout(nullptr),
        m_current(INVALID_PAGE_INDEX)
    {
        m_layout = setLayout(std::make_unique<Wt::WVBoxLayout>());
        m_layout->setContentsMargins(0, 0, 0, 0);
    }

    void pushPage(std::unique_ptr<PropertyPageWidget> propertyPage)
    {
        if (m_current != INVALID_PAGE_INDEX)
        {
            while (pageCount() - 1 > m_current)
            {
                delete getPage(m_current + 1);
            }
        }

        propertyPage->setPropertySheet(m_parentSheet);
        m_layout->addWidget(std::move(propertyPage));
        gotoPage(pageCount() - 1);
    }

    void nextPage()
    {
        if (m_current == INVALID_PAGE_INDEX || m_current == pageCount() - 1)
        {
            _ASSERT(false);
            return;
        }

        gotoPage(m_current + 1);
    }

    void prevPage()
    {
        if (m_current == INVALID_PAGE_INDEX || m_current == 0)
        {
            _ASSERT(false);
            return;
        }

        gotoPage(m_current - 1);
    }

    void gotoPage(int index)
    {
        _ASSERT(index < pageCount());
        if (m_current != INVALID_PAGE_INDEX)
        {
            getPage(m_current)->setHidden(true);
        }
        getPage(m_current = index)->setHidden(false);
    }

    PropertyPageWidget* firstPage() const
    {
        return getPage(0);
    }

    PropertyPageWidget* lastPage() const
    {
        return getPage(pageCount() - 1);
    }
     
    PropertyPageWidget* getPage() const
    {
        return getPage(m_current);
    }

    PropertyPageWidget* getPage(int index) const
    {
        if (index == INVALID_PAGE_INDEX)
        {
            return nullptr;
        }

        auto& pages = children();
        return static_cast<PropertyPageWidget*>(pages[index]);
    }

    int pageCount() const
    {
        return int(children().size());
    }

    void cut()
    {
        _ASSERT(m_current >= 0);
        while (pageCount() - 1 > m_current)
        {
            removeWidget(getPage(pageCount() - 1));
        }
    }

private:
    PropertySheetWidget* m_parentSheet;
    Wt::WVBoxLayout* m_layout;
    int m_current;
};

const PropertySheetWidget::Attributes PropertySheetWidget::DEFAULT_ATTRIBUTES = Attributes() | SHOW_HEADER | SHOW_BUTTONS | ENABLE_VALIDATION;

PropertySheetWidget::PropertySheetWidget(Attributes attributes):
    m_header(nullptr),
    m_body(nullptr),
    m_attributes(attributes)
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);

    if (m_attributes.test(SHOW_HEADER))
    {
        m_header = vbox->addWidget(std::make_unique<PropertySheetHeader>(this));        
    }

    m_body = vbox->addWidget(std::make_unique<PropertySheetBody>(this), 1);
}

void PropertySheetWidget::pushPage(std::unique_ptr<PropertyPageWidget> propertyPage)
{
    if (auto current = m_body->getPage())
    {
        if (m_attributes.test(ENABLE_VALIDATION) && current->validate() == false)
        {
            return;
        }
    }
    m_body->pushPage(std::move(propertyPage));
    refreshHeader();
}

void PropertySheetWidget::nextPage()
{
    _ASSERT(false); // next is not supported
}

void PropertySheetWidget::prevPage()
{
    if (auto current = m_body->getPage())
    {
        if (m_attributes.test(ENABLE_VALIDATION) && current->validate() == false)
        {
            return;
        }
    }

    m_body->prevPage();
    if (!m_attributes.test(SHOW_NEXT))
    {
        m_body->cut();
    }
    refreshHeader();
}

bool PropertySheetWidget::firstPage() const
{
    return m_body->firstPage() == m_body->getPage();
}

bool PropertySheetWidget::lastPage() const
{
    return m_body->lastPage() == m_body->getPage();
}

PropertyPageWidget* PropertySheetWidget::getPage() const
{
    return m_body->getPage();
}

bool PropertySheetWidget::apply()
{
    if (auto current = m_body->getPage())
    {
        if (m_attributes.test(ENABLE_VALIDATION))
        {
            return current->validate();
        }
    }
    return true;
}

void PropertySheetWidget::refreshHeader()
{
    if (!m_header || !m_body->pageCount())
    {
        return;
    }
    
    auto page = m_body->getPage();
    m_header->setCaption(page->getCaption());
    m_header->setDescription(page->getDescription());
    m_header->showBackButton(m_attributes.test(SHOW_BUTTONS) && !firstPage());  
}

PropertyPageWidget::PropertyPageWidget(Wt::WString caption, Wt::WString description, int identity):
    m_propertySheet(nullptr),
    m_caption(std::move(caption)),
    m_description(std::move(description)),
    m_identity(identity),
    m_dirty(false)
{
}

void PropertyPageWidget::setPropertySheet(PropertySheetWidget* propertySheet)
{
    m_propertySheet = propertySheet;
}

PropertySheetWidget* PropertyPageWidget::getPropertySheet() const
{
    return m_propertySheet;
}

Wt::WString PropertyPageWidget::getCaption() const
{
    return m_caption;
}

Wt::WString PropertyPageWidget::getDescription() const
{
    return m_description;
}

int PropertyPageWidget::getIdentity() const
{
    return m_identity;
}

void PropertyPageWidget::setDirty(bool dirty)
{
    m_dirty = dirty;
}

bool PropertyPageWidget::isDirty() const
{
    return m_dirty;
}

bool PropertyPageWidget::validate()
{
    return true;
}

bool validatePropertyPages(Wt::WWidget* _parent)
{   
    auto parent = dynamic_cast<Wt::WWebWidget*>(_parent);
    if (!parent)
    {
        return true;
    }

    if (auto propertyPageWidget = dynamic_cast<PropertyPageWidget*>(parent))
    {
        if (propertyPageWidget->isVisible())
        {
            return propertyPageWidget->validate();
        }
        return true;
    }   

    for (auto childWidget : parent->children())
    {
        if (!validatePropertyPages(childWidget))
        {
            return false;
        }
    }

    return true;
}

ContainerPage::ContainerPage(const Wt::WString& caption, const Wt::WString& description, std::unique_ptr<Wt::WContainerWidget> widget, int identity):
    PropertyPageWidget(caption, description, identity)
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->addWidget(std::move(widget), 1);
}