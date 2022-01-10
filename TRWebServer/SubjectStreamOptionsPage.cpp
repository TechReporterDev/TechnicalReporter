#include "stdafx.h"
#include "SubjectStreamOptionsPage.h"
#include "OptionProperties.h"
#include "SourceOptionsPage.h"
#include "RoleOptionsPage.h"
#include "PropertySheet.h"
#include "CommonPropertyPage.h"
#include "Application.h"

namespace {

TR::ReportVisibility GetEffectiveVisibility(TR::SubjectStreamOptions& subjectStreamOptions)
{
    if (subjectStreamOptions.m_stream_generating.m_stream_visibility == TR::ReportVisibility::DEFAULT_VISIBILITY)
    {
        return subjectStreamOptions.m_stream_generating.m_default_visibile ?
            TR::ReportVisibility::VISIBLE_REPORT :
            TR::ReportVisibility::INVISIBLE_REPORT;
    }
    return subjectStreamOptions.m_stream_generating.m_stream_visibility;
}

}

class StreamOptionsGeneralPage : public CommonPropertyPage
{   
public:
    StreamOptionsGeneralPage(TR::SubjectStreamOptions& subjectStreamOptions):
        CommonPropertyPage(L"General", L"General stream options"),
        m_subjectStreamOptions(subjectStreamOptions),
        m_showUncompatibleOptions(false)
    {
        CommonProperty stream_properties{L"General", L"General stream options",
            CommonProperty(L"Stream generating", L"",
                PolicyProperty(L"", L"Setup how stream will be generated", {L"Use storage", L"Not required"},
                    m_subjectStreamOptions.m_stream_generating.m_default_visibile,
                    m_subjectStreamOptions.m_stream_generating.m_stream_visibility
                )
            )
        };

        stream_properties.addProperties(
            OptionalProperty(
                [this] {
                    return m_showUncompatibleOptions == true ||
                    GetEffectiveVisibility(m_subjectStreamOptions) == TR::ReportVisibility::VISIBLE_REPORT;
                },
                CommonProperty(L"Stream storage", L"",
                    PolicyProperty(L"", L"Setup stream length", {L"Forever", L"Hour", L"Day", L"Month", L"Year", L"None"},
                        m_subjectStreamOptions.m_stream_generating.m_default_stream_length,
                        m_subjectStreamOptions.m_stream_generating.m_stream_length
                    )
                )
            )
        );

        stream_properties.addProperties(
            OptionalProperty(
                [this] {
                    return m_showUncompatibleOptions == true ||
                    GetEffectiveVisibility(m_subjectStreamOptions) == TR::ReportVisibility::VISIBLE_REPORT;
                },
                CommonProperty(L"Notification", L"",
                    PolicyProperty(L"", L"Select message severety to send email", {L"Info", L"Warning", L"Failed", L"None"},
                        m_subjectStreamOptions.m_stream_generating.m_default_notification_severity,
                        m_subjectStreamOptions.m_stream_generating.m_notification_severity
                    )
                )
            )
        );

        stream_properties.addProperties(
            OptionalProperty(
                [this] {
                    return m_showUncompatibleOptions == false &&
                        GetEffectiveVisibility(m_subjectStreamOptions) == TR::ReportVisibility::INVISIBLE_REPORT;
                },
                CommonProperty(L"Uncompatible features", L"",
                    AnyProperty(L"", L"Some features are uncompatible with generation mode. Click 'Show' to see them.", L"Show",
                        staticValue(boost::any()))
                )
            )
        );

        show(std::move(stream_properties));
    }

    virtual void handleEvent(boost::any any) override
    {
        m_showUncompatibleOptions = true;
        refresh();
    }

public:
    TR::SubjectStreamOptions& m_subjectStreamOptions;
    bool m_showUncompatibleOptions;
};

SubjectStreamOptionsWidget::SubjectStreamOptionsWidget(TR::SubjectStreamOptions subjectStreamOptions):
    m_subjectStreamOptions(std::move(subjectStreamOptions))
{
    createToolBar();

    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(9, 0, 9, 9);   

    auto propertySheet = vbox->addWidget(std::make_unique<PropertySheetWidget>());
    propertySheet->pushPage(std::make_unique<StreamOptionsGeneralPage>(m_subjectStreamOptions));
}

void SubjectStreamOptionsWidget::createToolBar()
{
    auto toolbar = setTitleBar(std::make_unique<Wt::WToolBar>());   

    auto btnOk = Wt::addButton<Wt::WPushButton>(*toolbar, "OK");
    btnOk->clicked().connect(std::bind(&SubjectStreamOptionsWidget::onOk, this));

    auto btnCancel = Wt::addButton<Wt::WPushButton>(*toolbar, "Cancel");
    btnCancel->clicked().connect(std::bind(&SubjectStreamOptionsWidget::onCancel, this));
}

void SubjectStreamOptionsWidget::onOk()
{
    auto& client = Application::instance()->getClient();
    client.setOptions(m_subjectStreamOptions.m_subject_key, m_subjectStreamOptions);
    Application::popPage();
}

void SubjectStreamOptionsWidget::onCancel()
{
    Application::popPage();
}

static std::string get_page_url(TR::SubjectStreamOptions& subjectStreamOptions)
{
    return (boost::format("/subject_options/subject_type=%1%/subject_key=%2%/report_type=%3%")
        % subjectStreamOptions.m_subject_key.which()
        % boost::polymorphic_get<TR::Core::KeyRestriction>(subjectStreamOptions.m_subject_key).m_value
        % stl_tools::to_string(subjectStreamOptions.m_stream_type_uuid)).str();
}

SubjectStreamOptionsPage::SubjectStreamOptionsPage(TR::SubjectStreamOptions subjectStreamOptions):
    MainFramePage(L"Options", get_page_url(subjectStreamOptions)),
    m_widget(subjectStreamOptions)
{
    auto& client = Application::instance()->getClient();
    if (auto sourceKey = boost::get<TR::SourceKey>(&subjectStreamOptions.m_subject_key))
    {
        m_path = SourceOptionsPage(client.getSourceInfo(*sourceKey)).getPath();
    }
    else if (auto roleKey = boost::get<TR::RoleKey>(&subjectStreamOptions.m_subject_key))
    {
        m_path = RoleOptionsPage(client.getRoleInfo(*roleKey)).getPath();
    }
    else
    {
        _ASSERT(false);
    }
    m_path.push_back({m_caption, m_url});
}

MainFrameWidget* SubjectStreamOptionsPage::getWidget()
{
    return &m_widget;
}