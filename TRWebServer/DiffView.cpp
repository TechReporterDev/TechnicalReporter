#include "stdafx.h"
#include "DiffView.h"
#include "RegularDiffView.h"
#include "XmlDiffView.h"
#include "TextDiffView.h"
#include "Application.h"

std::unique_ptr<DiffView> createDiffView(const TR::ReportTypeInfo& reportTypeInfo, std::shared_ptr<TR::Content> old_content, std::shared_ptr<TR::Content> new_content)
{
    _ASSERT(old_content && new_content);

    std::shared_ptr<TR::Diff> diff = old_content->make_diff(*new_content);
    if (!diff)
    {
        return std::make_unique<EmptyDiffView>();
    }

    switch (reportTypeInfo.m_syntax)
    {
    case TR::ReportTypeInfo::Syntax::REGULAR_XML:
    case TR::ReportTypeInfo::Syntax::COMPLIANCE_XML:
        return std::make_unique<RegularDiffView>(
            std::dynamic_pointer_cast<TR::RegularContent>(old_content), 
            std::dynamic_pointer_cast<TR::RegularContent>(new_content),
            std::dynamic_pointer_cast<TR::RegularDiff>(diff));

    case TR::ReportTypeInfo::Syntax::PLAIN_XML:
        return std::make_unique<XmlDiffView>(
            std::dynamic_pointer_cast<TR::XmlContent>(old_content), 
            std::dynamic_pointer_cast<TR::XmlContent>(new_content),
            std::dynamic_pointer_cast<TR::PlainXmlDiff>(diff));

    case TR::ReportTypeInfo::Syntax::PLAIN_TEXT:
        return std::make_unique<TextDiffView>(
            std::dynamic_pointer_cast<TR::TextContent>(old_content),
            std::dynamic_pointer_cast<TR::PlainTextDiff>(diff));

    default:
        _ASSERT(false);
        return std::make_unique<EmptyDiffView>();
    }
}