#pragma once
#include "TRWebServer.h"
#include "SubjectOptionsPage.h"

class RoleOptionsWidget: public SubjectOptionsWidget
{
public:
    RoleOptionsWidget(const TR::RoleInfo& roleInfo);
    
private:    
    TR::RoleInfo m_roleInfo;
};

class RoleOptionsPage: public SubjectOptionsPage
{
public:
    static std::unique_ptr<MainFramePage> createPage(const std::string& url);

    RoleOptionsPage(const TR::RoleInfo& roleInfo);
    virtual MainFrameWidget* getWidget() override;

private:
    TR::RoleInfo m_roleInfo;
    std::unique_ptr<RoleOptionsWidget> m_widget;
};