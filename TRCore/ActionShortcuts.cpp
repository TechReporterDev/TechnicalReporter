#include "stdafx.h"
#include "ActionShortcuts.h"
#include "ReportTypes.h"
#include "Actions.h"
#include "Transaction.h"
namespace TR { namespace Core {

ActionShortcut::ActionShortcut(UUID uuid, std::wstring name, ActionRef action_ref, boost::optional<ReportTypeRef> output_ref):
    m_uuid(uuid),
    m_name(std::move(name)),
    m_action_ref(action_ref),
    m_output_ref(output_ref)
{
}

UUID ActionShortcut::get_uuid() const
{
    return m_uuid;
}

std::wstring ActionShortcut::get_name() const
{
    return m_name;
}

ActionRef ActionShortcut::get_action_ref() const
{
    return m_action_ref;
}

boost::optional<ReportTypeRef> ActionShortcut::get_output_ref() const
{
    return m_output_ref;
}

ActionShortcutRef ActionShortcut::get_ref() const
{
    if (!m_ref)
    {
        throw Exception(L"ActionShortcut is not registred.");
    }
    return *m_ref;
}

ActionShortcut::operator ActionShortcutRef() const
{
    return get_ref();
}

namespace {
using ActionShortcutIndex = stl_tools::memfun_index_traits<UUID, ActionShortcut, &ActionShortcut::get_uuid, stl_tools::storage_index_cathegory::unique_index>;

struct HostActionIndex: stl_tools::single_storage_index<ActionUUID>
{
    static ActionUUID get_key(const std::unique_ptr<ActionShortcut>& action_shortcut)
    {
        return action_shortcut->get_action_ref().get_uuid();
    }
};

struct OutputIndex: stl_tools::single_storage_index<ReportTypeUUID, stl_tools::storage_index_optional::true_type>
{
    static boost::optional<ReportTypeUUID> get_key(const std::unique_ptr<ActionShortcut>& action_shortcut)
    {
        if (auto output_ref = action_shortcut->get_output_ref())
        {
            return output_ref->get_uuid();
        }
        return boost::none;
    }
};
} //namespace {

class ActionShortcutStorage: public stl_tools::storage<std::unique_ptr<ActionShortcut>, 
    ActionShortcutIndex, HostActionIndex, OutputIndex>
{
};

ActionShortcuts::ActionShortcuts():
    m_storage(std::make_unique<ActionShortcutStorage>())
{
}

ActionShortcuts::~ActionShortcuts()
{
}

void ActionShortcuts::set_report_types(ReportTypes* report_types)
{
    static_assert(ACTION_SHORTCUTS_PRIORITY > ReportTypes::REPORT_TYPES_PRIORITY, "Wrong component priority");
    report_types->connect_remove_report_type([&](ReportTypeUUID report_type_uuid, Transaction& t){
        if (! m_storage->find_range<OutputIndex>(report_type_uuid).empty())
        {
            throw Exception(L"Can`t remove ReportType referred from ActionShortcut");
        }
    }, ACTION_SHORTCUTS_PRIORITY);
}

void ActionShortcuts::set_actions(Actions* actions)
{
    static_assert(ACTION_SHORTCUTS_PRIORITY > Actions::ACTIONS_PRIORITY, "Wrong component priority");
    actions->connect_remove_action([&](ActionUUID action_uuid, Transaction& t){
        if (! m_storage->find_range<HostActionIndex>(action_uuid).empty())
        {
            throw Exception(L"Can`t remove Action referred from ActionShortcut");
        }

    }, ACTION_SHORTCUTS_PRIORITY);
}

const ActionShortcut& ActionShortcuts::add_shortcut(std::unique_ptr<ActionShortcut> action_shortcut, Transaction& t)
{
    _ASSERT(action_shortcut && !action_shortcut->m_ref);
    action_shortcut->m_ref = ActionShortcutRef(this, action_shortcut->get_uuid());
    auto position = transact_insert(*m_storage, std::move(action_shortcut), t);
    m_add_shortcut_sig(**position, t);  
    return **position;
}

void ActionShortcuts::remove_shortcut(UUID uuid, Transaction& t)
{
    auto position = m_storage->find(uuid);
    if (position == m_storage->end())
    {
        throw Exception(L"Action not found");
    }
    transact_erase(*m_storage, position, t);
    m_remove_shortcut_sig(uuid, t);
}

bool ActionShortcuts::has_shortcut(UUID uuid) const
{
    return m_storage->find(uuid) != m_storage->end();
}

const ActionShortcut& ActionShortcuts::get_shortcut(UUID uuid) const
{
    auto position = m_storage->find(uuid);
    if (position == m_storage->end())
    {
        throw Exception(L"ActionShortcut not found");
    }
    return **position;
}

ActionShortcuts::ActionShortcutRange ActionShortcuts::get_shortcuts() const
{
    return *m_storage | boost::adaptors::indirected;
}

ActionShortcuts::ActionShortcutRange ActionShortcuts::find_shortcuts(ActionRef action_ref) const
{
    return m_storage->find_range<HostActionIndex>(action_ref.get_uuid()) | boost::adaptors::indirected;
}

}} // namespace TR { namespace Core {