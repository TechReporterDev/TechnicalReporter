#include "stdafx.h"
#include "ExportSummaryDlg.h"
#include "CommonPropertyDoc.h"
#include "CommonPropertyPage.h"
#include "Application.h"

namespace {
class ContentResource : public Wt::WResource
{
public:
    ContentResource(std::shared_ptr<TR::Content> content, std::wstring fileName, std::string mimeType) :
        WResource(),
        m_content(content),
        m_mimeType(std::move(mimeType))
    {
        suggestFileName(std::move(fileName));
    }

    ~ContentResource() {
        beingDeleted();
    }

    void handleRequest(const Wt::Http::Request& request,
        Wt::Http::Response& response) {
        response.setMimeType(m_mimeType);
        auto blob = m_content->as_blob();
        if (!blob.empty())
        {
            response.out().write(&blob[0], blob.size());
        }
    }

private:
    std::shared_ptr<TR::Content> m_content;
    std::string m_mimeType;
};
}

ExportSummaryDlg::ExportSummaryDlg() :
    TaskDlg(L"Export summary", L"Running...")
{
    auto& client = Application::instance()->getClient();
    m_job = client.exportSummary(
        [this](std::shared_ptr<TR::Content> output) {
            Wt::WLink link = Wt::WLink(std::make_shared<ContentResource>(
                output, L"Summary.pdf", "application/pdf"));
            expand(std::make_unique<Wt::WAnchor>(link, "Click to download summary in pdf format"));
            setCompleted("Export completed");
        },
        [this](const std::string& err) {
            setFailed("Export failed!", err);
        }
    );
}

void ExportSummaryDlg::apply()
{    
}