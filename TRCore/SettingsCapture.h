#pragma once
#include "XmlNodeFilter.h"
#include "ServicesDecl.h"
namespace TR { namespace Core {

class SettingsCapture
{
public:
    static const Priority SETTINGS_CAPTURE_PRIORITY = 2000;

    SettingsCapture(Database& db, Basis& basis, Registry& registry);
    ~SettingsCapture();

    void                                run(Executive* executive, Transaction& t);

    void                                enable_capture_source_settings(SourceTypeRef source_type_ref, Transaction& t);
    void                                disable_capture_source_settings(SourceTypeRef source_type_ref, Transaction& t);

private:
    friend class SourceSettingsCaptureJob;
    class CaptureManager;

    Database& m_db;
    Sources& m_sources;
    SourceTypes& m_source_types;
    ReportTypes& m_report_types;
    Generators& m_generators;
    Collector& m_collector;
    ResourcePolicies& m_resource_policies;
    SourceResources& m_source_resources;
    Executive* m_executive;
    bool m_running;

    std::unique_ptr<CaptureManager> m_capture_manager;
};

}} // namespace TR { namespace Core {