#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "CommonProperty.h"
#include "RefValue.h"

template<class Policy>
SelectorProperty PolicyProperty(std::wstring name, std::wstring description, std::vector<std::wstring> policyNames, Policy defaultPolicy, Policy& policy)
{
    _ASSERT((int)defaultPolicy > 0 &&  (int)defaultPolicy <= int(policyNames.size()));

    std::vector<std::wstring> items;
    items.push_back((boost::wformat(L"Default (%1%)") % policyNames[(int)defaultPolicy - 1]).str());
    std::move(policyNames.begin(), policyNames.end(), std::back_inserter(items));
    
    return SelectorProperty(std::move(name), std::move(description), std::move(items), refValue(policy));
}

template<class Policy>
SelectorProperty PolicyProperty(std::wstring name, std::wstring description, std::pair<std::wstring, std::wstring> policyNames, bool firstIsDefault, Policy& policy)
{
    std::vector<std::wstring> items;
    items.push_back((boost::wformat(L"Default (%1%)") % (firstIsDefault? policyNames.first: policyNames.second)).str());
    items.push_back(std::move(policyNames.first));
    items.push_back(std::move(policyNames.second)); 
    
    return SelectorProperty(std::move(name), std::move(description), std::move(items), refValue(policy));
}

SelectorProperty ScheduleProperty(std::wstring name, std::wstring description, const std::vector<TR::ScheduleInfo>& schedulesInfo, TR::Key defaultSchedule, boost::optional<TR::Key>& scheduleKey);