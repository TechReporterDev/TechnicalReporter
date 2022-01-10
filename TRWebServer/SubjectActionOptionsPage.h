#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
class ActionOptionsEditor;

class SubjectActionOptionsWidget: public MainFrameWidget
{
public:
    using OnApply = std::function<void()>;
    SubjectActionOptionsWidget(TR::SubjectActionOptions subjectActionOptions);
    void setOnApply(OnApply onApply);

private:
    void createToolBar();
    void onOk();
    void onCancel();

    ActionOptionsEditor* m_actionOptionsEditor;
    OnApply m_onApply;
};

class SubjectActionOptionsPage: public MainFramePage
{
public:
    using OnApply = SubjectActionOptionsWidget::OnApply;
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    SubjectActionOptionsPage(TR::SubjectActionOptions subjectActionOptions);

    void                        setOnApply(OnApply onApply);
    virtual MainFrameWidget*    getWidget() override;

private:
    SubjectActionOptionsWidget m_widget;
};