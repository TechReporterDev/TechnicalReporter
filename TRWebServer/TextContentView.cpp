#include "stdafx.h"
#include "TextContentView.h"
#include "Application.h"

TextContentView::TextContentView(std::shared_ptr<const TR::TextContent> content)
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    update(content);    
}

void TextContentView::update(std::shared_ptr<const TR::Content> content)
{   
    auto vbox = Wt::clear(this);

    auto textContent = std::dynamic_pointer_cast<const TR::TextContent>(content);
    auto text = vbox->addWidget(std::make_unique<Wt::WText>(textContent->as_wstring(), Wt::TextFormat::Plain));
    text->addStyleClass("tr-content");
}