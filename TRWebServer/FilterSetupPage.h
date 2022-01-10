#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class FilterSetupWidget: public MainFrameWidget
{
public:
    FilterSetupWidget();
    FilterSetupWidget(const FilterSetupWidget&) = delete;

protected:
    void            initialize();
    virtual void    onPersistClick() = 0;
    void            onPreviewClick();
    void            onCloseClick(); 

    TR::CustomFilterSetup m_filterSetup;
    std::shared_ptr<LazyContentQuery> m_previewContent;
};

class CreateFilterWidget: public FilterSetupWidget
{
public:
    CreateFilterWidget(TR::ReportTypeInfo inputInfo, std::shared_ptr<TR::RegularContent> previewContent);
    CreateFilterWidget(const FilterSetupWidget&) = delete;

protected:
    // override FilterSetupWidget
    virtual void onPersistClick() override;

    TR::ReportTypeInfo m_inputInfo;
};

class CreateFilterPage: public MainFramePage
{
public:
    CreateFilterPage(TR::ReportTypeInfo inputInfo, std::shared_ptr<TR::RegularContent> previewContent = nullptr);
    virtual MainFrameWidget* getWidget() override;

private:
    CreateFilterWidget m_widget;
};

class ModifyFilterWidget: public FilterSetupWidget
{
public:
    ModifyFilterWidget(TR::CustomFilterInfo filterInfo, std::shared_ptr<LazyContentQuery> previewContent);
    ModifyFilterWidget(const FilterSetupWidget&) = delete;

protected:
    // override FilterSetupWidget
    virtual void onPersistClick() override;

    TR::CustomFilterInfo m_filterInfo;
};

class ModifyFilterPage: public MainFramePage
{
public:
    ModifyFilterPage(TR::CustomFilterInfo filterInfo, std::shared_ptr<LazyContentQuery> previewContent = nullptr);
    virtual MainFrameWidget* getWidget() override;

private:
    ModifyFilterWidget m_widget;
};