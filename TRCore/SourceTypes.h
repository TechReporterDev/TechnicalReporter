#pragma once
#include "BasisFwd.h"
#include "BasisRefs.h"
#include "BackgndFwd.h"
#include "stddecl.h"
namespace TR { namespace Core {
class Content;
class SourceRef;
class SourceTypeStorage;

class StreamTransport
{
public:
    StreamTransport(StreamTypeRef stream_type_ref);
    virtual ~StreamTransport() = default;
    StreamTypeRef get_stream_type() const;

protected:
    StreamTypeRef m_stream_type_ref;
};

class SourceType
{
public: 
    friend class SourceTypes;
    using DownloadFunctor = std::function<std::unique_ptr<Content>(const XmlDoc& source_config)>;
    using ActionFunctor = std::function<std::unique_ptr<Content>(const XmlDoc& params, const XmlDoc& source_config)>;

    SourceType(UUID uuid, std::wstring name, std::unique_ptr<XmlPropertiesDef> config_def, UUID family_uuid);
    virtual ~SourceType() = default;

    UUID                                        get_uuid() const;
    std::wstring                                get_name() const;
    std::shared_ptr<const XmlPropertiesDef>     get_config_def() const;
    std::unique_ptr<XmlPropertiesDoc>           get_default_config() const;
    UUID                                        get_family() const;
    
    SourceTypeRef                               get_ref() const;
    operator SourceTypeRef() const;

    // override
    virtual std::vector<ReportTypeRef>          get_downloads() const = 0;
    virtual DownloadFunctor                     prepare_download(ReportTypeRef report_type_ref, SourceRef source_ref) const = 0;

    virtual std::vector<StreamTypeRef>          get_streams() const = 0;
    virtual const StreamTransport&              get_transport(StreamTypeRef stream_type_ref) const = 0;

    virtual std::vector<ActionRef>              get_actions() const = 0;
    virtual std::vector<ReportTypeRef>          get_reloads(ActionRef action_ref) const = 0;
    virtual ActionFunctor                       prepare_action(ActionRef action_ref, SourceRef source_ref) const = 0;

protected:
    SourceType(const SourceType&) = default;
    //SourceType(SourceType&&) = default; //not supported in MSVC2013 
    SourceType& operator = (const SourceType&) = default;
    //SourceType& operator = (SourceType&&) = default; //not supported in MSVC2013 

    UUID m_uuid;
    std::wstring m_name;
    std::shared_ptr<XmlPropertiesDef> m_config_def;
    UUID m_family_uuid;

    boost::optional<SourceTypeRef> m_ref;
};

class PoolingTransport: public StreamTransport
{
public:
    using PoolingFunctor = std::function<std::vector<StreamMessage>(const XmlDoc& source_config)>;

    PoolingTransport(StreamTypeRef stream_type_ref);
    virtual PoolingFunctor prepare_pooling() const = 0;
};

class GroupSourceType: public SourceType
{
public:
    static const UUID s_uuid;
    static const std::shared_ptr<XmlPropertiesDef> s_config_def;

    GroupSourceType();

    virtual std::vector<ReportTypeRef>                  get_downloads() const override;
    virtual DownloadFunctor                             prepare_download(ReportTypeRef report_type_ref, SourceRef source_ref) const override;

    virtual std::vector<StreamTypeRef>                  get_streams() const override;
    virtual const StreamTransport&                      get_transport(StreamTypeRef stream_type_ref) const override;

    virtual std::vector<ActionRef>                      get_actions() const override;
    virtual std::vector<ReportTypeRef>                  get_reloads(ActionRef action_ref) const override;
    virtual ActionFunctor                               prepare_action(ActionRef action_ref, SourceRef source_ref) const override;
};

class SourceTypes
{
public:
    static const Priority SOURCE_TYPES_PRIORITY = 300;
    using SourceTypeRange = boost::any_range<const SourceType, boost::forward_traversal_tag>;

    SourceTypes();
    ~SourceTypes();
    void                    set_database(Database* db);
    void                    set_report_types(ReportTypes* report_types);
    void                    set_actions(Actions* action);


    const SourceType&       add_source_type(std::unique_ptr<SourceType> source_type);
    const SourceType&       add_source_type(std::unique_ptr<SourceType> source_type, Transaction& t);
    void                    remove_source_type(UUID uuid);
    void                    remove_source_type(UUID uuid, Transaction& t);
    bool                    has_source_type(UUID uuid) const;
    const SourceType&       get_source_type(UUID uuid) const;
    SourceTypeRange         get_source_types() const;

    template<class T>
    void connect_add_source_type(T slot, Priority priority) { m_add_source_type_sig.connect(slot, priority); }

    template<class T>
    void connect_remove_source_type(T slot, Priority priority) { m_remove_source_type_sig.connect(slot, priority); }

private:
    Database* m_db;
    std::unique_ptr<SourceTypeStorage> m_storage;

    OrderedSignal<void(SourceTypeRef, Transaction&)> m_add_source_type_sig;
    OrderedSignal<void(UUID, Transaction&)> m_remove_source_type_sig;
};

}} //namespace TR { namespace Core {