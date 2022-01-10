#pragma once
#include "TRWebServer.h"
#include "RegularDocTreeView.h"

class RegularDocEditor: public RegularDocTreeView
{
public:
    RegularDocEditor(TR::XML::XmlRegularDoc& doc);
};