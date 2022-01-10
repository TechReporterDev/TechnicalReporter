#include "stdafx.h"
#include "PatternEditor.h"
#include "TextContentEditor.h"
#include "RegularContentEditor.h"
#include "UploadDialog.h"
#include "Application.h"

namespace {
std::vector<char> read_blob(const std::wstring& file_system_path)
{
    std::ifstream fs(file_system_path);
    if (!fs)
    {
        throw std::runtime_error("Can`t read file");
    }

    return{std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>()};
}

std::unique_ptr<ContentEditor> createContentEditor(std::shared_ptr<TR::Content> content)
{
    if (!content)
    {
        return std::make_unique<NullContentEditor>();
    }
    else if (auto regularContent = std::dynamic_pointer_cast<TR::RegularContent>(content))
    {
        return std::make_unique<RegularContentEditor>(regularContent);
    }
    else if (auto textContent = std::dynamic_pointer_cast<TR::TextContent>(content))
    {
        return std::make_unique<TextContentEditor>(textContent);
    }
    else
    {
        _ASSERT(false);
    }

    throw std::logic_error("Can`t edit given content");
}
} //namespace {

PatternEditor::PatternEditor(TR::SubjectKey subjectKey, TR::ReportTypeUUID reportTypeUUID):
    m_subjectKey(subjectKey),
    m_reportTypeUUID(reportTypeUUID),
    m_contentEditor(nullptr)
{
    auto app = Application::instance();
    auto& client = app->getClient();
    auto pattern = client.getPattern(m_subjectKey, m_reportTypeUUID);
    
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);

    auto hbox = vbox->addLayout(std::make_unique<Wt::WHBoxLayout>());
    hbox->addStretch(1);

    auto toolbar = hbox->addWidget(std::make_unique<Wt::WToolBar>());
    
    auto createBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "New");
    createBtn->clicked().connect([this, vbox](const Wt::WMouseEvent){
        auto& client = Application::instance()->getClient();
        auto pattern = client.createPattern(m_reportTypeUUID);
        
        if (m_contentEditor)        
        {
            vbox->removeWidget(m_contentEditor);
        }

        m_contentEditor = vbox->addWidget(createContentEditor(pattern), 1);
    });

    auto uploadBtn = Wt::addButton<UploadButton>(*toolbar, "Open");
    uploadBtn->connectUploaded([this, vbox](const Wt::WString& path){
        try
        {
            auto& client = Application::instance()->getClient();
            auto pattern = client.createPattern(m_reportTypeUUID, read_blob(path));
            
            if (m_contentEditor)
            {
                vbox->removeWidget(m_contentEditor);
            }
            m_contentEditor = vbox->addWidget(createContentEditor(pattern), 1);
        }
        catch (...)
        {
            Wt::WMessageBox::show(L"Failed", L"Feailed to upload pattern", Wt::StandardButton::Ok);
        }
    });

    toolbar->addSeparator();

    auto removeBtn = Wt::addButton<Wt::WPushButton>(*toolbar, "Remove");
    removeBtn->clicked().connect([this, vbox](Wt::WMouseEvent){
        if (m_contentEditor)
        {
            vbox->removeWidget(m_contentEditor);
        }
        m_contentEditor = vbox->addWidget(createContentEditor(nullptr), 1);
    });

    m_contentEditor = vbox->addWidget(createContentEditor(pattern), 1);
}

void PatternEditor::apply()
{
    auto app = Application::instance();
    auto& client = app->getClient();
    
    if (auto content = m_contentEditor->getContent())
    {
        client.setPattern(m_subjectKey, m_reportTypeUUID, content->as_blob());
    }
    else
    {
        client.resetPattern(m_subjectKey, m_reportTypeUUID);
    }
}