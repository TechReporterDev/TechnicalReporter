#pragma once
#include "TRWebServer.h"

class PropertyPageWidget;
class PropertySheetHeader;
class PropertySheetBody;

class PropertySheetWidget: public Wt::WContainerWidget
{
public:
    enum Attribute { SHOW_HEADER = 0x01, SHOW_BUTTONS = 0x02, SHOW_NEXT = 0x04, ENABLE_VALIDATION = 0x08};
    using Attributes = Wt::WFlags<Attribute>;
    static const Attributes DEFAULT_ATTRIBUTES; //SHOW_HEADER | SHOW_BUTTONS | ENABLE_VALIDATION

    PropertySheetWidget(Attributes attributes = DEFAULT_ATTRIBUTES);

    void pushPage(std::unique_ptr<PropertyPageWidget> propertyPage);
    void nextPage();
    void prevPage();
    bool firstPage() const;
    bool lastPage() const;
    
    PropertyPageWidget* getPage() const;
    virtual bool apply();

private:
    void refreshHeader();

    PropertySheetHeader* m_header;
    PropertySheetBody* m_body;
    Attributes m_attributes;
};

class PropertyPageWidget: public Wt::WContainerWidget
{
public:
    PropertyPageWidget(Wt::WString caption, Wt::WString description, int identity);
    
    void                    setPropertySheet(PropertySheetWidget* propertySheet);
    PropertySheetWidget*    getPropertySheet() const;
        
    Wt::WString             getCaption() const;
    Wt::WString             getDescription() const;
    int                     getIdentity() const;
    
    void                    setDirty(bool dirty);
    bool                    isDirty() const;

    virtual bool            validate();

protected:
    PropertySheetWidget* m_propertySheet;
    Wt::WString m_caption;
    Wt::WString m_description;
    int m_identity;
    bool m_dirty;
};

bool validatePropertyPages(Wt::WWidget* parent);

class ContainerPage: public PropertyPageWidget
{
public:
    ContainerPage(const Wt::WString& caption, const Wt::WString& description, std::unique_ptr<Wt::WContainerWidget> widget, int identity = 0);
};