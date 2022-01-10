#include "stdafx.h"
#include "TextContentEditor.h"
#include "Application.h"

TextContentEditor::TextContentEditor(std::shared_ptr<TR::TextContent> content) :
    m_text(nullptr)
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);

    m_text = vbox->addWidget(std::make_unique<Wt::WTextArea>(content->as_wstring()));
    m_text->addStyleClass("tr-content");
}

std::shared_ptr<TR::Content> TextContentEditor::getContent()
{
    return std::make_shared<TR::Core::PlainTextContent>(m_text->text().toUTF8(), nullptr);
}

void TextContentEditor::setContent(std::shared_ptr<TR::Content> content)
{
    auto textContent = std::dynamic_pointer_cast<TR::TextContent>(content);
    m_text->setText(textContent->as_wstring());
}