#pragma once
#include "AppEventDisp.h"
#include "TRWebServer.h"
class AppClient;

class AppEventHandler: private TR::EventHandler
{
public:
    friend class AppClient;

    AppEventHandler(AppClient* client);
    AppEventHandler(const AppEventHandler&) = delete;
    AppEventHandler& operator = (const AppEventHandler&) = delete;
    
protected:
    virtual void onAddSource(const TR::SourceInfo& sourceInfo){}
    virtual void onUpdateSource(const TR::SourceInfo& sourceInfo){}
    virtual void onRemoveSource(TR::Key sourceKey, const std::vector<TR::SourceKey>& removedSources){}
    virtual void onAddLink(TR::Link link){}
    virtual void onRemoveLink(TR::Link link){}
    virtual void onUpdateCurrentReport(const TR::CurrentReportInfo& currentReportInfo, bool contentChanged){}

    AppClient* m_client;

private:
    // TR::EventHandler override
    virtual void on_add_source(const TR::SourceInfo& source_info) override final;
    virtual void on_update_source(const TR::SourceInfo& source_info) override final;
    virtual void on_remove_source(TR::Key source_key, const std::vector<TR::SourceKey>& removed_sources) override final;
    virtual void on_add_link(TR::Link link) override final;
    virtual void on_remove_link(TR::Link link) override final;
    virtual void on_update_current_report(const TR::CurrentReportInfo& current_report_info, bool content_changed) override final;
};

class AppClient
{
public:
    AppClient(AppEventDisp& appEventDisp);
    AppClient(const AppClient&) = delete;
    AppClient& operator = (const AppClient&) = delete;

    void                                        connect(TR::Core::Core& core, const std::wstring& user);
    TR::DirectClient&                           getDC();

    // security
    std::vector<TR::UserInfo>                   getUsers() const;
    TR::Key                                     addUser(const TR::UserSettings& userSettings);
    void                                        updateUser(TR::Key userKey, const TR::UserSettings& userSettings);
    void                                        removeUser(TR::Key userKey);

    TR::SourceAccessInfo                        getAccessInfo(TR::SourceKey sourceKey) const;
    std::vector<TR::AccessEntry>                getAccessEntries(TR::SourceKey SourceKey) const;
    virtual void                                setAccessList(const TR::SourceAccessList& sourceAccessList) const;

    // summary info
    TR::SummaryInfo                             getSummaryInfo() const;

    // report type info
    std::vector<TR::ReportTypeInfo>             getReportTypesInfo() const;
    TR::ReportTypeInfo                          getReportTypeInfo(TR::ReportTypeUUID reportTypeUUID) const;
    std::shared_ptr<const TR::XmlDoc>           getContentDef(TR::ReportTypeUUID reportTypeUUID) const;

    // stream type info
    std::vector<TR::StreamTypeInfo>             getStreamTypesInfo() const;
    TR::StreamTypeInfo                          getStreamTypeInfo(TR::StreamTypeUUID streamTypeUUID) const;
    std::shared_ptr<const TR::XmlDefDoc>        getMessageDef(TR::StreamTypeUUID stream_type_uuid) const;

    // action info
    std::vector<TR::ActionInfo>                 getActionsInfo() const;
    TR::ActionInfo                              getActionInfo(TR::ActionUUID actionUUID) const;
    TR::Params                                  getDefaultParams(TR::ActionUUID actionUUID) const;
    TR::Params                                  createParams(TR::ActionUUID actionUUID, const std::string& paramsXML) const;

    // source type info
    std::vector<TR::SourceTypeInfo>             getSourceTypesInfo() const;
    TR::SourceTypeInfo                          getSourceTypeInfo(TR::UUID sourceTypeUUID) const;
    std::shared_ptr<const TR::XmlDefDoc>        getConfigDef(TR::UUID sourceTypeUUID) const;
    TR::SourceConfig                            getDefaultConfig(TR::UUID sourceTypeUUID) const;

    // roles info
    std::vector<TR::RoleInfo>                   getRolesInfo() const;
    TR::RoleInfo                                getRoleInfo(TR::RoleKey roleKey) const;
    TR::RoleKey                                 addRole(const TR::RoleSettings& roleSettings) const;
    void                                        updateRole(TR::RoleKey roleKey, const TR::RoleSettings& roleSettings) const;
    void                                        removeRole(TR::RoleKey roleKey);

    // source tree access
    std::vector<TR::SourceInfo>                 getSourcesInfo() const;
    TR::SourceInfo                              getSourceInfo(TR::SourceKey sourceKey) const;
    TR::SourceKey                               addSource(const TR::SourceSettings& sourceSettings);
    void                                        updateSource(TR::SourceKey sourceKey, const TR::SourceSettings& sourceSettings);
    void                                        removeSource(TR::SourceKey sourceKey);
    TR::SourceConfig                            getSourceConfig(TR::SourceKey sourceKey) const;
    void                                        setSourceConfig(TR::SourceKey sourceKey, TR::SourceConfig config);

    // source link
    std::vector<TR::Link>                       getLinks() const;
    void                                        addLink(TR::Link link);
    void                                        removeLink(TR::Link link);

    // report info
    TR::CurrentReportInfo                       getCurrentReport(TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID) const;
    std::shared_ptr<TR::Content>                getCurrentContent(TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID) const;
    TR::Client::Job                             queryCurrentContent(TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID, TR::Client::ContentReadyHandler completedHandler, TR::Client::JobFailedHandler failedHandler) const;
    
    // archive
    std::vector<TR::ArchivedReportInfo>         getArchive(TR::SourceKey sourceKey) const;
    TR::ArchivedReportInfo                      getArchivedReport(TR::ArchivedReportKey archivedReportKey) const;
    std::shared_ptr<TR::Content>                getArchivedContent(TR::ArchivedReportKey archivedReportKey) const;
    TR::Client::Job                             queryArchivedContent(TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID, time_t time, TR::Client::ContentReadyHandler completedHandler, TR::Client::JobFailedHandler failedHandler) const;

    // stream info
    TR::StreamInfo                              getStreamInfo(TR::SourceKey sourceKey, TR::StreamTypeUUID streamTypeUUID) const;
    TR::Client::Job                             queryStreamSegment(TR::SourceKey sourceKey, TR::StreamTypeUUID streamTypeUUID, time_t start, time_t stop, TR::Client::StreamReadyHandler completedHandler, TR::Client::JobFailedHandler failedHandler) const;

    // options
    TR::SubjectReportOptions                    getOptions(TR::SubjectKey subjectKey, TR::ReportTypeUUID reportTypeUUID) const;
    TR::SubjectStreamOptions                    getOptions(TR::SubjectKey subjectKey, TR::StreamTypeUUID streamTypeUUID) const;
    void                                        setOptions(TR::SubjectKey subjectKey, const TR::ReportOptions& reportOptions);
    void                                        setOptions(TR::SubjectKey subjectKey, const TR::StreamOptions& streamOptions);
    void                                        setOptions(TR::SubjectKey subjectKey, const TR::ActionOptions& shortcutOptions);
    
    TR::SubjectOptions                          getSubjectOptions(TR::SubjectKey subjectKey) const;
    TR::ReportTypeOptions                       getReportTypeOptions(TR::ReportTypeUUID reportTypeUUID) const;
    TR::StreamTypeOptions                       getStreamTypeOptions(TR::StreamTypeUUID streamTypeUUID) const;
    TR::ActionTypeOptions                       getActionOptions(TR::ActionUUID actionUUID, boost::optional<TR::UUID> shortcutUUID) const;
    
    // custom query
    TR::CustomQueryInfo                         getCustomQueryInfo(TR::Key customQueryKey) const;
    TR::CustomQueryDefinition                   getCustomQueryDef(TR::Key customQueryKey) const;
    TR::CustomQueryDefinition                   createCustomQuery(std::vector<TR::ReportTypeUUID> inputUUIDs) const;
    TR::Key                                     addCustomQuery(std::wstring name, TR::CustomQueryDefinition customQueryDef);
    void                                        removeCustomQuery(TR::Key customQueryKey);

    //custom filter
    TR::CustomFilterInfo                        getCustomFilterInfo(TR::Key customFilterKey) const;
    TR::Key                                     addCustomFilter(std::wstring name, TR::ReportTypeUUID inputUUID, TR::CustomFilterSetup customFilterSetup);
    TR::CustomFilterSetup                       createFilterSetup(TR::ReportTypeUUID inputUUID) const;
    TR::CustomFilterSetup                       getFilterSetup(TR::Key customFilterKey) const;
    void                                        setFilterSetup(TR::Key customFilterKey, TR::CustomFilterSetup customFilterSetup) const;

    //tracking stream
    TR::TrackingStreamInfo                      getTrackingStreamInfo(TR::Key trackingStreamKey) const;
    TR::Key                                     addTrackingStream(std::wstring name, TR::ReportTypeUUID inputUUID, std::string input_xpath);

    //stream filter
    TR::StreamFilterInfo                        getStreamFilterInfo(TR::Key streamFilterKey) const;
    TR::StreamFilterSetup                       createFilterSetup(TR::StreamTypeUUID inputUUID) const;
    TR::Key                                     addStreamFilter(std::wstring name, TR::StreamTypeUUID  inputUUID, TR::StreamFilterSetup streamFilterSetup);
    TR::StreamFilterSetup                       getStreamFilterSetup(TR::Key streamFilterKey) const;
    void                                        setStreamFilterSetup(TR::Key streamFilterKey, TR::StreamFilterSetup streamFilterSetup);

    // compliance
    std::vector<TR::ComplianceInfo>             getCompliancesInfo() const;
    TR::ComplianceInfo                          getComplianceInfo(TR::ComplianceKey complianceKey) const;
    TR::ComplianceSetup                         getComplianceSetup(TR::SubjectKey subjectKey, TR::ComplianceKey complianceKey) const;
    void                                        setComplianceSetup(TR::SubjectKey subjectKey, TR::ComplianceKey complianceKey, TR::ComplianceSetup complianceSetup);
    
    // validation               
    TR::ValidationInfo                          getValidationInfo(TR::ValidationKey validationKey) const;
    TR::ValidationKey                           addValidation(std::wstring name, TR::ReportTypeUUID inputUUID, TR::ValidationSettings defaultSettings) const;
    TR::ValidationSettings                      createValidationSettings(TR::ReportTypeUUID inputUUID) const;
    TR::ValidationSettings                      getValidationSettings(TR::SubjectKey subjectKey, TR::ValidationKey validationKey) const;
    void                                        setValidationSettings(TR::SubjectKey subjectKey, TR::ValidationKey validationKey, TR::ValidationSettings validationSettings);

    // grouping
    TR::GroupingInfo                            getGroupingInfo(TR::Key groupingKey) const;
    TR::Key                                     addGrouping(std::wstring name, TR::ReportTypeUUID inputUUID);

    //custom report 
    void                                        removeCustomReport(TR::ReportTypeUUID reportTypeUUID, bool recursive);
    
    // integrity checking 
    std::shared_ptr<TR::Content>                createPattern(TR::ReportTypeUUID reportTypeUUID) const;
    std::shared_ptr<TR::Content>                createPattern(TR::ReportTypeUUID reportTypeUUID, TR::Blob blob) const;
    std::shared_ptr<TR::Content>                getPattern(TR::SubjectKey subjectKey, TR::ReportTypeUUID reportTypeUUID) const;
    std::shared_ptr<TR::Content>                getEffectivePattern(TR::SubjectKey subjectKey, TR::ReportTypeUUID reportTypeUUID) const;
    void                                        setPattern(TR::SubjectKey subjectKey, TR::ReportTypeUUID reportTypeUUID, TR::Blob blob);
    void                                        resetPattern(TR::SubjectKey subjectKey, TR::ReportTypeUUID reportTypeUUID);

    // custom actions
    TR::UUID                                    addCustomAction(std::wstring name, TR::ActionUUID actionUUID, TR::Params params);
    void                                        removeCustomAction(TR::UUID customActionUUID);

    // scheduler
    std::vector<TR::ScheduleInfo>               getSchedulesInfo() const;
    TR::ScheduleInfo                            getScheduleInfo(TR::Key scheduleKey) const;
    TR::Key                                     addSchedule(const TR::ScheduleSettings& scheduleSettings);
    void                                        updateSchedule(TR::Key scheduleKey, const TR::ScheduleSettings& scheduleSettings);
    void                                        removeSchedule(TR::Key scheduleKey);

    // operations
    TR::Client::Job                             reloadReport(TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID, TR::Client::ContentReadyHandler completedHandler, TR::Client::JobFailedHandler failedHandler);
    TR::Client::Job                             rebuildReport(TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID, TR::Client::JobCompletedHandler completedHandler, TR::Client::JobFailedHandler failedHandler);
    TR::Client::Job                             launchAction(TR::SourceKey sourceKey, TR::ActionUUID actionUUID, TR::Params params, TR::Client::ContentReadyHandler completedHandler, TR::Client::JobFailedHandler failedHandler);
    TR::Client::Job                             launchShortcut(TR::SourceKey sourceKey, TR::UUID shortcutUUID, TR::Client::ContentReadyHandler completedHandler, TR::Client::JobFailedHandler failedHandler);

    //export
    TR::Client::Job                             exportSummary(TR::Client::ContentReadyHandler completedHandler, TR::Client::JobFailedHandler failedHandler);

    // features control
    std::vector<TR::FeatureInfo>                getFeaturesInfo() const;
    TR::FeatureInfo                             getFeatureInfo(TR::UUID featureUUID) const;
    void                                        installFeature(TR::UUID featureUUID);
    void                                        uninstallFeature(TR::UUID featureUUID);

private:
    TR::DirectClient m_dc;
};

class LazyContentQuery
{
public:
    using CompletedHandler = std::function<void(std::shared_ptr<TR::Content>)>;
    using FailedHandler = std::function<void(const std::string&)>;

    LazyContentQuery(AppClient* client, TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID);
    LazyContentQuery(TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID, std::shared_ptr<TR::Content> content);
    virtual ~LazyContentQuery() = default;

    TR::SourceKey       getSourceKey() const;
    TR::ReportTypeUUID  getReportTypeUUID() const;

    void                run(CompletedHandler completedHandler, FailedHandler failedHandler);
    void                cancel();

protected:
    virtual void doRun(CompletedHandler completedHandler, FailedHandler failedHandler) = 0;
    virtual void doCancel() = 0;

    AppClient* m_client;
    TR::SourceKey m_sourceKey;
    TR::ReportTypeUUID m_reportTypeUUID;
    std::shared_ptr<TR::Content> m_content;
    bool m_cached;
};

class LazyQueryBatch
{
public:
    using Contents = std::vector<std::shared_ptr<TR::Content>>;
    using CompletedHandler = std::function<void(Contents)>;
    using FailedHandler = std::function<void(const std::string&)>;

    LazyQueryBatch(stl_tools::container_initializer<std::unique_ptr<LazyContentQuery>> contentQueries);

    void run(CompletedHandler completedHandler, FailedHandler failedHandler);
    void cancel();

private:
    std::vector<std::unique_ptr<LazyContentQuery>> m_contentQueries;
};

class CurrentContentQuery: public LazyContentQuery
{
public:
    CurrentContentQuery(AppClient* client, TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID);
    CurrentContentQuery(TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID, std::shared_ptr<TR::Content> content);

protected:
    virtual void doRun(CompletedHandler completedHandler, FailedHandler failedHandler) override;
    virtual void doCancel() override;
    TR::Client::Job m_job;
};

class ArchivedContentQuery: public LazyContentQuery
{
public:
    ArchivedContentQuery(AppClient* client, TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID, time_t time);
    ArchivedContentQuery(TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID, time_t time, std::shared_ptr<TR::Content> content);
    
    time_t get_time() const;

protected:
    virtual void doRun(CompletedHandler completedHandler, FailedHandler failedHandler) override;
    virtual void doCancel() override;

    time_t m_time;
    TR::Client::Job m_job;
};

class PatternQuery: public LazyContentQuery
{
public:
    PatternQuery(AppClient* client, TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID);
    PatternQuery(TR::SourceKey sourceKey, TR::ReportTypeUUID reportTypeUUID, std::shared_ptr<TR::Content> content);

protected:
    virtual void doRun(CompletedHandler completedHandler, FailedHandler failedHandler) override;
    virtual void doCancel() override;
};

class LazyStreamQuery
{
public:
    using CompletedHandler = std::function<void(const std::vector<TR::StreamMessage>&)>;
    using FailedHandler = std::function<void(const std::string&)>;

    LazyStreamQuery(TR::SourceKey sourceKey, TR::StreamTypeUUID streamTypeUUID);
    LazyStreamQuery(TR::SourceKey sourceKey, TR::StreamTypeUUID streamTypeUUID, std::vector<TR::StreamMessage> streamMessages);
    virtual ~LazyStreamQuery() = default;
    
    TR::SourceKey       getSourceKey() const;
    TR::StreamTypeUUID  getStreamTypeUUID() const;

    void                run(CompletedHandler completedHandler, FailedHandler failedHandler);
    void                cancel();

protected:
    virtual void doRun(CompletedHandler completedHandler, FailedHandler failedHandler) = 0;
    virtual void doCancel();
        
    TR::SourceKey m_sourceKey;
    TR::StreamTypeUUID m_streamTypeUUID;
    std::vector<TR::StreamMessage> m_streamMessages;
    bool m_cached;
};

class StreamSegmentQuery: public LazyStreamQuery
{
public:
    StreamSegmentQuery(AppClient* client, TR::SourceKey sourceKey, TR::StreamTypeUUID streamTypeUUID, time_t start, time_t stop);
    StreamSegmentQuery(TR::SourceKey sourceKey, TR::StreamTypeUUID streamTypeUUID, std::vector<TR::StreamMessage> streamMessages);

protected:
    virtual void doRun(CompletedHandler completedHandler, FailedHandler failedHandler) override;
    virtual void doCancel() override;

    AppClient* m_client;
    time_t m_start;
    time_t m_stop;
    TR::Client::Job m_job;
};

// tools
using SourceInfoByKey = std::map<TR::SourceKey, TR::SourceInfo>;
using ReportTypeInfoByUUID = std::map<TR::ReportTypeUUID, TR::ReportTypeInfo>;
using StreamTypeInfoByUUID = std::map<TR::StreamTypeUUID, TR::StreamTypeInfo>;
using ActionInfoByUUID = std::map<TR::ActionUUID, TR::ActionInfo>;
using ShortcutInfoByUUID = std::map<TR::UUID, TR::ActionShortcutInfo>;

bool            isRoot(const TR::SourceInfo& sourceInfo);
bool            isSuitRoot(const TR::SourceInfo& sourceInfo);
bool            isSuit(const TR::SourceInfo& sourceInfo);

std::wstring    formatXPath(const TR::XmlNodeFilter& nodeFilter, const TR::XmlDefDoc& filterDef);
std::wstring    formatCondition(const TR::XmlNodeFilter& nodeFilter);