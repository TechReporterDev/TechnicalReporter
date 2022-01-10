#include "stdafx.h"
#include "Reservoir.h"
#include "Sources.h"
#include "StreamTypes.h"
#include "Transaction.h"
#include "Executive.h"
#include "JobManager.h"
#include "DateTime.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
#include "StreamSettingsData-odb.hxx"
#include "StreamRecordData-odb.hxx"
#include "StreamInputData-odb.hxx"
namespace TR { namespace Core {

StreamRecord::StreamRecord(StreamTypeRef stream_type_ref, SourceRef source_ref, StreamMessage message):
    m_stream_type_ref(stream_type_ref),
    m_source_ref(source_ref),
    m_message(std::move(message))
{
}

Reservoir::Reservoir():
    m_db(nullptr),
    m_stream_types(nullptr),
    m_sources(nullptr),
    m_executive(nullptr)
{
}

Reservoir::~Reservoir()
{
}

void Reservoir::set_database(Database* db)
{
    _ASSERT(!m_db);
    m_db = db;
}

void Reservoir::set_stream_types(StreamTypes* stream_types)
{
    static_assert(RESERVOIR_PRIORITY > StreamTypes::STREAM_TYPES_PRIORITY, "Wrong component priority");
    _ASSERT(!m_stream_types);
    m_stream_types = stream_types;
    m_stream_types->connect_remove_stream_type([this](UUID uuid, Transaction& t){
        on_remove_stream_type(uuid, t);
    }, RESERVOIR_PRIORITY);
}

void Reservoir::set_sources(Sources* sources)
{
    static_assert(RESERVOIR_PRIORITY > Sources::SOURCES_PRIORITY, "Wrong component priority");
    _ASSERT(!m_sources);
    m_sources = sources;    
    m_sources->connect_remove_source([this](SourceKey source_key, const std::vector<SourceKey>& removed_sources, Transaction& t){
        on_remove_sources(removed_sources, t);
    }, RESERVOIR_PRIORITY);
}

void Reservoir::run(Executive* executive, Transaction& t)
{
    _ASSERT(!m_executive);
    transact_assign(m_executive, executive, t);

    t.connect_commit([this]()
    {
        check_stream_length_one_by_one();
    });
    //to do checking of stream length
}

void Reservoir::set_stream_length(SubjectRef subject_ref, StreamTypeRef stream_type_ref, StreamLength length, Transaction& t)
{
    StreamSettingsData stream_settings_data({pack(subject_ref), pack(stream_type_ref)}, StreamLength::DEFAULT);
    auto found = m_db->find<StreamSettingsData>(stream_settings_data.m_stream_settings_identity, stream_settings_data);
    stream_settings_data.m_length = length;
    if (found)
    {
        m_db->update(stream_settings_data);
    }
    else
    {
        m_db->persist(stream_settings_data);
    }
}

StreamLength Reservoir::get_stream_length(SubjectRef subject_ref, StreamTypeRef stream_type_ref)
{
    ReadOnlyTransaction t(*m_db);
    StreamSettingsData stream_data;
    m_db->find<StreamSettingsData>({pack(subject_ref), pack(stream_type_ref)}, stream_data);
    return stream_data.m_length;
}

StreamLength Reservoir::get_effective_stream_length(SubjectRef subject_ref, StreamTypeRef stream_type_ref)
{
    ReadOnlyTransaction t(*m_db);
    auto stream_length = get_stream_length(subject_ref, stream_type_ref);
    if (stream_length != StreamLength::DEFAULT)
    {
        return stream_length;
    }

    if (auto base_ref = get_base_ref(subject_ref))
    {
        return get_effective_stream_length(*base_ref, stream_type_ref);
    }

    return StreamLength::FOREVER;
}

void Reservoir::append_input(SourceRef source_ref, StreamTypeRef stream_type_ref, const std::vector<StreamRecord>& stream_records, Transaction& t)
{
    for (const auto& stream_record : stream_records)
    {
        odb::persist(*m_db, 
            StreamInputData(
                0,
                pack(source_ref),
                pack(stream_type_ref),
                pack_stream_record(stream_record)));
    }
}

Key Reservoir::read_input(SourceRef source_ref, StreamTypeRef stream_type_ref, std::vector<StreamRecord>& stream_records) const
{
    ReadOnlyTransaction t(*m_db);
    stream_records.clear();
    Key key = 0;
    for (auto& stream_input_data : m_db->query<StreamInputData>(
        (odb::query<StreamInputData>::source_key.value == source_ref.get_key() &&
        odb::query<StreamInputData>::stream_type_uuid.value == stream_type_ref.get_uuid()) + "LIMIT 10"))
    {
        stream_records.emplace_back(unpack_stream_record(stream_input_data.m_input_record));
        key = stream_input_data.m_key;
    }   

    return key;
}

void Reservoir::erase_input(SourceRef source_ref, StreamTypeRef stream_type_ref, Key key, Transaction& t)
{
    m_db->erase_query<StreamInputData>(
        odb::query<StreamInputData>::source_key.value == source_ref.get_key() &&
        odb::query<StreamInputData>::stream_type_uuid.value == stream_type_ref.get_uuid() &&
        odb::query<StreamInputData>::key <= key);
}

void Reservoir::append_record(const StreamRecord& stream_record, Transaction& t)
{
    if (get_effective_stream_length(stream_record.m_source_ref, stream_record.m_stream_type_ref) != StreamLength::NONE)
    {
        odb::persist(*m_db, pack_stream_record(stream_record));
    }   
    m_append_record_sig(stream_record, t);
}

std::vector<StreamRecord> Reservoir::read_stream(SourceRef source_ref, StreamTypeRef stream_type_ref, time_t start, time_t stop) const
{
    ReadOnlyTransaction t(*m_db);
    std::vector<StreamRecord> stream_records;
    for (auto& stream_record_data : m_db->query<StreamRecordData>(
        odb::query<StreamRecordData>::source_key.value == source_ref.get_key() &&
        odb::query<StreamRecordData>::stream_type_uuid.value == stream_type_ref.get_uuid() &&
        odb::query<StreamRecordData>::time >= start && odb::query<StreamRecordData>::time < stop))
    {
        stream_records.emplace_back(unpack_stream_record(stream_record_data));
    }
    return stream_records;
}

time_t Reservoir::get_stream_time(SourceRef source_ref, StreamTypeRef stream_type_ref) const
{
    return 0;
}

StreamRecordData Reservoir::pack_stream_record(const StreamRecord& stream_record) const
{
    return StreamRecordData(
        0,
        pack(stream_record.m_source_ref),
        pack(stream_record.m_stream_type_ref),
        stream_record.m_message.m_body->as_string(),
        stream_record.m_message.m_time
    );
}

StreamRecord Reservoir::unpack_stream_record(const StreamRecordData& stream_record_data) const
{
    auto stream_type = m_stream_types->get_stream_type(stream_record_data.m_stream_type_uuid);
    return StreamRecord(        
        stream_type,
        unpack(stream_record_data.m_source_key, m_sources),     
        StreamMessage(StreamMessage::Severety::INFO, L"", stream_record_data.m_time, std::make_shared<GeneralRegularContent>(stream_record_data.m_message, stream_type.get_message_def()))
    );
}

void Reservoir::check_stream_length_one_by_one()
{
    no_except([&]{
        std::vector<std::pair<SourceKey, StreamTypeUUID>> targets;
        for (auto& source : m_sources->get_sources())
        {
            for (auto& stream_type : m_stream_types->get_stream_types())
            {
                targets.push_back({source.get_ref().get_key(), stream_type.get_uuid()});
            }
        }

        m_executive->async(std::bind(&Reservoir::check_next_stream_length, this, stl_tools::force_move(targets)));
    });
}

void Reservoir::check_next_stream_length(std::vector<std::pair<SourceKey, StreamTypeUUID>> targets)
{
    no_except([&]{
        if (targets.empty())
        {
            m_executive->async(std::bind(&Reservoir::check_stream_length_one_by_one, this), time(nullptr) + 10);
            return;
        }

        auto source_key = targets.back().first;
        auto stream_type_uuid = targets.back().second;
        targets.pop_back();

        check_one_stream_length(source_key, stream_type_uuid);
        m_executive->async(std::bind(&Reservoir::check_next_stream_length, this, stl_tools::force_move(targets)));
    });
}

void Reservoir::check_one_stream_length(SourceKey source_key, StreamTypeUUID stream_type_uuid)
{
    try
    {
        Transaction t(*m_db);
        auto source = m_sources->get_source(source_key);
        auto stream_type = m_stream_types->get_stream_type(stream_type_uuid);
        {
            return;
        }

        auto remove_time = time(nullptr);
        switch (get_effective_stream_length(source, stream_type))
        {
        case StreamLength::FOREVER:
            remove_time = 0;
            break;

        case StreamLength::HOUR:
            remove_time -= ONE_HOUR;
            break;

        case StreamLength::DAY:
            remove_time -= ONE_DAY;
            break;

        case StreamLength::MONTH:
            remove_time -= ONE_MONTH;
            break;

        case StreamLength::YEAR:
            remove_time -= ONE_YEAR;
            break;

        case StreamLength::NONE:
            break;

        default:
            _ASSERT(false);
        }

        m_db->erase_query<StreamRecordData>(
            odb::query<StreamRecordData>::source_key.value == source_key &&
            odb::query<StreamRecordData>::stream_type_uuid.value == stream_type_uuid &&
            odb::query<StreamRecordData>::time < remove_time);

        t.commit();
    }
    catch (std::exception&)
    {
        // write to log
    }
}

void Reservoir::on_remove_stream_type(UUID source_type_uuid, Transaction& t)
{
    m_db->erase_query<StreamRecordData>(odb::query<StreamRecordData>::stream_type_uuid.value == source_type_uuid);
    m_db->erase_query<StreamInputData>(odb::query<StreamInputData>::stream_type_uuid.value == source_type_uuid);
}

void Reservoir::on_remove_sources(const std::vector<SourceKey>& removed_sources, Transaction& t)
{
    m_db->erase_query<StreamRecordData>(odb::query<StreamRecordData>::source_key.value.in_range(removed_sources.begin(), removed_sources.end()));
    m_db->erase_query<StreamInputData>(odb::query<StreamInputData>::source_key.value.in_range(removed_sources.begin(), removed_sources.end()));
}

}} //namespace TR { namespace Core {