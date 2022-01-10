#include "stdafx.h"
#include "SourceSelectionPage.h"
#include "SourceOptionsPage.h"
#include "Application.h"

SourceOptionsWidget::SourceOptionsWidget(const TR::SourceInfo& sourceInfo):
    SubjectOptionsWidget(sourceInfo.m_key),
    m_sourceInfo(sourceInfo)
{   
}

std::unique_ptr<MainFramePage> SourceOptionsPage::createPage(const std::string& url)
{
    boost::regex url_reg("/options/sources/key=(\\d+)");
    boost::smatch match;
    if (!boost::regex_match(url, match, url_reg))
    {
        return nullptr;
    }

    TR::SourceKey sourceKey(boost::lexical_cast<int>(match[1].str()));
    auto& client = Application::instance()->getClient();
    return std::make_unique<SourceOptionsPage>(client.getSourceInfo(sourceKey));
}

SourceOptionsPage::SourceOptionsPage(const TR::SourceInfo& sourceInfo):
    SubjectOptionsPage(sourceInfo.m_name, "/options/sources/key=" + std::to_string(sourceInfo.m_key)),
    m_sourceInfo(sourceInfo)
{
    m_path = SourceSelectionPage().getPath();
    m_path.push_back({m_caption, m_url});
}

MainFrameWidget* SourceOptionsPage::getWidget()
{
    if (!m_widget)
    {
        m_widget.reset(new SourceOptionsWidget(m_sourceInfo));
    }
    return m_widget.get();
}