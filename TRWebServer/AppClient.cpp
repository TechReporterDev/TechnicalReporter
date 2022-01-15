#include "stdafx.h"
#include "AppClient.h"
#include "TRCore\SuitsFeature.h"
#include "Application.h"

#define DELEGATE_METHOD(MEMBER_CALL, ERROR_MESSAGE) try{ return m_dc.MEMBER_CALL; } catch(std::exception& e){ throw AppError(ERROR_MESSAGE, e.what()); }

AppEventHandler::AppEventHandler(AppClient* client):
    TR::EventHandler(&client->getDC()),
    m_client(client)
{
}

void AppEventHandler::on_add_source(const TR::SourceInfo& sourceInfo)
{
    onAddSource(sourceInfo);
}

void AppEventHandler::on_update_source(const TR::SourceInfo& sourceInfo)
{
    onUpdateSource(sourceInfo);
}

void AppEventHandler::on_remove_source(TR::Key sourceKey, const std::vector<TR::SourceKey>& removedSources)
{
    onRemoveSource(sourceKey, removedSources);
}

void AppEventHandler::on_add_link(TR::Link link)
{
    onAddLink(link);
}

void AppEventHandler::on_remove_link(TR::Link link)
{
    onRemoveLink(link);
}

void AppEventHandler::on_update_current_report(const TR::CurrentReportInfo& currentReportInfo, bool contentChanged)
{
    onUpdateCurrentReport(currentReportInfo, contentChanged);
}

AppClient::AppClient(AppEventDisp& eventDisp):
    m_dc([&eventDisp](TR::Client::PackedEvent evt){ eventDisp.dispatch(evt); })
{
}

void AppClient::connect(TR::Core::Core& core, const std::wstring& user)
{
    m_dc.connect(core, user);
}

TR::DirectClient& AppClient::getDC()
{
    return m_dc;
}

std::vector<TR::UserInfo> AppClient::getUsers() const
{
    DELEGATE_METHOD(get_users(), "unknown_error");  
}

TR::Key AppClient::addUser(const TR::UserSettings& userSettings)
{
    DELEGATE_METHOD(add_user(userSettings), "unknown_error");
}

void AppClient::updateUser(TR::Key userKey, const TR::UserSettings& userSettings)
{
    DELEGATE_METHOD(update_user(userKey, userSettings), "unknown_error");
}

void AppClient::removeUser(TR::Key userKey)
{
    DELEGATE_METHOD(remove_user(userKey), "unknown_error");
}

TR::SourceAccessInfo AppClient::getAccessInfo(TR::SourceKey sourceKey) const
{
    DELEGATE_METHOD(get_access_info(sourceKey), "unknown_error");
}

std::vector<TR::AccessEntry> AppClient::getAccessEntries(TR::SourceKey sourceKey) const
{
    DELEGATE_METHOD(get_access_entries(sourceKey), "unknown_error");
}

void AppClient::setAccessList(const TR::SourceAccessList& sourceAccessList) const
{
    DELEGATE_METHOD(set_access_list(sourceAccessList), "unknown_error");
}

TR::SummaryInfo AppClient::getSummaryInfo() const
{
    DELEGATE_METHOD(get_summary_info(), "unknown_error");
}


std::vector<TR::ReportTypeInfo> AppClient::getReportTypesInfo() const
{
    DELEGATE_METHOD(get_report_types_info(), "unknown_error");
}

TR::ReportTypeInfo AppClient::getReportTypeInfo(TR::ReportTypeUUID reportTypeUUID) const
{
    DELEGATE_METHOD(get_report_type_info(reportTypeUUID), "unknown_error");
}

std::shared_ptr<const TR::XmlDoc> AppClient::getContentDef(TR::ReportTypeUUID reportTypeUUID) const
{
    DELEGATE_METHOD(get_content_def(reportTypeUUID), "unknown_error");
}

std::vector<TR::StreamTypeInfo> AppClient::getStreamTypesInfo() const
{
    DELEGATE_METHOD(get_stream_types_info(), "unknown_error");
}

TR::StreamTypeInfo AppClient::getStreamTypeInfo(TR::StreamTypeUUID streamTypeUUID) const
{
    DELEGATE_METHOD(get_stream_type_info(streamTypeUUID), "unknown_error");
}

std::shared_ptr<const TR::XmlDefDoc> AppClient::getMessageDef(TR::StreamTypeUUID streamTypeUUID) const
{
    DELEGATE_METHOD(get_message_def(streamTypeUUID), "unknown_error");
}

std::vector<TR::ActionInfo> AppClient::getActionsInfo() const
{
    DELEGATE_METHOD(get_actions_info(), "unknown_error");
}

TR::ActionInfo AppClient::getActionInfo(TR::ActionUUID actionUUID) const
{
    DELEGATE_METHOD(get_action_info(actionUUID), "unknown_error");
}

TR::Params AppClient::getDefaultParams(TR::ActionUUID actionUUID) const
{
    DELEGATE_METHOD(get_default_params(actionUUID), "unknown_error");
}

TR::Params AppClient::createParams(TR::ActionUUID actionUUID, const std::string& paramsXML) const
{
    DELEGATE_METHOD(create_params(actionUUID, paramsXML), "unknown_error");
}

std::vector<TR::SourceTypeInfo> AppClient::getSourceTypesInfo() const
{
    DELEGATE_METHOD(get_source_types_info(), "unknown_error");
}

TR::SourceTypeInfo AppClient::getSourceTypeInfo(TR::SourceTypeUUID sourceTypeUUID) const
{
    DELEGATE_METHOD(get_source_type_info(sourceTypeUUID), "unknown_error");
}

std::shared_ptr<const TR::XmlDefDoc> AppClient::getConfigDef(TR::SourceTypeUUID sourceTypeUUID) const
{
    DELEGATE_METHOD(get_config_def(sourceTypeUUID), "unknown_error");
}

TR::SourceConfig AppClient::getDefaultConfig(TR::SourceTypeUUID sourceTypeUUID) const
{
    DELEGATE_METHOD(get_default_config(sourceTypeUUID), "unknown_error");
}

std::vector<TR::RoleInfo> AppClient::getRolesInfo() const
{
    DELEGATE_METHOD(get_roles_info(), "unknown_error");
}

TR::RoleInfo AppClient::getRoleInfo(TR::RoleKey roleKey) const
{
    DELEGATE_METHOD(get_role_info(roleKey), "unknown_error");
}

TR::RoleKey AppClient::addRole(const TR::RoleSettings& roleSettings) const
{
    DELEGATE_METHOD(add_role(roleSettings), "unknown_error");
}

void AppClient::updateRole(TR::RoleKey roleKey, const TR::RoleSettings& roleSettings) const
{
    DELEGATE_METHOD(update_role(roleKey, roleSettings), "unknown_error");
}

void AppClient::removeRole(TR::RoleKey roleKey)
{
    DELEGATE_METHOD(remove_role(roleKey), "unknown_error");
}

std::vector<TR::SourceInfo> AppClient::getSourcesInfo() const
{
    DELEGATE_METHOD(get_sources_info(), "unknown_error");
}

TR::SourceInfo AppClient::getSourceInfo(TR::SourceKey sourceKey) const
{
    DELEGATE_METHOD(get_source_info(sourceKey), "unknown_error");
}

TR::SourceKey AppClient::addSource(const TR::SourceSettings& sourceSettings)
{
    DELEGATE_METHOD(add_source(sourceSettings), "unknown_error");
}

void AppClient::updateSource(TR::SourceKey sourceKey, const TR::SourceSettings& sourceSettings)
{
    DELEGATE_METHOD(update_source(sourceKey, sourceSettings), "unknown_error");
}

void AppClient::removeSource(TR::SourceKey sourceKey)
{
    DELEGATE_METHOD(remove_source(sourceKey), "unknown_error");
}

TR::SourceConfig AppClient::getSourceConfig(TR::SourceKey sourceKey) const
{
    DELEGATE_METHOD(get_source_config(sourceKey), "unknown_error");
}

void AppClient::setSourceConfig(TR::SourceKey sourceKey, TR::SourceConfig config)
{
    DELEGATE_METHOD(set_source_config(sourceKey, std::move(config)), "unknown_error");
}

std::vector<TR::Link> AppClient::getLinks() const
{
    DELEGATE_METHOD(get_links(), "unknown_error");
}

void AppClient::addLink(TR::Link link)
{
    DELEGATE_METHOD(add_link(link), "unknown_error");
}

void AppClient::removeLink(TR::Link link)
{
    DELEGATE_METHOD(remove_link(link), "unknown_error");
}

TR::CurrentReportInfo AppClient::getCurrentReport(TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID) const
{
    DELEGATE_METHOD(get_current_report(sourceKey, reportTypeUUID), "unknown_error");
}

std::shared_ptr<TR::Content> AppClient::getCurrentContent(TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID) const
{
    DELEGATE_METHOD(get_current_content(sourceKey, reportTypeUUID), "unknown_error");
}

TR::Client::Job AppClient::queryCurrentContent(TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID, TR::Client::ContentReadyHandler completedHandler, TR::Client::JobFailedHandler failedHandler) const
{
    DELEGATE_METHOD(query_current_content(sourceKey, reportTypeUUID, completedHandler, failedHandler), "unknown_error");
}

std::vector<TR::ArchivedReportInfo> AppClient::getArchive(TR::SourceKey sourceKey) const
{
    DELEGATE_METHOD(get_archive(sourceKey), "unknown_error");
}

TR::ArchivedReportInfo AppClient::getArchivedReport(TR::ArchivedReportKey archivedReportKey) const
{
    DELEGATE_METHOD(get_archived_report(archivedReportKey), "unknown_error");
}

std::shared_ptr<TR::Content> AppClient::getArchivedContent(TR::ArchivedReportKey archivedReportKey) const
{
    DELEGATE_METHOD(get_archived_content(archivedReportKey), "unknown_error");
}

TR::Client::Job AppClient::queryArchivedContent(TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID, time_t time, TR::Client::ContentReadyHandler completedHandler, TR::Client::JobFailedHandler failedHandler) const
{
    DELEGATE_METHOD(query_archived_content(sourceKey, reportTypeUUID, time, completedHandler, failedHandler), "unknown_error");
}

TR::StreamInfo AppClient::getStreamInfo(TR::SourceKey sourceKey, TR::StreamTypeUUID streamTypeUUID) const
{
    DELEGATE_METHOD(get_stream_info(sourceKey, streamTypeUUID), "unknown_error");
}

TR::Client::Job AppClient::queryStreamSegment(TR::SourceKey sourceKey, TR::StreamTypeUUID streamTypeUUID, time_t start, time_t stop, TR::Client::StreamReadyHandler completedHandler, TR::Client::JobFailedHandler failedHandler) const
{
    DELEGATE_METHOD(query_stream_segment(sourceKey, streamTypeUUID, start, stop, completedHandler, failedHandler), "unknown_error");
}

TR::SubjectReportOptions AppClient::getOptions(TR::SubjectKey subjectKey, TR::ReportTypeUUID reportTypeUUID) const
{
    DELEGATE_METHOD(get_options(subjectKey, reportTypeUUID), "unknown_error");
}

TR::SubjectStreamOptions AppClient::getOptions(TR::SubjectKey subjectKey, TR::StreamTypeUUID streamTypeUUID) const
{
    DELEGATE_METHOD(get_options(subjectKey, streamTypeUUID), "unknown_error");
}

void AppClient::setOptions(TR::SubjectKey subjectKey, const TR::ReportOptions& reportOptions)
{
    DELEGATE_METHOD(set_options(subjectKey, reportOptions), "unknown_error");
}

void AppClient::setOptions(TR::SubjectKey subjectKey, const TR::StreamOptions& streamOptions)
{
    DELEGATE_METHOD(set_options(subjectKey, streamOptions), "unknown_error");
}

void AppClient::setOptions(TR::SubjectKey subjectKey, const TR::ActionOptions& shortcutOptions)
{
    DELEGATE_METHOD(set_options(subjectKey, shortcutOptions), "unknown_error");
}

TR::SubjectOptions AppClient::getSubjectOptions(TR::SubjectKey subjectKey) const
{
    DELEGATE_METHOD(get_subject_options(subjectKey), "unknown_error");
}

TR::ReportTypeOptions AppClient::getReportTypeOptions(TR::ReportTypeUUID reportTypeUUID) const
{
    DELEGATE_METHOD(get_report_type_options(reportTypeUUID), "unknown_error");
}

TR::StreamTypeOptions AppClient::getStreamTypeOptions(TR::StreamTypeUUID streamTypeUUID) const
{
    DELEGATE_METHOD(get_stream_type_options(streamTypeUUID), "unknown_error");
}

TR::ActionTypeOptions AppClient::getActionOptions(TR::ActionUUID actionUUID, boost::optional<TR::UUID> shortcutUUID) const
{
    DELEGATE_METHOD(get_action_options(actionUUID, shortcutUUID), "unknown_error");
}

TR::CustomQueryInfo AppClient::getCustomQueryInfo(TR::Key customQueryKey) const
{
    DELEGATE_METHOD(get_custom_query_info(customQueryKey), "unknown_error");
}

TR::CustomQueryDefinition AppClient::getCustomQueryDef(TR::Key customQueryKey) const
{
    DELEGATE_METHOD(get_custom_query_def(customQueryKey), "unknown_error");
}

TR::CustomQueryDefinition AppClient::createCustomQuery(std::vector<TR::ReportTypeUUID> inputUUIDs) const
{
    DELEGATE_METHOD(create_custom_query(std::move(inputUUIDs)), "unknown_error");
}

TR::Key AppClient::addCustomQuery(std::wstring name, TR::CustomQueryDefinition customQueryDef)
{
    DELEGATE_METHOD(add_custom_query(std::move(name), std::move(customQueryDef)), "unknown_error");
}

void AppClient::removeCustomQuery(TR::Key customQueryKey)
{
    DELEGATE_METHOD(remove_custom_query(customQueryKey), "unknown_error");
}

TR::CustomFilterInfo AppClient::getCustomFilterInfo(TR::Key customFilterKey) const
{
    DELEGATE_METHOD(get_custom_filter_info(customFilterKey), "unknown_error");
}

TR::Key AppClient::addCustomFilter(std::wstring name, TR::ReportTypeUUID inputUUID, TR::CustomFilterSetup customFilterSetup)
{
    DELEGATE_METHOD(add_custom_filter(name, inputUUID, std::move(customFilterSetup)), "unknown_error");
}

TR::CustomFilterSetup AppClient::createFilterSetup(TR::ReportTypeUUID inputUUID) const
{
    DELEGATE_METHOD(create_filter_setup(inputUUID), "unknown_error");
}

TR::CustomFilterSetup AppClient::getFilterSetup(TR::Key customFilterKey) const
{
    DELEGATE_METHOD(get_filter_setup(customFilterKey), "unknown_error");
}

void AppClient::setFilterSetup(TR::Key customFilterKey, TR::CustomFilterSetup customFilterSetup) const
{
    DELEGATE_METHOD(set_filter_setup(customFilterKey, std::move(customFilterSetup)), "unknown_error");
}

TR::TrackingStreamInfo AppClient::getTrackingStreamInfo(TR::Key trackingStreamKey) const
{
    DELEGATE_METHOD(get_tracking_stream_info(trackingStreamKey), "unknown_error");
}

TR::Key AppClient::addTrackingStream(std::wstring name, TR::ReportTypeUUID inputUUID, std::string input_xpath)
{
    DELEGATE_METHOD(add_tracking_stream(std::move(name), inputUUID, std::move(input_xpath)), "unknown_error");
}

TR::StreamFilterInfo AppClient::getStreamFilterInfo(TR::Key streamFilterKey) const
{
    DELEGATE_METHOD(get_stream_filter_info(streamFilterKey), "unknown_error");
}

TR::StreamFilterSetup AppClient::createFilterSetup(TR::StreamTypeUUID inputUUID) const
{
    DELEGATE_METHOD(create_filter_setup(inputUUID), "unknown_error");
}

TR::Key AppClient::addStreamFilter(std::wstring name, TR::StreamTypeUUID  inputUUID, TR::StreamFilterSetup streamFilterSetup)
{
    DELEGATE_METHOD(add_stream_filter(std::move(name), inputUUID, streamFilterSetup), "unknown_error");
}

TR::StreamFilterSetup AppClient::getStreamFilterSetup(TR::Key streamFilterKey) const
{
    DELEGATE_METHOD(get_stream_filter_setup(streamFilterKey), "unknown_error");
}

void AppClient::setStreamFilterSetup(TR::Key streamFilterKey, TR::StreamFilterSetup streamFilterSetup)
{
    DELEGATE_METHOD(set_stream_filter_setup(streamFilterKey, streamFilterSetup), "unknown_error");
}

std::vector<TR::ComplianceInfo> AppClient::getCompliancesInfo() const
{
    DELEGATE_METHOD(get_compliances_info(), "unknown_error");
}

TR::ComplianceInfo AppClient::getComplianceInfo(TR::ComplianceKey complianceKey) const
{
    DELEGATE_METHOD(get_compliance_info(complianceKey), "unknown_error");
}

TR::ComplianceSetup AppClient::getComplianceSetup(TR::SubjectKey subjectKey, TR::ComplianceKey complianceKey) const
{
    DELEGATE_METHOD(get_compliance_setup(subjectKey, complianceKey), "unknown_error");
}

void AppClient::setComplianceSetup(TR::SubjectKey subjectKey, TR::ComplianceKey complianceKey, TR::ComplianceSetup complianceSetup)
{
    DELEGATE_METHOD(set_compliance_setup(subjectKey, complianceKey, std::move(complianceSetup)), "unknown_error");
}

TR::ValidationInfo AppClient::getValidationInfo(TR::ValidationKey validationKey) const
{
    DELEGATE_METHOD(get_validation_info(validationKey), "unknown_error");
}

TR::ValidationKey AppClient::addValidation(std::wstring name, TR::ReportTypeUUID inputUUID, TR::ValidationSettings defaultSettings) const
{
    DELEGATE_METHOD(add_validation(std::move(name), inputUUID, std::move(defaultSettings)), "unknown_error");
}

TR::ValidationSettings AppClient::createValidationSettings(TR::ReportTypeUUID inputUUID) const
{
    DELEGATE_METHOD(create_validation_settings(inputUUID), "unknown_error");
}

TR::ValidationSettings AppClient::getValidationSettings(TR::SubjectKey subjectKey, TR::ValidationKey validationKey) const
{
    DELEGATE_METHOD(get_validation_settings(subjectKey, validationKey), "unknown_error");
}

void AppClient::setValidationSettings(TR::SubjectKey subjectKey, TR::ValidationKey validationKey, TR::ValidationSettings validationSettings)
{
    DELEGATE_METHOD(set_validation_settings(subjectKey, validationKey, std::move(validationSettings)), "unknown_error");
}

TR::GroupingInfo AppClient::getGroupingInfo(TR::Key groupingKey) const
{
    DELEGATE_METHOD(get_grouping_info(groupingKey), "unknown_error");
}

TR::Key AppClient::addGrouping(std::wstring name, TR::ReportTypeUUID inputUUID)
{
    DELEGATE_METHOD(add_grouping(std::move(name), inputUUID), "unknown_error");
}

void AppClient::removeCustomReport(TR::ReportTypeUUID reportTypeUUID, bool recursive)
{
    DELEGATE_METHOD(remove_custom_report(reportTypeUUID, recursive), "dependent_reports_found");
}

std::shared_ptr<TR::Content> AppClient::createPattern(TR::ReportTypeUUID reportTypeUUID) const
{
    DELEGATE_METHOD(create_pattern(reportTypeUUID), "unknown_error");
}

std::shared_ptr<TR::Content> AppClient::createPattern(TR::ReportTypeUUID reportTypeUUID, TR::Blob blob) const
{
    DELEGATE_METHOD(create_pattern(reportTypeUUID, std::move(blob)), "unknown_error");
}

std::shared_ptr<TR::Content> AppClient::getPattern(TR::SubjectKey subjectKey, TR::ReportTypeUUID reportTypeUUID) const
{
    DELEGATE_METHOD(get_pattern(subjectKey, reportTypeUUID), "unknown_error");
}

std::shared_ptr<TR::Content> AppClient::getEffectivePattern(TR::SubjectKey subjectKey, TR::ReportTypeUUID reportTypeUUID) const
{
    DELEGATE_METHOD(get_effective_pattern(subjectKey, reportTypeUUID), "unknown_error");
}

void AppClient::setPattern(TR::SubjectKey subjectKey, TR::ReportTypeUUID reportTypeUUID, TR::Blob blob)
{
    DELEGATE_METHOD(set_pattern(subjectKey, reportTypeUUID, std::move(blob)), "unknown_error");
}

void AppClient::resetPattern(TR::SubjectKey subjectKey, TR::ReportTypeUUID reportTypeUUID)
{
    DELEGATE_METHOD(reset_pattern(subjectKey, reportTypeUUID), "unknown_error");
}

TR::UUID AppClient::addCustomAction(std::wstring name, TR::ActionUUID actionUUID, TR::Params params)
{
    DELEGATE_METHOD(add_custom_action(std::move(name), actionUUID, std::move(params)), "unknown_error");
}

void AppClient::removeCustomAction(TR::UUID customActionUUID)
{
    DELEGATE_METHOD(remove_custom_action(customActionUUID), "unknown_error");
}


std::vector<TR::ScheduleInfo> AppClient::getSchedulesInfo() const
{
    DELEGATE_METHOD(get_schedules_info(), "unknown_error");
}

TR::ScheduleInfo AppClient::getScheduleInfo(TR::Key scheduleKey) const
{
    DELEGATE_METHOD(get_schedule_info(scheduleKey), "unknown_error");
}

TR::Key AppClient::addSchedule(const TR::ScheduleSettings& scheduleSettings)
{
    DELEGATE_METHOD(add_schedule(scheduleSettings), "unknown_error");
}

void AppClient::updateSchedule(TR::Key scheduleKey, const TR::ScheduleSettings& scheduleSettings)
{
    DELEGATE_METHOD(update_schedule(scheduleKey, scheduleSettings), "unknown_error");
}

void AppClient::removeSchedule(TR::Key scheduleKey)
{
    DELEGATE_METHOD(remove_schedule(scheduleKey), "unknown_error");
}

TR::Client::Job AppClient::reloadReport(TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID, TR::Client::ContentReadyHandler completedHandler, TR::Client::JobFailedHandler failedHandler)
{
    DELEGATE_METHOD(reload_report(sourceKey, reportTypeUUID, completedHandler, failedHandler), "unknown_error");
}

TR::Client::Job AppClient::rebuildReport(TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID, TR::Client::JobCompletedHandler completedHandler, TR::Client::JobFailedHandler failedHandler)
{
    DELEGATE_METHOD(rebuild_report(sourceKey, reportTypeUUID, completedHandler, failedHandler), "unknown_error");
}

TR::Client::Job AppClient::launchAction(TR::SourceKey sourceKey, TR::ActionUUID actionUUID, TR::Params params, TR::Client::ContentReadyHandler completedHandler, TR::Client::JobFailedHandler failedHandler)
{
    DELEGATE_METHOD(launch_action(sourceKey, actionUUID, std::move(params), completedHandler, failedHandler), "unknown_error");
}

TR::Client::Job AppClient::launchShortcut(TR::SourceKey sourceKey, TR::UUID shortcutUUID, TR::Client::ContentReadyHandler completedHandler, TR::Client::JobFailedHandler failedHandler)
{
    DELEGATE_METHOD(launch_shortcut(sourceKey, shortcutUUID, completedHandler, failedHandler), "unknown_error");
}

TR::Client::Job AppClient::exportSummary(TR::Client::ContentReadyHandler completedHandler, TR::Client::JobFailedHandler failedHandler)
{
    DELEGATE_METHOD(export_summary(completedHandler, failedHandler), "unknown_error");
}

std::vector<TR::FeatureInfo> AppClient::getFeaturesInfo() const
{
    DELEGATE_METHOD(get_features_info(), "unknown_error");
}

TR::FeatureInfo AppClient::getFeatureInfo(TR::UUID featureUUID) const
{
    DELEGATE_METHOD(get_feature_info(featureUUID), "unknown_error");
}

void AppClient::installFeature(TR::UUID featureUUID)
{
    DELEGATE_METHOD(install_feature(featureUUID), "unknown_error");
}

void AppClient::uninstallFeature(TR::UUID featureUUID)
{
    DELEGATE_METHOD(uninstall_feature(featureUUID), "unknown_error");
}

LazyContentQuery::LazyContentQuery(AppClient* client, TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID):
    m_client(client),
    m_sourceKey(sourceKey),
    m_reportTypeUUID(reportTypeUUID),
    m_cached(false)
{
}

LazyContentQuery::LazyContentQuery(TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID, std::shared_ptr<TR::Content> content):
    m_client(nullptr),
    m_sourceKey(sourceKey),
    m_reportTypeUUID(reportTypeUUID),
    m_content(content),
    m_cached(true)
{
}

TR::SourceKey LazyContentQuery::getSourceKey() const
{
    return m_sourceKey;
}

TR::ReportTypeUUID LazyContentQuery::getReportTypeUUID() const
{
    return m_reportTypeUUID;
}

void LazyContentQuery::run(CompletedHandler completedHandler, FailedHandler failedHandler)
{
    if (m_content)
    {
        completedHandler(m_content);
        return;
    }
    doRun(
        [this, completedHandler](std::shared_ptr<TR::Content> content){
            m_content = content;
            m_cached = true;
            completedHandler(m_content);
        }, 
        failedHandler
    );
}

void LazyContentQuery::cancel()
{
    doCancel();
}

LazyQueryBatch::LazyQueryBatch(stl_tools::container_initializer<std::unique_ptr<LazyContentQuery>> contentQueries):
    m_contentQueries(contentQueries.move_begin(), contentQueries.move_end())
{
}

void LazyQueryBatch::run(CompletedHandler completedHandler, FailedHandler failedHandler)
{
    if (m_contentQueries.empty())
    {
        completedHandler({});
    }

    try
    {
        auto contents = std::make_shared<Contents>(m_contentQueries.size());
        auto completed = std::make_shared<std::vector<bool>>(m_contentQueries.size(), false);

        for (size_t position = 0; position < m_contentQueries.size(); ++position)
        {
            m_contentQueries[position]->run(
                [contents, completed, position, completedHandler](std::shared_ptr<TR::Content> content) {
                    (*contents)[position] = content;
                    (*completed)[position] = true;

                    if (boost::range::find(*completed, false) == completed->end())
                    {
                        completedHandler(*contents);
                    }
                },

                [this, failedHandler](const std::string& err){
                    cancel();
                    failedHandler(err);
                }
            );
        }
    }
    catch (std::exception&)
    {
        cancel();
        throw;
    }
}

void LazyQueryBatch::cancel()
{
    for (auto& contentQuery : m_contentQueries)
    {
        contentQuery->cancel();
    }
}

CurrentContentQuery::CurrentContentQuery(AppClient* client, TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID):
    LazyContentQuery(client, sourceKey, reportTypeUUID)
{
}

CurrentContentQuery::CurrentContentQuery(TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID, std::shared_ptr<TR::Content> content):
    LazyContentQuery(sourceKey, reportTypeUUID, content)
{
}

void CurrentContentQuery::doRun(CompletedHandler completedHandler, FailedHandler failedHandler)
{
    _ASSERT(!m_job.is_running());
    m_job = m_client->queryCurrentContent(m_sourceKey, m_reportTypeUUID, completedHandler, failedHandler);
}

void CurrentContentQuery::doCancel()
{
    if (m_job.is_running())
    {
        m_job.cancel();
    }
}

ArchivedContentQuery::ArchivedContentQuery(AppClient* client, TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID, time_t time):
    LazyContentQuery(client, sourceKey, reportTypeUUID),
    m_time(time)
{
}

ArchivedContentQuery::ArchivedContentQuery(TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID, time_t time, std::shared_ptr<TR::Content> content):
    LazyContentQuery(sourceKey, reportTypeUUID, content),
    m_time(time)
{
}

time_t ArchivedContentQuery::get_time() const
{
    return m_time;
}

void ArchivedContentQuery::doRun(CompletedHandler completedHandler, FailedHandler failedHandler)
{
    _ASSERT(!m_job.is_running());
    m_job = m_client->queryArchivedContent(m_sourceKey, m_reportTypeUUID, m_time, completedHandler, failedHandler);
}

void ArchivedContentQuery::doCancel()
{
    if (m_job.is_running())
    {
        m_job.cancel();
    }
}

PatternQuery::PatternQuery(AppClient* client, TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID):
    LazyContentQuery(client, sourceKey, reportTypeUUID)
{
}

PatternQuery::PatternQuery(TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID, std::shared_ptr<TR::Content> content):
    LazyContentQuery(sourceKey, reportTypeUUID, content)
{
}

void PatternQuery::doRun(CompletedHandler completedHandler, FailedHandler failedHandler)
{
    completedHandler(m_client->getEffectivePattern(m_sourceKey, m_reportTypeUUID));
}

void PatternQuery::doCancel()
{
}

LazyStreamQuery::LazyStreamQuery(TR::SourceKey sourceKey, TR::StreamTypeUUID streamTypeUUID):
    m_sourceKey(sourceKey),
    m_streamTypeUUID(streamTypeUUID),
    m_cached(false)
{
}

LazyStreamQuery::LazyStreamQuery(TR::SourceKey sourceKey, TR::StreamTypeUUID streamTypeUUID, std::vector<TR::StreamMessage> streamMessages):
    m_sourceKey(sourceKey),
    m_streamTypeUUID(streamTypeUUID),
    m_streamMessages(std::move(streamMessages)),
    m_cached(true)
{
}

TR::SourceKey LazyStreamQuery::getSourceKey() const
{
    return m_sourceKey;
}

TR::StreamTypeUUID LazyStreamQuery::getStreamTypeUUID() const
{
    return m_streamTypeUUID;
}

void LazyStreamQuery::run(CompletedHandler completedHandler, FailedHandler failedHandler)
{
    if (m_cached)
    {
        completedHandler(m_streamMessages);
        return;
    }
    
    doRun(
        [this, completedHandler](std::vector<TR::StreamMessage> streamMessages){
            m_streamMessages = std::move(streamMessages);
            m_cached = true;
            completedHandler(m_streamMessages);
        },
        failedHandler
    );
}

void LazyStreamQuery::cancel()
{
    doCancel();
}

void LazyStreamQuery::doCancel()
{
    _ASSERT(false);
}

StreamSegmentQuery::StreamSegmentQuery(AppClient* client, TR::SourceKey sourceKey, TR::StreamTypeUUID streamTypeUUID, time_t start, time_t stop):
    LazyStreamQuery(sourceKey, streamTypeUUID),
    m_client(client),
    m_start(start),
    m_stop(stop)
{
}

StreamSegmentQuery::StreamSegmentQuery(TR::SourceKey sourceKey, TR::StreamTypeUUID streamTypeUUID, std::vector<TR::StreamMessage> streamMessages):
    LazyStreamQuery(sourceKey, streamTypeUUID, std::move(streamMessages)),
    m_client(nullptr),
    m_start(0),
    m_stop(0)
{
}

void StreamSegmentQuery::doRun(CompletedHandler completedHandler, FailedHandler failedHandler)
{
    _ASSERT(!m_job.is_running());
    m_job = m_client->queryStreamSegment(m_sourceKey, m_streamTypeUUID, m_start, m_stop,
        [this, completedHandler](TR::StreamSegment streamSegment){
            completedHandler(std::move(streamSegment.m_messages));
        },
        failedHandler
    );
}

void StreamSegmentQuery::doCancel()
{
    if (m_job.is_running())
    {
        m_job.cancel();
    }
}

bool isRoot(const TR::SourceInfo& sourceInfo)
{
    return sourceInfo.m_key == TR::ROOT_GROUP_KEY;
}

bool isSuitRoot(const TR::SourceInfo& sourceInfo)
{
    return sourceInfo.m_source_type_uuid == TR::Core::SuitRootSourceType::SOURCE_TYPE_UUID;
}

bool isSuit(const TR::SourceInfo& sourceInfo)
{
    return sourceInfo.m_source_type_uuid == TR::Core::SuitSourceType::SOURCE_TYPE_UUID;
}

std::wstring formatXPath(const TR::XmlNodeFilter& nodeFilter, const TR::XmlDefDoc& filterDef)
{
    if (auto def_node = static_cast<const TR::XML::XmlDefNode*>(xml_tools::find_xpath_node(filterDef, nodeFilter.m_xpath.c_str())))
    {
        return def_node->format_caption();
    }
    return L"Invalid";
}

std::wstring formatCondition(const TR::XmlNodeFilter& nodeFilter)
{
    std::wstring str;
    switch (nodeFilter.m_action)
    {
    case TR::XmlNodeFilter::Action::ACCEPT:
        str += L"accept ";
        break;

    case TR::XmlNodeFilter::Action::REJECT:
        str += L"reject ";
        break;

    default:
        _ASSERT(false);
    }

    switch (nodeFilter.m_predicate)
    {
    case TR::XmlNodeFilter::Predicate::EQUAL:
        str += L"equal ";
        break;

    case TR::XmlNodeFilter::Predicate::LESS:
        str += L"less ";
        break;

    case TR::XmlNodeFilter::Predicate::MORE:
        str += L"more ";
        break;

    case TR::XmlNodeFilter::Predicate::MATCH:
        str += L"match ";
        break;

    default:
        _ASSERT(false);
    }

    if (!nodeFilter.m_patterns.empty())
    {
        str += nodeFilter.m_patterns[0];
    }

    if (nodeFilter.m_patterns.size() > 1)
    {
        str += L" ...";
    }

    return str;
}