#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "DiffView.h"

class XmlDiffView: public DiffView
{
public:
    XmlDiffView(std::shared_ptr<TR::XmlContent> old_content, std::shared_ptr<TR::XmlContent> new_content, std::shared_ptr<TR::PlainXmlDiff> diff);

private:
    std::shared_ptr<TR::XmlContent> m_old_content;
    std::shared_ptr<TR::XmlContent> m_new_content;
    std::shared_ptr<TR::PlainXmlDiff> m_diff;
};