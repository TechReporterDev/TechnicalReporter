#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "DiffView.h"
class TextDiffTableView;

class TextDiffView: public DiffView
{
public:
    TextDiffView(std::shared_ptr<TR::TextContent> oldContent, std::shared_ptr<TR::PlainTextDiff> diff);
    virtual std::unique_ptr<Wt::WWidget> getViewBar() override;

private:
    std::shared_ptr<TR::TextContent> m_oldContent;
    std::shared_ptr<TR::PlainTextDiff> m_diff;

    TextDiffTableView* m_tableView;
};
