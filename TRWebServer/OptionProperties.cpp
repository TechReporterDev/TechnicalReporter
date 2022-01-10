#include "stdafx.h"
#include "OptionProperties.h"
#include "CustomValue.h"
#include "Application.h"

SelectorProperty ScheduleProperty(std::wstring name, std::wstring description, const std::vector<TR::ScheduleInfo>& schedulesInfo, TR::Key defaultSchedule, boost::optional<TR::Key>& scheduleKey)
{
    std::vector<std::wstring> scheduleNames;        
    auto defaultPosition = boost::find(schedulesInfo | stl_tools::members(&TR::ScheduleInfo::m_schedule_key), defaultSchedule).base();
    if (defaultPosition == schedulesInfo.end())
    {
        throw std::logic_error("Schedule not found");       
    }
    scheduleNames.push_back((boost::wformat(L"Default (%1%)") % defaultPosition->m_name).str());
    boost::copy(schedulesInfo | stl_tools::members(&TR::ScheduleInfo::m_name), std::back_inserter(scheduleNames));

    auto getSchedule = [&schedulesInfo, &scheduleKey]{ 
        if (!scheduleKey)
        {
            return 0;
        }

        auto found = boost::find(schedulesInfo | stl_tools::members(&TR::ScheduleInfo::m_schedule_key), *scheduleKey);
        if(found.base() == schedulesInfo.end())
        {
            throw std::logic_error("Schedule not found");
        }

        return int(found.base() - schedulesInfo.begin() + 1);
    };

    auto setSchedule = [&schedulesInfo, &scheduleKey](int value){ 
        scheduleKey = value? boost::make_optional(schedulesInfo[value - 1].m_schedule_key): boost::none;
    };

    return SelectorProperty(std::move(name), std::move(description), std::move(scheduleNames), customValue<int>(
        getSchedule,
        setSchedule)
    );
}
