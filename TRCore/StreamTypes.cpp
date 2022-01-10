#include "stdafx.h"
#include "StreamTypes.h"
#include "Transaction.h"
namespace TR {namespace Core {

StreamType::StreamType(StreamTypeUUID uuid, std::wstring name, std::unique_ptr<XmlDefDoc> message_def, int version):
    m_uuid(uuid),
    m_name(std::move(name)), 
    m_message_def(std::move(message_def)),
    m_version(version)
{
}

StreamTypeUUID StreamType::get_uuid() const
{
    return m_uuid;
}

std::wstring StreamType::get_name() const
{
    return m_name;
}

std::shared_ptr<const XmlDefDoc> StreamType::get_message_def() const
{
    return m_message_def;
}

int StreamType::get_version() const
{
    return m_version;
}

StreamTypeRef StreamType::get_ref() const
{
    if (!m_ref)
    {
        throw Exception(L"ReportType is not registred.");
    }
    return *m_ref;
}

StreamType::operator StreamTypeRef() const
{
    return get_ref();
}

struct StreamTypeUUIDIndex: stl_tools::unique_storage_index<StreamTypeUUID>
{
    static StreamTypeUUID get_key(const std::unique_ptr<StreamType>& stream_type)
    {
        return stream_type->get_uuid();
    }
};

class StreamTypesStorage: public stl_tools::storage<std::unique_ptr<StreamType>, StreamTypeUUIDIndex>
{
};

StreamTypes::StreamTypes():
    m_db(nullptr),
    m_storage(std::make_unique<StreamTypesStorage>())
{
}

StreamTypes::~StreamTypes()
{
}

void StreamTypes::set_database(Database* db)
{
    m_db = db;
}

const StreamType& StreamTypes::add_stream_type(std::unique_ptr<StreamType> stream_type, Transaction& t)
{
    auto uuid = stream_type->get_uuid();
    auto position = transact_insert(*m_storage, std::move(stream_type), t);
    auto& added_stream_type = **position;
    added_stream_type.m_ref = StreamTypeRef(this, uuid);
    m_add_stream_type_sig(added_stream_type.get_ref(), t);
    return added_stream_type;
}

void StreamTypes::remove_stream_type(StreamTypeUUID uuid, Transaction& t)
{
    auto found = m_storage->find(uuid);
    if (found == m_storage->end())
    {
        throw Exception(L"Stream type not found");
    }
    transact_erase(*m_storage, found, t);
    m_remove_stream_type_sig(uuid, t);
}

bool StreamTypes::has_stream_type(StreamTypeUUID uuid) const
{
    return m_storage->find(uuid) != m_storage->end();
}

const StreamType& StreamTypes::get_stream_type(StreamTypeUUID uuid) const
{
    auto found = m_storage->find(uuid);
    if (found == m_storage->end())
    {
        throw Exception(L"Stream type not found");
    }
    return **found;
}

StreamTypes::StreamTypeRange StreamTypes::get_stream_types() const
{
    return *m_storage | boost::adaptors::indirected;
}

}}//namespace TR {namespace Core {
