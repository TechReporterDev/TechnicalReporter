#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class StreamFilterView: public Wt::WContainerWidget
{
public:
    StreamFilterView(TR::XML::XmlQueryDoc& filterDoc);

private:
    void showFilterNode(TR::XML::XmlQueryNode& filterNode);

    const TR::XmlQueryDoc& m_filterDoc;
};