#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class ContentEditor: public Wt::WContainerWidget
{
public:
    virtual ~ContentEditor() = default;
    virtual std::shared_ptr<TR::Content>    getContent() = 0;
    virtual void                            setContent(std::shared_ptr<TR::Content>) = 0;

protected:
    ContentEditor() = default;
    ContentEditor(const ContentEditor&) = default;
    ContentEditor& operator = (const ContentEditor&) = default;
};

class NullContentEditor: public ContentEditor
{
public:
    NullContentEditor()
    {
        auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
        vbox->setContentsMargins(0, 0, 0, 0);
        vbox->addWidget(std::make_unique<Wt::WText>("Empty content"), 1);
    }

    virtual std::shared_ptr<TR::Content> getContent() override
    {
        return nullptr;
    }

    virtual void setContent(std::shared_ptr<TR::Content> content) override
    {
        _ASSERT(false);
    }
};