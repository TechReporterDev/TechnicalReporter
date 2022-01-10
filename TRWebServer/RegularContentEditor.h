#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "ContentEditor.h"

class RegularContentEditor: public ContentEditor
{
public:
    RegularContentEditor(std::shared_ptr<TR::RegularContent> content);
    virtual std::shared_ptr<TR::Content> getContent() override;
    virtual void setContent(std::shared_ptr<TR::Content> content) override;

private:
    std::unique_ptr<TR::XML::XmlRegularDoc> m_doc;
    std::shared_ptr<const TR::XML::XmlDefDoc> m_defDoc;
};