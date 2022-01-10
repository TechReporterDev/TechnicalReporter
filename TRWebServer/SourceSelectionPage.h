#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class SourceList;
class SourceSelectionWidget: public MainFrameWidget
{
public: 
    SourceSelectionWidget();    
    void createToolBar();

private:        
    SourceList* m_sourceList;
};

class SourceSelectionPage: public MainFramePage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    SourceSelectionPage();
    virtual MainFrameWidget* getWidget() override;

private:
    std::unique_ptr<SourceSelectionWidget> m_widget;
};