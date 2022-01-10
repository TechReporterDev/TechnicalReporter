#pragma once
#include "TRWebServer.h"
#include "ContentView.h"

class TextContentView: public ContentView
{
public:
    TextContentView(std::shared_ptr<const TR::TextContent> content);

    // override ContentView
    virtual void update(std::shared_ptr<const TR::Content> content);
};
