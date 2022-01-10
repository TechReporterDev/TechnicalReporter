#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class DiffView: public Wt::WContainerWidget
{
public:
    virtual std::unique_ptr<Wt::WWidget> getViewBar()
    {
        return nullptr;
    }
};

class EmptyDiffView: public DiffView
{
public:
    EmptyDiffView(Wt::WString text = L"Diff view is not available")
    {
        addWidget(std::make_unique<Wt::WText>(text));
    }
};

std::unique_ptr<DiffView> createDiffView(const TR::ReportTypeInfo& reportTypeInfo, std::shared_ptr<TR::Content> old_content, std::shared_ptr<TR::Content> new_content);