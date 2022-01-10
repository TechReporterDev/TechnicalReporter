#include "stdafx.h"
#include "ActionDelegates.h"
#include "ReportTypes.h"
#include "Actions.h"
#include "Transaction.h"
namespace TR { namespace Core {

bool operator < (const ActionDependency& left, const ActionDependency& right)
{
    if (left.m_host_action_refs < right.m_host_action_refs)
    {
        return true;
    }

    if (left.m_host_action_refs > right.m_host_action_refs)
    {
        return false;
    }

    return left.m_guest_action_ref < right.m_guest_action_ref;
}

ActionDelegate::ActionDelegate(std::vector<ReportTypeRef> input_refs, std::vector<ActionRef> host_action_refs, ActionRef guest_action_ref):
    m_input_refs(std::move(input_refs)),
    m_host_action_refs(std::move(host_action_refs)),
    m_guest_action_ref(guest_action_ref)
{
}

ActionDependency ActionDelegate::get_action_dependency() const
{
    return ActionDependency(m_host_action_refs, m_guest_action_ref);
}

std::vector<ReportTypeRef> ActionDelegate::get_input_refs() const
{
    return m_input_refs;
}

std::vector<ActionRef> ActionDelegate::get_host_action_refs() const
{
    return m_host_action_refs;
}

ActionRef ActionDelegate::get_guest_action_ref() const
{
    return m_guest_action_ref;
}

namespace {
using ActionDelegateIndex = stl_tools::memfun_index_traits<ActionDependency, ActionDelegate, &ActionDelegate::get_action_dependency, stl_tools::storage_index_cathegory::unique_index>;

struct InputIndex: stl_tools::multi_storage_index<ReportTypeUUID>
{
    template<class F>
    static void enum_keys(const std::unique_ptr<ActionDelegate>& action_delegate, F f)
    {
        for (auto input_ref : action_delegate->get_input_refs())
        {
            f(input_ref.get_uuid());
        }
    }
};

struct HostActionIndex: stl_tools::multi_storage_index<ActionUUID>
{
    template<class F>
    static void enum_keys(const std::unique_ptr<ActionDelegate>& action_delegate, F f)
    {
        for (auto action_ref : action_delegate->get_host_action_refs())
        {
            f(action_ref.get_uuid());
        }
    }
};

struct GuestActionIndex: stl_tools::single_storage_index<ActionUUID>
{
    static ActionUUID get_key(const std::unique_ptr<ActionDelegate>& action_delegate)
    {
        return action_delegate->get_guest_action_ref().get_uuid();
    }
};

} //namespace {

class ActionDelegateStorage: public stl_tools::storage<std::unique_ptr<ActionDelegate>, 
    ActionDelegateIndex, InputIndex, HostActionIndex, GuestActionIndex>
{
};

ActionDelegates::ActionDelegates():
    m_storage(std::make_unique<ActionDelegateStorage>())
{
}

ActionDelegates::~ActionDelegates()
{
}

void ActionDelegates::set_report_types(ReportTypes* report_types)
{
    static_assert(ACTION_DELEGATES_PRIORITY > ReportTypes::REPORT_TYPES_PRIORITY, "Wrong component priority");
    report_types->connect_remove_report_type([&](ReportTypeUUID report_type_uuid, Transaction& t){
        if (! m_storage->find_range<InputIndex>(report_type_uuid).empty())
        {
            throw Exception(L"Can`t remove ReportType referred from ActionDelegate");
        }
    }, ACTION_DELEGATES_PRIORITY);
}

void ActionDelegates::set_actions(Actions* actions)
{
    static_assert(ACTION_DELEGATES_PRIORITY > Actions::ACTIONS_PRIORITY, "Wrong component priority");
    actions->connect_remove_action([&](ActionUUID action_uuid, Transaction& t){
        if (! m_storage->find_range<HostActionIndex>(action_uuid).empty())
        {
            throw Exception(L"Can`t remove Action referred from ActionDelegate");
        }

        if (! m_storage->find_range<GuestActionIndex>(action_uuid).empty())
        {
            throw Exception(L"Can`t remove Action referred from ActionDelegate");
        }
    }, ACTION_DELEGATES_PRIORITY);
}

const ActionDelegate& ActionDelegates::add_action_delegate(std::unique_ptr<ActionDelegate> action_delegate, Transaction& t)
{
    _ASSERT(action_delegate);
    if (m_storage->find(action_delegate->get_action_dependency()) != m_storage->end())
    {
        throw Exception(L"Action delegate duplicate found");
    }

    auto position = transact_insert(*m_storage, std::move(action_delegate), t);
    m_add_delegate_sig(**position, t);
    return **position;
}

void ActionDelegates::remove_action_delegate(ActionDependency action_dependency, Transaction& t)
{
    auto position = m_storage->find(action_dependency);
    if (position == m_storage->end())
    {
        throw Exception(L"Action delegate not found");
    }

    transact_erase(*m_storage, position, t);
    m_remove_delegate_sig(action_dependency, t);
}

const ActionDelegate& ActionDelegates::get_action_delegate(ActionDependency action_dependency) const
{
    auto position = m_storage->find(action_dependency);
    if (position == m_storage->end())
    {
        throw Exception(L"ActionDelegate not found");
    }
    return **position;
}

ActionDelegates::ActionDelegateRange ActionDelegates::find_by_host_action(ActionRef host_action_ref) const
{
    return m_storage->find_range<HostActionIndex>(host_action_ref.get_uuid()) | boost::adaptors::indirected;
}

ActionDelegates::ActionDelegateRange ActionDelegates::find_by_input(ReportTypeRef input_ref) const
{
    return m_storage->find_range<InputIndex>(input_ref.get_uuid()) | boost::adaptors::indirected;
}

}} // namespace TR { namespace Core {