#include "stdafx.h"
#include "ContentView.h"
#include "TextContentView.h"
#include "XmlContentView.h"
#include "RegularContentView.h"
#include "ComplianceContentView.h"
#include "ValidatedContentView.h"
#include "Layout.h"
#include "Application.h"


EmptyContentView::EmptyContentView(const Wt::WString& text)
{
    auto center = centerLayout(this);
    center->addWidget(std::make_unique<Wt::WText>(text));
}

EmptyContentView::EmptyContentView(std::unique_ptr<Wt::WContainerWidget> body)
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->addWidget(std::move(body));
}

std::unique_ptr<ContentView> createContentView(std::shared_ptr<const TR::Content> content)
{
    if (!content)
    {
        return std::make_unique<EmptyContentView>();
    }
    else if (auto validatedContent = std::dynamic_pointer_cast<const TR::ValidatedContent>(content))
    {
        return std::make_unique<ValidatedContentView>(validatedContent);
    }
    else if (auto complianceContent = std::dynamic_pointer_cast<const TR::ComplianceContent>(content))
    {
        return std::make_unique<ComplianceContentView>(complianceContent);
    }
    else if (auto regularContent = std::dynamic_pointer_cast<const TR::RegularContent>(content))
    {
        return std::make_unique<RegularContentView>(regularContent);
    }
    else if (auto xmlContent = std::dynamic_pointer_cast<const TR::XmlContent>(content))
    {
        return std::make_unique<XmlContentView>(xmlContent);
    }   
    else if (auto textContent = std::dynamic_pointer_cast<const TR::TextContent>(content))
    {
        return std::make_unique<TextContentView>(textContent);
    }   
    else
    {
        _ASSERT(false);
    }
    throw std::logic_error("Unknown content type found");
}