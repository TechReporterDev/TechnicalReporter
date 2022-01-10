#pragma once
#include "RegistryDecl.h"
#include "StreamTypes.h"
#include "StreamMessage.h"
namespace TR { namespace Core {
struct StreamRecordData;
struct StreamInputData;

struct StreamRecord
{
    StreamRecord(StreamTypeRef stream_type_ref, SourceRef source_ref, StreamMessage message);

    StreamTypeRef   m_stream_type_ref;
    SourceRef       m_source_ref;
    StreamMessage   m_message;
};

enum class StreamLength {   
    DEFAULT = 0,    
    FOREVER, 
    HOUR, 
    DAY, 
    MONTH, 
    YEAR, 
    NONE
};

class Reservoir
{
public:
    static const Priority RESERVOIR_PRIORITY = 550;

    Reservoir();
    ~Reservoir();

    void                            set_database(Database* db);
    void                            set_stream_types(StreamTypes* stream_types);
    void                            set_sources(Sources* sources);
    void                            run(Executive* executive, Transaction& t);

    void                            set_stream_length(SubjectRef subject_ref, StreamTypeRef stream_type_ref, StreamLength length, Transaction& t);
    StreamLength                    get_stream_length(SubjectRef subject_ref, StreamTypeRef stream_type_ref);
    StreamLength                    get_effective_stream_length(SubjectRef subject_ref, StreamTypeRef stream_type_ref);

    void                            append_input(SourceRef source_ref, StreamTypeRef stream_type_ref, const std::vector<StreamRecord>& stream_records, Transaction& t);
    Key                             read_input(SourceRef source_ref, StreamTypeRef stream_type_ref, std::vector<StreamRecord>& stream_records) const;
    void                            erase_input(SourceRef source_ref, StreamTypeRef stream_type_ref, Key key, Transaction& t);

    void                            append_record(const StreamRecord& stream_record, Transaction& t);
    std::vector<StreamRecord>       read_stream(SourceRef source_ref, StreamTypeRef stream_type_ref, time_t start, time_t stop) const;
    time_t                          get_stream_time(SourceRef source_ref, StreamTypeRef stream_type_ref) const;

    template<class T>
    void connect_append_record(T slot, Priority priority) { m_append_record_sig.connect(slot, priority); }

private:
    StreamRecordData                pack_stream_record(const StreamRecord& stream_record) const;
    StreamRecord                    unpack_stream_record(const StreamRecordData& stream_record_data) const;

    void                            check_stream_length_one_by_one();
    void                            check_next_stream_length(std::vector<std::pair<SourceKey, StreamTypeUUID>> targets);
    void                            check_one_stream_length(SourceKey source_key, StreamTypeUUID stream_type_uuid);

    void                            on_remove_stream_type(UUID source_type_uuid, Transaction& t);
    void                            on_remove_sources(const std::vector<SourceKey>& removed_sources, Transaction& t);

    Database* m_db;
    StreamTypes* m_stream_types;
    Sources* m_sources;
    Executive* m_executive;

    OrderedSignal<void(const StreamRecord& stream_record, Transaction& t)> m_append_record_sig;
};

}}//namespace TR {namespace Core {