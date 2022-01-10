#pragma once
#include "BackgndFwd.h"
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "ServicesValues.hxx"
#include "ScheduleData-odb.hxx"
namespace TR { namespace Core {

class ScheduledTaskDataset
{
public:
    ScheduledTaskDataset(Database& db);
    ~ScheduledTaskDataset();

    //single record
    ScheduledTaskData                   get(const SubjectIdentity& subject_identity, const PeriodicOperationVariant& operation) const;
    void                                put(const ScheduledTaskData& scheduled_task_data, Transaction& t);

    //multiple records
    std::vector<ScheduledTaskData>      find_schedule_equal(Key schedule_key) const;
    void                                erase_source_equal(const std::vector<SourceKey>& source_keys, Transaction& t);
    void                                erase_subject_equal(const SubjectIdentity& subject_identity, Transaction& t);
    void                                erase_operation_equal(const PeriodicOperationVariant& operation, Transaction& t);

private:
    class Cache;

    Database& m_db;
    std::unique_ptr<Cache> m_cache;
};

}} //namespace TR { namespace Core {