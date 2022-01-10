#include "stdafx.h"
#include "SubjectReportOptionsPage.h"
#include "ReportOptionsEditor.h"
#include "ReportViewOptionsEditor.h"
#include "SourceOptionsPage.h"
#include "RoleOptionsPage.h"
#include "Application.h"

SubjectReportOptionsWidget::SubjectReportOptionsWidget(TR::SubjectReportOptions subjectReportOptions):
    m_optionsEditor(nullptr)
{
    createToolBar();

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);
    if (subjectReportOptions.m_report_generating.m_default_deferred)
    {
        m_optionsEditor = vbox->addWidget(std::make_unique<ReportViewOptionsEditor>(subjectReportOptions), 1);
    }
    else
    {
        m_optionsEditor = vbox->addWidget(std::make_unique<ReportOptionsEditor>(subjectReportOptions), 1);
    }
}

void SubjectReportOptionsWidget::setOnApply(OnApply onApply)
{
    _ASSERT(!m_onApply);
    m_onApply = onApply;
}

void SubjectReportOptionsWidget::createToolBar()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());   

    auto btnOk = Wt::addButton<Wt::WPushButton>(*toolbar, "OK");
    btnOk->clicked().connect(std::bind(&SubjectReportOptionsWidget::onOk, this));

    auto btnCancel = Wt::addButton<Wt::WPushButton>(*toolbar, "Cancel");    
    btnCancel->clicked().connect(std::bind(&SubjectReportOptionsWidget::onCancel, this));
}

void SubjectReportOptionsWidget::onOk()
{
    m_optionsEditor->apply();
    if (m_onApply)
    {
        m_onApply();
    }
    Application::popPage();
}

void SubjectReportOptionsWidget::onCancel()
{
    Application::popPage();
}

static std::string get_page_url(TR::SubjectReportOptions& subjectReportOptions)
{
    return (boost::format("/subject_options/subject_type=%1%/subject_key=%2%/report_type=%3%")
        % subjectReportOptions.m_subject_key.which()
        % boost::polymorphic_get<TR::Core::KeyRestriction>(subjectReportOptions.m_subject_key).m_value
        % stl_tools::to_string(subjectReportOptions.m_report_type_uuid)).str();
}

SubjectReportOptionsPage::SubjectReportOptionsPage(TR::SubjectReportOptions subjectReportOptions):
    MainFramePage(L"Options", get_page_url(subjectReportOptions)),
    m_widget(subjectReportOptions)
{
    auto& client = Application::instance()->getClient();
    if (auto sourceKey = boost::get<TR::SourceKey>(&subjectReportOptions.m_subject_key))
    {
        m_path = SourceOptionsPage(client.getSourceInfo(*sourceKey)).getPath();
    }
    else if (auto roleKey = boost::get<TR::RoleKey>(&subjectReportOptions.m_subject_key))
    {
        m_path = RoleOptionsPage(client.getRoleInfo(*roleKey)).getPath();
    }
    else
    {
        _ASSERT(false);
    }
    m_path.push_back({m_caption, m_url});
}

void SubjectReportOptionsPage::setOnApply(OnApply onApply)
{
    m_widget.setOnApply(onApply);
}

MainFrameWidget* SubjectReportOptionsPage::getWidget()
{
    return &m_widget;
}