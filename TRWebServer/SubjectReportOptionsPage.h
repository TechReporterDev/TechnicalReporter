#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
class PropertySheetWidget;

class SubjectReportOptionsWidget: public MainFrameWidget
{
public:
    using OnApply = std::function<void()>;
    SubjectReportOptionsWidget(TR::SubjectReportOptions subjectReportOptions);
    void setOnApply(OnApply onApply);

private:    
    void createToolBar();   
    void onOk();
    void onCancel();

    PropertySheetWidget* m_optionsEditor;
    OnApply m_onApply;
};

class SubjectReportOptionsPage: public MainFramePage
{
public:
    using OnApply = SubjectReportOptionsWidget::OnApply;
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    SubjectReportOptionsPage(TR::SubjectReportOptions subjectReportOptions);
    
    void                        setOnApply(OnApply onApply);
    virtual MainFrameWidget*    getWidget() override;

private:
    SubjectReportOptionsWidget m_widget;
};