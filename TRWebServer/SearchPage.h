#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

struct SearchRecord;
class SearchWidget: public MainFrameWidget
{
public:
    SearchWidget(std::wstring searchString);
    void createToolBar();

private:
    SearchRecord makeSearchRecord(const TR::SourceInfo& sourceInfo);
    SearchRecord makeSearchRecord(const TR::CurrentReportInfo& currentReportInfo);
    SearchRecord makeSearchRecord(const TR::SubjectReportOptions& reportOptions);

    std::map<TR::ReportTypeUUID, TR::ReportTypeInfo> m_reportTypesInfo;
    std::map<TR::SourceKey, TR::SourceInfo> m_sourcesInfo;
    std::map<TR::RoleKey, TR::RoleInfo> m_rolesInfo;
};

class SearchPage: public MainFramePage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    SearchPage(std::wstring searchString);
    virtual MainFrameWidget* getWidget() override;

private:
    SearchWidget m_widget;
};