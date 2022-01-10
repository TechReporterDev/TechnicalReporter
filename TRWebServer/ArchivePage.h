#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "ArchiveTable.h"

class ArchiveWidget: public MainFrameWidget
{
public:
    ArchiveWidget(const TR::SourceInfo& sourceInfo);

protected:
    void createToolBar();
    ArchiveTable* m_archiveTable;
    TR::SourceInfo m_sourceInfo;
};

class ArchivePage: public MainFramePage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    ArchivePage(const TR::SourceInfo& sourceInfo);
    virtual MainFrameWidget* getWidget() override;

protected:
    TR::SourceInfo m_sourceInfo;
    std::unique_ptr<ArchiveWidget> m_widget;
};