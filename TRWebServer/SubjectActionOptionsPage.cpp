#include "stdafx.h"
#include "SubjectActionOptionsPage.h"
#include "ActionOptionsEditor.h"
#include "SourceOptionsPage.h"
#include "RoleOptionsPage.h"
#include "Application.h"

SubjectActionOptionsWidget::SubjectActionOptionsWidget(TR::SubjectActionOptions subjectActionOptions):
    m_actionOptionsEditor(nullptr)
{
    createToolBar();

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);
    m_actionOptionsEditor = vbox->addWidget(std::make_unique<ActionOptionsEditor>(subjectActionOptions), 1);
}

void SubjectActionOptionsWidget::setOnApply(OnApply onApply)
{
    _ASSERT(!m_onApply);
    m_onApply = onApply;
}

void SubjectActionOptionsWidget::createToolBar()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());   

    auto btnOk = Wt::addButton<Wt::WPushButton>(*toolbar, "OK");
    btnOk->clicked().connect(std::bind(&SubjectActionOptionsWidget::onOk, this));

    auto btnCancel = Wt::addButton<Wt::WPushButton>(*toolbar, "Cancel");
    btnCancel->clicked().connect(std::bind(&SubjectActionOptionsWidget::onCancel, this));
}

void SubjectActionOptionsWidget::onOk()
{
    m_actionOptionsEditor->apply();
    if (m_onApply)
    {
        m_onApply();
    }
    Application::popPage();
}

void SubjectActionOptionsWidget::onCancel()
{
    Application::popPage();
}

static std::string get_page_url(TR::SubjectActionOptions& subjectActionOptions)
{
    auto url = (boost::format("/subject_options/subject_type=%1%/subject_key=%2%/action=%3%")
        % subjectActionOptions.m_subject_key.which()
        % boost::polymorphic_get<TR::Core::KeyRestriction>(subjectActionOptions.m_subject_key).m_value
        % stl_tools::to_string(subjectActionOptions.m_action_uuid)).str();
    
    if (subjectActionOptions.m_shortcut_uuid)
    {
        url += ",shortcut_uuid=" + stl_tools::to_string(*subjectActionOptions.m_shortcut_uuid);
    }
    return url;
}

SubjectActionOptionsPage::SubjectActionOptionsPage(TR::SubjectActionOptions subjectActionOptions):
    MainFramePage(L"Options", get_page_url(subjectActionOptions)),
    m_widget(subjectActionOptions)
{
    auto& client = Application::instance()->getClient();
    if (auto sourceKey = boost::get<TR::SourceKey>(&subjectActionOptions.m_subject_key))
    {
        m_path = SourceOptionsPage(client.getSourceInfo(*sourceKey)).getPath();
    }
    else if (auto roleKey = boost::get<TR::RoleKey>(&subjectActionOptions.m_subject_key))
    {
        m_path = RoleOptionsPage(client.getRoleInfo(*roleKey)).getPath();
    }
    else
    {
        _ASSERT(false);
    }
    m_path.push_back({m_caption, m_url});
}

void SubjectActionOptionsPage::setOnApply(OnApply onApply)
{
    m_widget.setOnApply(onApply);
}

MainFrameWidget* SubjectActionOptionsPage::getWidget()
{
    return &m_widget;
}