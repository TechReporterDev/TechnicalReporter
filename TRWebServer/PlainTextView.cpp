#include "stdafx.h"
#include "PlainTextView.h"
#include "Application.h"

PlainTextView::PlainTextView(const std::wstring& plainText, std::shared_ptr<const TR::XmlDoc> keywords)
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);

    auto text = vbox->addWidget(std::make_unique<Wt::WText>(plainText, Wt::TextFormat::Plain));
    text->addStyleClass("tr-content");
}