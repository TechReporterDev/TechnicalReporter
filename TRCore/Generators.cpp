#include "stdafx.h"
#include "Generators.h"
#include "Transaction.h"
#include "SourceTypes.h"
#include "ReportTypes.h"
namespace TR { namespace Core {

namespace {
struct OperationIndex: stl_tools::unique_storage_index<std::pair<SourceTypeRef, ReportTypeRef>>
{
    static std::pair<SourceTypeRef, ReportTypeRef> get_key(const std::shared_ptr<Generator>& generator)
    {
        return std::make_pair(generator->get_source_type(), generator->get_report_type());
    }
};

struct SourceTypeIndex : stl_tools::single_storage_index<SourceTypeUUID>
{
    static SourceTypeUUID get_key(const std::shared_ptr<Generator>& generator)
    {
        return generator->get_source_type().get_uuid();
    }
};

struct ReportTypeIndex: stl_tools::single_storage_index<ReportTypeUUID>
{
    static ReportTypeUUID get_key(const std::shared_ptr<Generator>& generator)
    {
        return generator->get_report_type().get_uuid();
    }
};

struct IdentityIndex : stl_tools::unique_storage_index<UUID>
{
    static UUID get_key(const std::shared_ptr<Generator>& generator)
    {
        return generator->get_uuid();
    }
};

} //namespace {

class Generators::Storage: public stl_tools::storage<std::shared_ptr<Generator>, IdentityIndex, OperationIndex, SourceTypeIndex, ReportTypeIndex>
{
};

Generators::Generators():
    m_source_types(nullptr),
    m_report_types(nullptr),
    m_storage(std::make_unique<Storage>())
{
}

Generators::~Generators()
{
}

void Generators::set_source_types(SourceTypes* source_types)
{
    m_source_types = source_types;
    m_source_types->connect_remove_source_type([this](UUID source_type_uuid, Transaction& t) {
        if (!m_storage->find_range<SourceTypeIndex>(SourceTypeUUID(source_type_uuid)).empty())
        {
            throw Exception(L"Can`t remove 'SourceType' until generator exists");
        }
    }, GENERATORS_PRIORITY);
}

void Generators::set_report_types(ReportTypes* report_types)
{
    m_report_types = report_types;
    m_report_types->connect_remove_report_type([this](ReportTypeUUID report_type_uuid, Transaction& t){
        if (!m_storage->find_range<ReportTypeIndex>(report_type_uuid).empty())
        {
            throw Exception(L"Can`t remove 'ReportType' until generator exists");
        }       
    }, GENERATORS_PRIORITY);
}

void Generators::add_generator(std::shared_ptr<Generator> generator, Transaction& t)
{
    auto generator_uuid = generator->get_uuid();

    if (m_storage->find<OperationIndex>(std::make_pair(generator->get_source_type(), generator->get_report_type())) != m_storage->end() ||
        m_storage->find(generator_uuid) != m_storage->end())
    {
        throw std::logic_error("Generator already exists.");
    }
    transact_insert(*m_storage, generator, t);
    m_add_generator_sig(generator_uuid, t);
}

void Generators::remove_generator(const UUID& uuid, Transaction& t)
{
    auto it = m_storage->find(uuid);
    if (it == m_storage->end())
    {
        throw std::logic_error("Generator not found.");
    }
    transact_erase(*m_storage, it, t);
    m_remove_generator_sig(uuid, t);
}

std::shared_ptr<Generator> Generators::get_generator(const UUID& uuid) const
{
    auto it = m_storage->find(uuid);
    if (it == m_storage->end())
    {
        throw std::logic_error("Generator not found.");
    }
    return *it;
}

std::vector<std::shared_ptr<Generator>> Generators::find_by_source_type(SourceTypeRef source_type_ref) const
{
    auto range = m_storage->find_range<SourceTypeIndex>(source_type_ref.get_uuid());
    return { range.begin(), range.end() };
}

std::vector<std::shared_ptr<Generator>> Generators::find_by_output(ReportTypeRef report_type_ref) const
{
    auto range = m_storage->find_range<ReportTypeIndex>(report_type_ref.get_uuid());
    return { range.begin(), range.end() };
}

}} //namespace TR { namespace Core {