#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "DiffView.h"

class RegularDiffView: public DiffView
{
public:
    RegularDiffView(std::shared_ptr<TR::RegularContent> old_content, std::shared_ptr<TR::RegularContent> new_content, std::shared_ptr<TR::RegularDiff> diff);

private:
    std::shared_ptr<TR::RegularContent> m_old_content;
    std::shared_ptr<TR::RegularContent> m_new_content;
    std::shared_ptr<TR::RegularDiff> m_diff;
};