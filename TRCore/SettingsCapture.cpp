#include "stdafx.h"
#include "SettingsCapture.h"
#include "Basis.h"
#include "Registry.h"
#include "Transaction.h"
#include "JobManager.h"
#include "Executive.h"

namespace TR { namespace Core {

class SourceSettingsCapture: public Generator
{
public:
    SourceSettingsCapture(const ReportTypeRef& report_type_ref, const SourceTypeRef& source_type_ref):
        Generator(source_type_ref.get_uuid()),
        m_report_type_ref(report_type_ref),
        m_source_type_ref(source_type_ref)
    {
    }

    virtual SourceTypeRef get_source_type() const override
    {
        return m_source_type_ref;
    }

    virtual ReportTypeRef get_report_type() const override
    {
        return m_report_type_ref;
    }

    virtual Functor prepare_functor(SourceRef source_ref) const override
    {
        auto source = *source_ref;
        auto& report_type = dynamic_cast<const RegularReportType&>(*m_report_type_ref);

        return [config = source.get_config(), make_content = report_type.prepare_make_content_from_xml()]() mutable {
            return make_content(xml_tools::clone_doc(*config));
        };
    }   

private:
    ReportTypeRef m_report_type_ref;
    SourceTypeRef m_source_type_ref;
};

class SourceSettingsCaptureJob : public Job, public std::enable_shared_from_this<SourceSettingsCaptureJob>
{
public:
    class Routine
    {
    public:
        Routine(std::shared_ptr<SourceSettingsCaptureJob> job) :
            m_job(job),
            m_settings_capture(job->m_settings_capture),
            m_sources(job->m_settings_capture.m_sources),
            m_report_types(job->m_settings_capture.m_report_types),
            m_collector(job->m_settings_capture.m_collector),
            m_resource_policies(job->m_settings_capture.m_resource_policies),
            m_source_resources(job->m_settings_capture.m_source_resources),
            m_setup_uuid(stl_tools::null_uuid())
        {
        }

        std::function<std::unique_ptr<Content>()> prepare_functor()
        {
            auto job = m_job.lock();
            if (!job)
            {
                return nullptr;
            }

            auto source = m_sources.get_source(job->m_source_key);
            auto settings_report_uuid = ReportTypeUUID(source.get_source_type().get_uuid());
            if (!m_report_types.has_report_type(settings_report_uuid))
            {
                return nullptr;
            }

            auto& settings_report_type = m_report_types.get_report_type(settings_report_uuid);

            auto source_resource = m_source_resources.get_source_resource(source, settings_report_type);
            if (!source_resource.m_significant)
            {
                return nullptr;
            }

            if (m_setup_uuid == stl_tools::null_uuid())
            {
                if (auto current_settings_report = m_collector.find_current_report(source, settings_report_type))
                {                    
                    m_setup_uuid = current_settings_report->get_transformation_circumstances().m_setup_uuid;
                }                
            }

            if (m_setup_uuid == source_resource.m_setup_uuid)
            {
                return nullptr;
            }
            m_setup_uuid = source_resource.m_setup_uuid;

            auto generator = m_settings_capture.m_generators.find_by_output(settings_report_type);
            if (generator.size() != 1)
            {
                _ASSERT(false);
                return nullptr;
            }

            return generator.at(0)->prepare_functor(source);          
        }

        void on_completed(std::unique_ptr<Content> content) const
        {
            auto job = m_job.lock();
            if (!job)
            {
                return;
            }

            auto source = m_sources.get_source(job->m_source_key);
            auto settings_report_uuid = ReportTypeUUID(source.get_source_type().get_uuid());
            auto& report_type = m_report_types.get_report_type(settings_report_uuid);
            if (content)
            {
                m_collector.collect_report({ source, report_type, std::move(content), time(nullptr) }, { settings_report_uuid , m_setup_uuid, {} });
            }

            m_settings_capture.m_executive->run_processing(*this);
        }

        void on_failed(const std::exception& err) const
        {
            auto job = m_job.lock();
            if (!job)
            {
                return;
            }

            job->emit_failed(err);
        }

        void on_cancel() const
        {
            auto job = m_job.lock();
            if (!job)
            {
                return;
            }

            job->emit_completed();
        }

    private:
        std::weak_ptr<SourceSettingsCaptureJob> m_job;
        SettingsCapture& m_settings_capture;
        Sources& m_sources;
        ReportTypes& m_report_types;
        Collector& m_collector;
        ResourcePolicies& m_resource_policies;
        SourceResources& m_source_resources;
        UUID m_setup_uuid;
    };

    SourceSettingsCaptureJob(SettingsCapture& settings_capture, SourceRef source_ref) :
        m_settings_capture(settings_capture),
        m_source_key(source_ref.get_key())
    {
    }

    void do_run()
    {
        m_settings_capture.m_executive->run_processing(Routine(shared_from_this()));
    }

private:
    SettingsCapture& m_settings_capture;
    SourceKey m_source_key;
};

class SettingsCapture::CaptureManager
{
public:
    CaptureManager(SettingsCapture& settings_capture, Executive& executive) :
        m_settings_capture(settings_capture),
        m_executive(executive)
    {
    }

    void start_source_settings_capture(SourceRef source_ref)
    {
        auto strand = source_ref.get_key();
        if (auto current_job = m_job_manager.current_job(strand))
        {
            return;
        }

        m_job_manager.run_job(strand, std::make_shared<SourceSettingsCaptureJob>(m_settings_capture, source_ref));
    }

private:
    SettingsCapture& m_settings_capture;
    Executive& m_executive;
    JobManager<SourceKey> m_job_manager;
};

SettingsCapture::SettingsCapture(Database& db, Basis& basis, Registry& registry):
    m_db(db),
    m_sources(registry.m_sources),
    m_source_types(basis.m_source_types),
    m_report_types(basis.m_report_types),
    m_generators(basis.m_transformations.m_generators),
    m_collector(registry.m_collector),
    m_resource_policies(registry.m_resource_policies),
    m_source_resources(registry.m_source_resources),
    m_executive(nullptr),
    m_running(false)
{
}

SettingsCapture::~SettingsCapture() = default;

void SettingsCapture::run(Executive* executive, Transaction& t)
{
    _ASSERT(!m_executive);
    transact_assign(m_executive, executive, t);
    transact_assign(m_capture_manager, std::make_unique<CaptureManager>(*this, *executive), t);

    t.connect_commit([this] {
        m_running = true;

        for (auto& source : m_sources.get_sources())
        {
            m_capture_manager->start_source_settings_capture(source);
        }
    });

    m_sources.connect_add_source([this](SourceRef source_ref, Transaction& t) {
        auto& source_type = (*source_ref).get_source_type();
        auto report_type_uuid = ReportTypeUUID(source_type.get_uuid());
        if (!m_report_types.has_report_type(report_type_uuid))
        {
            return;
        }

        m_resource_policies.set_setup_uuid(source_ref, m_report_types.get_report_type(report_type_uuid), stl_tools::gen_uuid(), t);

        t.connect_commit([this, source_ref] {
            if (m_running)
            {
                m_capture_manager->start_source_settings_capture(source_ref);
            }
        });
    }, SETTINGS_CAPTURE_PRIORITY);

    m_sources.connect_update_source([this](SourceRef source_ref, Transaction& t) {
        auto& source_type = (*source_ref).get_source_type();
        auto report_type_uuid = ReportTypeUUID(source_type.get_uuid());
        if (!m_report_types.has_report_type(report_type_uuid))
        {
            return;
        }

        m_resource_policies.set_setup_uuid(source_ref, m_report_types.get_report_type(report_type_uuid), stl_tools::gen_uuid(), t);

        t.connect_commit([this, source_ref] {
            if (m_running)
            {
                m_capture_manager->start_source_settings_capture(source_ref);
            }
        });
    }, SETTINGS_CAPTURE_PRIORITY);
}

void SettingsCapture::enable_capture_source_settings(SourceTypeRef source_type_ref, Transaction& t)
{
    auto& source_type = *source_type_ref;
    auto settings_def = source_type.get_config_def();

    auto& report_type = m_report_types.add_report_type(std::make_unique<GeneralRegularReportType>(
        source_type.get_uuid(),
        source_type.get_name(),
        xml_tools::clone_doc(*settings_def)), t);

    m_generators.add_generator(std::make_unique<SourceSettingsCapture>(report_type, source_type), t);
}

void SettingsCapture::disable_capture_source_settings(SourceTypeRef source_type_ref, Transaction& t)
{
    auto& source_type = *source_type_ref;
    m_generators.remove_generator(source_type.get_uuid(), t);
    auto settings_report_uuid = ReportTypeUUID(source_type.get_uuid());
    m_report_types.remove_report_type(settings_report_uuid, t);
}

}} //namespace TR { namespace Core {