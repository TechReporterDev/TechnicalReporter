#pragma once
#include "TRWebServer.h"
#include "ContentView.h"

class ComplianceContentView: public ContentView
{
public:
    ComplianceContentView(std::shared_ptr<const TR::ComplianceContent> content);
    virtual void update(std::shared_ptr<const TR::Content> content) override;

private:
    void reset();
    std::shared_ptr<const TR::ComplianceContent> m_content;
};
