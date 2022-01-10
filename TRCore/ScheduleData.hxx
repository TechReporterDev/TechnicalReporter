#pragma once
#include <boost\optional.hpp>
#include "STLTools\uuid.h"
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "ServicesValues.hxx"
#include "AppValues.hxx"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)
	
struct ScheduleData
{
    ScheduleData():
        m_key(0),
        m_period(0),
        m_day(0),
        m_week_day(0),
        m_day_time(0),
        m_scheduled_time(0)
    {
    }

	ScheduleData(Key key, std::string name, int period, int	day, int week_day, time_t day_time, time_t scheduled_time):
        m_key(key),
        m_name(std::move(name)),
        m_period(period),
        m_day(day),
        m_week_day(week_day),
        m_day_time(day_time),
		m_scheduled_time(scheduled_time)
    {
    }

    Key					m_key;
    std::string			m_name;
    int					m_period;
    int					m_day;
    int					m_week_day;
    time_t				m_day_time;
	time_t				m_scheduled_time;
};

struct ScheduledTaskIdentity
{
    ScheduledTaskIdentity()
    {
    }

    ScheduledTaskIdentity(SubjectIdentity subject, PeriodicOperationVariant operation):
        m_subject(subject),
        m_operation(operation)
    {
    }

    SubjectIdentity				m_subject;
    PeriodicOperationVariant	m_operation;
};

inline bool operator < (const ScheduledTaskIdentity& left, const ScheduledTaskIdentity& right)
{
	if (left.m_subject < right.m_subject)
	{
		return true;
	}
	else if (right.m_subject < left.m_subject)
	{
		return false;
	}

	return right.m_operation < left.m_operation;
}

struct ScheduledTaskData
{
    ScheduledTaskData():
        m_launch_time(0),
        m_result(true)
    {
    }

	ScheduledTaskData(ScheduledTaskIdentity identity, boost::optional<Key> schedule_key = boost::none, time_t launch_time = 0, bool result = true, std::string message = ""):
        m_identity(identity),
        m_schedule_key(schedule_key),
        m_launch_time(launch_time),
        m_result(result),
        m_message(std::move(message))
    {
	}

	ScheduledTaskIdentity	m_identity;
    boost::optional<Key>	m_schedule_key;
    time_t					m_launch_time;
    bool					m_result;
    std::string				m_message;
};

#pragma db object(ScheduleData)
#pragma db member(ScheduleData::m_key) id auto

#pragma db value(ScheduledTaskIdentity)
#pragma db object(ScheduledTaskData)
#pragma db member(ScheduledTaskData::m_identity) id
#pragma warning(pop)

}} //namespace CI { namespace Core {