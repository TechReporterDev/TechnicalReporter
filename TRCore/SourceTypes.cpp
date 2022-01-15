#include "stdafx.h"
#include "SourceTypes.h"
#include "ReportTypes.h"
#include "Actions.h"
#include "Transaction.h"
#include "Content.h"
#include "RegistryRefs.h"
namespace TR { namespace Core {

StreamTransport::StreamTransport(StreamTypeRef stream_type_ref):
    m_stream_type_ref(stream_type_ref)
{
}

StreamTypeRef StreamTransport::get_stream_type() const
{
    return m_stream_type_ref;
}

PoolingTransport::PoolingTransport(StreamTypeRef stream_type_ref):
    StreamTransport(stream_type_ref)
{
}

SourceType::SourceType(SourceTypeUUID uuid, std::wstring name, std::unique_ptr<XmlPropertiesDef> config_def, UUID family_uuid):
    m_uuid(uuid),
    m_name(std::move(name)),
    m_config_def(std::move(config_def)),
    m_family_uuid(family_uuid)
{
}

SourceTypeUUID SourceType::get_uuid() const
{
    return m_uuid;
}

std::wstring SourceType::get_name() const
{
    return m_name;
}

std::shared_ptr<const XmlPropertiesDef> SourceType::get_config_def() const
{
    return m_config_def;
}

std::unique_ptr<XmlPropertiesDoc> SourceType::get_default_config() const
{
    return create_properties_doc(*m_config_def);
}

UUID SourceType::get_family() const
{
    return m_family_uuid;
}

SourceTypeRef SourceType::get_ref() const
{
    if (!m_ref)
    {
        throw Exception(L"Source type is not registred.");
    }
    return *m_ref;
}

SourceType::operator SourceTypeRef() const
{
    return get_ref();
}

const SourceTypeUUID GroupSourceType::s_uuid(stl_tools::gen_uuid(L"E82F3179-49DB-463C-83D1-A36D110C5A43"));
const std::shared_ptr<XmlPropertiesDef> GroupSourceType::s_config_def = XML::parse_properties_def(
    "<?xml version=\"1.0\"?>"
    "<settings style=\"COLLECTION\" caption=\"Group settings\" description=\"Group settings description\">"
    "</settings>");

GroupSourceType::GroupSourceType():
    SourceType(s_uuid, L"Group", xml_tools::clone_doc(*s_config_def), stl_tools::null_uuid())
{
}

std::vector<ReportTypeRef> GroupSourceType::get_downloads() const
{
    return {};
}

SourceType::DownloadFunctor GroupSourceType::prepare_download(ReportTypeRef, SourceRef) const
{
    _ASSERT(false);
    return nullptr;
}

std::vector<StreamTypeRef> GroupSourceType::get_streams() const
{
    return{};
}

const StreamTransport& GroupSourceType::get_transport(StreamTypeRef stream_type_ref) const
{
    _ASSERT(false);
    throw std::logic_error("Invalid stream type");
}

std::vector<ActionRef> GroupSourceType::get_actions() const
{
    return{};
}

std::vector<ReportTypeRef> GroupSourceType::get_reloads(ActionRef action_ref) const
{
    _ASSERT(false);
    return{};
}

SourceType::ActionFunctor GroupSourceType::prepare_action(ActionRef, SourceRef) const
{
    _ASSERT(false);
    return nullptr;
}

namespace {
using SourceTypeIndex = stl_tools::memfun_index_traits<SourceTypeUUID, SourceType, &SourceType::get_uuid, stl_tools::storage_index_cathegory::unique_index>;

struct DownloadIndex: stl_tools::multi_storage_index<ReportTypeUUID>
{
    template<class F>
    static void enum_keys(const std::unique_ptr<SourceType>& source_type, F f)
    {
        for (auto download_ref : source_type->get_downloads())
        {
            f(download_ref.get_uuid());
        }
    }
};

struct ActionIndex: stl_tools::multi_storage_index<ActionUUID>
{
    template<class F>
    static void enum_keys(const std::unique_ptr<SourceType>& source_type, F f)
    {
        for (auto action_ref : source_type->get_actions())
        {
            f(action_ref.get_uuid());
        }
    }
};
}// namespace {

class SourceTypeStorage: public stl_tools::storage<std::unique_ptr<SourceType>, SourceTypeIndex, DownloadIndex, ActionIndex>
{
};

SourceTypes::SourceTypes():
    m_db(nullptr),
    m_storage(std::make_unique<SourceTypeStorage>())
{   
}

SourceTypes::~SourceTypes()
{
}

void SourceTypes::set_database(Database* db)
{
    _ASSERT(!m_db);
    m_db = db;
}

void SourceTypes::set_report_types(ReportTypes* report_types)
{
    static_assert(SOURCE_TYPES_PRIORITY > ReportTypes::REPORT_TYPES_PRIORITY, "Wrong component priority");
    report_types->connect_remove_report_type([&](ReportTypeUUID report_type_uuid, Transaction& t){
        if (! m_storage->find_range<DownloadIndex>(report_type_uuid).empty())
        {
            throw Exception(L"Can`t remove ReportType referred from SourceType");
        }
    }, SOURCE_TYPES_PRIORITY);
}

void SourceTypes::set_actions(Actions* action)
{
    static_assert(SOURCE_TYPES_PRIORITY > Actions::ACTIONS_PRIORITY, "Wrong component priority");
    action->connect_remove_action([&](ActionUUID action_uuid, Transaction& t){
        if (! m_storage->find_range<ActionIndex>(action_uuid).empty())
        {
            throw Exception(L"Can`t remove Action referred from SourceType");
        }
    }, SOURCE_TYPES_PRIORITY);
}

const SourceType& SourceTypes::add_source_type(std::unique_ptr<SourceType> source_type)
{
    Transaction t(*m_db);
    auto& added_source_type = add_source_type(std::move(source_type), t);
    t.commit();
    return added_source_type;
}

const SourceType& SourceTypes::add_source_type(std::unique_ptr<SourceType> source_type, Transaction& t)
{
    _ASSERT(source_type && !source_type->m_ref);
    source_type->m_ref = SourceTypeRef(this, source_type->get_uuid());
    auto position = transact_insert(*m_storage, std::move(source_type), t);
    auto& added_source_type = **position;
    m_add_source_type_sig(added_source_type.get_ref(), t);
    return added_source_type;
}

void SourceTypes::remove_source_type(SourceTypeUUID uuid)
{
    Transaction t(*m_db);
    remove_source_type(uuid, t);
    t.commit();
}

void SourceTypes::remove_source_type(SourceTypeUUID uuid, Transaction& t)
{
    auto found = m_storage->find(uuid);
    if (found == m_storage->end())
    {
        throw Exception(L"Source type not found");
    }
    transact_erase(*m_storage, found, t);
    m_remove_source_type_sig(uuid, t);
}

bool SourceTypes::has_source_type(SourceTypeUUID uuid) const
{
    return m_storage->find(uuid) != m_storage->end();
}

const SourceType& SourceTypes::get_source_type(SourceTypeUUID uuid) const
{
    auto found = m_storage->find(uuid);
    if (found == m_storage->end())
    {
        throw Exception(L"Source type not found");
    }
    return **found;
}

SourceTypes::SourceTypeRange SourceTypes::get_source_types() const
{
    return *m_storage | boost::adaptors::indirected;
}

}} //namespace TR { namespace Core {