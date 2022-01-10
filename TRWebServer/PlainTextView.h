#pragma once
#include "TRWebServer.h"
#include "ContentView.h"

class PlainTextView: public Wt::WContainerWidget
{
public:
    PlainTextView(const std::wstring& text, std::shared_ptr<const TR::XmlDoc> keywords);
};