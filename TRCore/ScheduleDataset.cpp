#include "stdafx.h"
#include "ScheduleDataset.h"
#include "Transaction.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
namespace TR {  namespace Core {

namespace {
struct IdentityIndex: stl_tools::unique_storage_index<ScheduledTaskIdentity>
{
    static key_type get_key(const ScheduledTaskData& scheduled_task_data)
    {
        return scheduled_task_data.m_identity;
    }
};

struct SubjectIndex: stl_tools::single_storage_index<SubjectIdentity>
{
    static key_type get_key(const ScheduledTaskData& scheduled_task_data)
    {
        return scheduled_task_data.m_identity.m_subject;
    }
};

struct OperationIndex: stl_tools::single_storage_index<PeriodicOperationVariant>
{
    static key_type get_key(const ScheduledTaskData& scheduled_task_data)
    {
        return scheduled_task_data.m_identity.m_operation;
    }
};

struct ScheduleIndex: stl_tools::single_storage_index<boost::optional<Key>>
{
    static key_type get_key(const ScheduledTaskData& scheduled_task_data)
    {
        return scheduled_task_data.m_schedule_key;
    }
};

} //namespace {


class ScheduledTaskDataset::Cache: public stl_tools::storage<ScheduledTaskData, IdentityIndex, SubjectIndex, OperationIndex, ScheduleIndex>
{
};

ScheduledTaskDataset::ScheduledTaskDataset(Database& db):
    m_db(db),
    m_cache(std::make_unique<Cache>())
{
    ReadOnlyTransaction t(m_db);
    for (auto& scheduled_task_data : m_db.query<ScheduledTaskData>())
    {
        m_cache->insert(scheduled_task_data);
    }
}

ScheduledTaskDataset::~ScheduledTaskDataset()
{
}

ScheduledTaskData ScheduledTaskDataset::get(const SubjectIdentity& subject_identity, const PeriodicOperationVariant& operation) const
{
    auto position = m_cache->find({subject_identity, operation});
    if (position != m_cache->end())
    {
        return *position;
    }
    return ScheduledTaskData({subject_identity, operation});
}

void ScheduledTaskDataset::put(const ScheduledTaskData& scheduled_task_data, Transaction& t)
{
    auto position = m_cache->find(scheduled_task_data.m_identity);
    if (position != m_cache->end())
    {
        m_db.update(scheduled_task_data);
        transact_update(*m_cache, scheduled_task_data, t);
    }
    else
    {
        m_db.persist(scheduled_task_data);
        transact_insert(*m_cache, scheduled_task_data, t);
    }
}

std::vector<ScheduledTaskData> ScheduledTaskDataset::find_schedule_equal(Key schedule_key) const
{
    auto range = m_cache->find_range<ScheduleIndex>(schedule_key);
    return {range.begin(), range.end()};
}

void ScheduledTaskDataset::erase_source_equal(const std::vector<SourceKey>& source_keys, Transaction& t)
{
    for (auto source_key : source_keys)
    {
        transact_erase_range(*m_cache, m_cache->find_range<SubjectIndex>(source_key), t);
    }
    m_db.erase_query<ScheduledTaskData>(odb::query<ScheduledTaskData>::identity.subject.value1.value.in_range(source_keys.begin(), source_keys.end()));
}

void ScheduledTaskDataset::erase_subject_equal(const SubjectIdentity& subject_identity, Transaction& t)
{
    transact_erase_range(*m_cache, m_cache->find_range<SubjectIndex>(subject_identity), t);
    m_db.erase_query<ScheduledTaskData>(
        odb::query<ScheduledTaskData>::identity.subject.value1.value == subject_identity.m_value1 &&
        odb::query<ScheduledTaskData>::identity.subject.value2.value == subject_identity.m_value2);
}

void ScheduledTaskDataset::erase_operation_equal(const PeriodicOperationVariant& operation, Transaction& t)
{
    transact_erase_range(*m_cache, m_cache->find_range<OperationIndex>(operation), t);
    m_db.erase_query<ScheduledTaskData>(
        odb::query<ScheduledTaskData>::identity.operation.value1.report_type_uuid.value == operation.m_value1.m_report_type_uuid &&
        odb::query<ScheduledTaskData>::identity.operation.value2.action_shortcut_uuid == operation.m_value2.m_action_shortcut_uuid);
}

}} //namespace TR { namespace Core {