#pragma once
#include "TRWebServer.h"
#include "Dialog.h"
#include "MainFrame.h"

class SourceBrowseDlg: public Dialog
{
public:
    typedef std::function<void(TR::SourceInfo sourceInfo)> OnApply;

    SourceBrowseDlg(TR::SourceKey selected, TR::SourceKey excluded = TR::SourceKey(0));
    void connectApply(OnApply onApply);

private:
    // Dialog override
    virtual void apply() override;

    Wt::WTreeView* m_treeView;
    OnApply m_onApply;
};