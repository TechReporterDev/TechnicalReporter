#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class StreamTypeList;
class StreamSelectionWidget: public MainFrameWidget
{
public:
    StreamSelectionWidget();
    void refresh();

private:
    void createToolBar();
    void onCreateClick();

    StreamTypeList* m_streamTypeList;
};

class StreamSelectionPage: public MainFramePage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    StreamSelectionPage();
    virtual MainFrameWidget* getWidget() override;

private:
    std::unique_ptr<StreamSelectionWidget> m_widget;
};