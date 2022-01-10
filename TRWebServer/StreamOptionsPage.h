#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
class StreamSubjectOptions;
class StreamModifiedOptions;

class StreamOptionsWidget: public MainFrameWidget
{
public:
    StreamOptionsWidget(const TR::StreamTypeInfo& streamTypeInfo);

private:
    void            createToolBar();
    virtual void    load() override;

    StreamSubjectOptions* m_roleOptions;
    StreamSubjectOptions* m_sourceOptions;
    StreamModifiedOptions* m_modifiedOptions;

    TR::StreamTypeInfo m_streamTypeInfo;
    std::map<TR::SourceKey, TR::SourceInfo> m_sourcesInfo;
    std::map<TR::RoleKey, TR::RoleInfo> m_rolesInfo;
};

class StreamOptionsPage: public MainFramePage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    StreamOptionsPage(const TR::StreamTypeInfo& streamTypeInfo);
    virtual MainFrameWidget*    getWidget() override;

private:
    TR::StreamTypeInfo m_streamTypeInfo;
    std::unique_ptr<StreamOptionsWidget> m_widget;
};