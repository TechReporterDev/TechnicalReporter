#pragma once
#include "RegistryDecl.h"
namespace TR { namespace Core {

class Conveyor
{
public:
    static const Priority CONVEYOR_PRIORITY = 1300;

    Conveyor(Database& db, Basis& basis);
    ~Conveyor();
    
    void    set_sources(Sources* sources);
    void    set_links(Links* links);
    void    set_collector(Collector* collector);
    void    set_reservoir(Reservoir* reservoir);
    void    set_source_resources(SourceResources* source_resources);
    void    run(Executive* executive, Transaction& t);

private:
    friend class TransformationJob; 
    friend class TrackingJob;
    class TransformManager;

    TransformationCircumstances get_transformation_circumstances(const SourceResource& source_resource) const;

    void    on_update_current_report(CurrentReportRef current_report_ref, bool content_changed, Transaction& t);
    void    on_append_record(const StreamRecord& stream_record, Transaction& t);
    void    on_update_source_resource(SourceRef source_ref, ResourceRef resource_ref, Transaction& t);
    void    on_update_childs(SourceRef source_ref, Transaction& t);

    Database& m_db;
    Basis& m_basis; 
    Transformations& m_transformations;
    SourceTypeActivities& m_source_type_activities;
    Sources* m_sources;
    Links* m_links;
    Collector* m_collector;
    Reservoir* m_reservoir;
    SourceResources* m_source_resources;
    Executive* m_executive;
    bool m_running;
    std::unique_ptr<TransformManager> m_transform_manager;
};

}} //namespace TR { namespace Core {