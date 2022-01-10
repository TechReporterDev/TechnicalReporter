#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "ContentEditor.h"

class TextContentEditor: public ContentEditor
{
public:
    TextContentEditor(std::shared_ptr<TR::TextContent> content);
    virtual std::shared_ptr<TR::Content> getContent() override;
    virtual void setContent(std::shared_ptr<TR::Content> content) override;

private:
    Wt::WTextArea* m_text;
};
