#pragma once
#include "stddecl.h"
#include "Features.h"
#include "ComposedFeature.h"
#include "SourceTypes.h"
#include "Content.h"
namespace TR { namespace Core {

class SuitRootSourceType: public SourceType
{
public:
    static const SourceTypeUUID SOURCE_TYPE_UUID;
    SuitRootSourceType();
    
    // override
    virtual std::vector<ReportTypeRef>      get_downloads() const override;
    virtual DownloadFunctor                 prepare_download(ReportTypeRef report_type_ref, SourceRef source_ref) const override;

    virtual std::vector<StreamTypeRef>      get_streams() const override;
    virtual const StreamTransport&          get_transport(StreamTypeRef stream_type_ref) const override;

    virtual std::vector<ActionRef>          get_actions() const override;
    virtual std::vector<ReportTypeRef>      get_reloads(ActionRef action_ref) const override;
    virtual ActionFunctor                   prepare_action(ActionRef action_ref, SourceRef source_ref) const override;
};

class SuitSourceType: public SourceType
{
public:
    static const SourceTypeUUID SOURCE_TYPE_UUID;
    SuitSourceType();

    // override
    virtual std::vector<ReportTypeRef>      get_downloads() const override;
    virtual DownloadFunctor                 prepare_download(ReportTypeRef report_type_ref, SourceRef source_ref) const override;

    virtual std::vector<StreamTypeRef>      get_streams() const override;
    virtual const StreamTransport&          get_transport(StreamTypeRef stream_type_ref) const override;

    virtual std::vector<ActionRef>          get_actions() const override;
    virtual std::vector<ReportTypeRef>      get_reloads(ActionRef action_ref) const override;
    virtual ActionFunctor                   prepare_action(ActionRef action_ref, SourceRef source_ref) const override;
};

class SuitsFeature: public ComposedFeature
{
public:
    static UUID get_feature_uuid();
    SuitsFeature();

protected:
    virtual void    install(CoreImpl& core_impl, bool restore, Transaction& t) override;
    virtual void    uninstall(CoreImpl& core_impl, Transaction& t) override;
};

}} //namespace TR { namespace Core {