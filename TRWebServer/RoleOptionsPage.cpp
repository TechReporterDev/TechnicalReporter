#include "stdafx.h"
#include "RoleSelectionPage.h"
#include "RoleOptionsPage.h"
#include "Application.h"

RoleOptionsWidget::RoleOptionsWidget(const TR::RoleInfo& roleInfo):
    SubjectOptionsWidget(roleInfo.m_key),
    m_roleInfo(roleInfo)
{   
}

std::unique_ptr<MainFramePage> RoleOptionsPage::createPage(const std::string& url)
{
    boost::regex url_reg("/options/roles/key=(\\d+)");
    boost::smatch match;
    if (!boost::regex_match(url, match, url_reg))
    {
        return nullptr;
    }

    TR::RoleKey roleKey(boost::lexical_cast<int>(match[1].str()));
    auto& client = Application::instance()->getClient();
    return std::make_unique<RoleOptionsPage>(client.getRoleInfo(roleKey));
}

RoleOptionsPage::RoleOptionsPage(const TR::RoleInfo& roleInfo):
    SubjectOptionsPage(roleInfo.m_name, "/options/roles/key=" + std::to_string(roleInfo.m_key)),
    m_roleInfo(roleInfo)
{
    m_path = RoleSelectionPage().getPath();
    m_path.push_back({m_caption, m_url});
}

MainFrameWidget* RoleOptionsPage::getWidget()
{
    if (!m_widget)
    {
        m_widget.reset(new RoleOptionsWidget(m_roleInfo));
    }
    return m_widget.get();
}