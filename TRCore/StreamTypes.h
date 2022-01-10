#pragma once
#include "BackgndFwd.h"
#include "BasisFwd.h"
#include "BasisRefs.h"
#include "Content.h"
#include "Exception.h"
#include "stddecl.h"
namespace TR { namespace Core {
class StreamTypesStorage;

class StreamType
{
public:
    friend class StreamTypes;

    StreamType(StreamTypeUUID uuid, std::wstring name, std::unique_ptr<XmlDefDoc> message_def, int version = 0);
    virtual ~StreamType() = default;

    StreamTypeUUID                      get_uuid() const;
    std::wstring                        get_name() const;
    std::shared_ptr<const XmlDefDoc>    get_message_def() const;
    int                                 get_version() const;

    StreamTypeRef                       get_ref() const;
    operator StreamTypeRef() const; 

protected:
    StreamTypeUUID m_uuid;
    std::wstring m_name;
    std::shared_ptr<const XmlDefDoc> m_message_def;
    int m_version;

    boost::optional<StreamTypeRef> m_ref;
};

class StreamTypes
{
public:
    static const Priority STREAM_TYPES_PRIORITY = 101;
    using StreamTypeRange = boost::any_range<const StreamType, boost::forward_traversal_tag>;

    StreamTypes();
    ~StreamTypes();
    void                    set_database(Database* db);

    const StreamType&       add_stream_type(std::unique_ptr<StreamType> stream_type, Transaction& t);
    void                    remove_stream_type(StreamTypeUUID uuid, Transaction& t);
    bool                    has_stream_type(StreamTypeUUID uuid) const;
    const StreamType&       get_stream_type(StreamTypeUUID uuid) const;
    StreamTypeRange         get_stream_types() const;

    template<class T>
    void connect_add_stream_type(T slot, Priority priority) { m_add_stream_type_sig.connect(slot, priority); }

    template<class T>
    void connect_remove_stream_type(T slot, Priority priority) { m_remove_stream_type_sig.connect(slot, priority); }

private:
    Database* m_db;
    std::unique_ptr<StreamTypesStorage> m_storage;

    OrderedSignal<void(StreamTypeRef, Transaction&)> m_add_stream_type_sig;
    OrderedSignal<void(StreamTypeUUID, Transaction&)> m_remove_stream_type_sig;
};

}} //namespace TR { namespace Core {