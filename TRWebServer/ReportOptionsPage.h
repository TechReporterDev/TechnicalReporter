#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
class ReportSubjectOptions;
class ReportModifiedOptions;

class ReportOptionsWidget: public MainFrameWidget
{
public:
    ReportOptionsWidget(const TR::ReportTypeInfo& reportTypeInfo);

private:
    void createToolBar();
    virtual void load() override;

    ReportSubjectOptions* m_sourceOptions;
    ReportSubjectOptions* m_roleOptions;
    ReportModifiedOptions* m_modifiedOptions;

    TR::ReportTypeInfo m_reportTypeInfo;
    std::map<TR::SourceKey, TR::SourceInfo> m_sourcesInfo;
    std::map<TR::RoleKey, TR::RoleInfo> m_rolesInfo;
};

class ReportOptionsPage: public MainFramePage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    ReportOptionsPage(const TR::ReportTypeInfo& reportTypeInfo);
    virtual MainFrameWidget*    getWidget() override;

private:
    TR::ReportTypeInfo m_reportTypeInfo;
    std::unique_ptr<ReportOptionsWidget> m_widget;
};