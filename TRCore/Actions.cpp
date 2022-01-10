#include "stdafx.h"
#include "Actions.h"
#include "ReportTypes.h"
#include "Transaction.h"
namespace TR { namespace Core {

Action::Action(ActionUUID uuid, std::wstring name, std::unique_ptr<XmlPropertiesDef> params_def):
    m_uuid(uuid),
    m_name(std::move(name)),
    m_params_def(std::move(params_def))
{
}

Actions::~Actions()
{
}

ActionUUID Action::get_uuid() const
{
    return m_uuid;
}

std::wstring Action::get_name() const
{
    return m_name;
}

std::shared_ptr<const XmlPropertiesDef> Action::get_params_def() const
{
    return m_params_def;
}

std::unique_ptr<XmlPropertiesDoc> Action::get_default_params() const
{
    if (!m_params_def)
    {
        return nullptr;
    }

    return create_properties_doc(*m_params_def);
}

ActionRef Action::get_ref() const
{
    if (!m_ref)
    {
        throw Exception(L"Action is not registred.");
    }
    return *m_ref;
}

Action::operator ActionRef() const
{
    return get_ref();
}

Action::MakeContent Action::prepare_make_content() const
{
    return nullptr;
}

std::unique_ptr<ReportType> Action::make_report_type(UUID uuid, std::wstring name, const XmlDoc& params) const
{
    return nullptr;
}

namespace {
using ActionIndex = stl_tools::memfun_index_traits<ActionUUID, Action, &Action::get_uuid, stl_tools::storage_index_cathegory::unique_index>;
} // namespace {

class ActionStorage: public stl_tools::storage<std::unique_ptr<Action>, ActionIndex>
{
};

Actions::Actions():
    m_storage(std::make_unique<ActionStorage>())
{
}

const Action& Actions::add_action(std::unique_ptr<Action> action, Transaction& t)
{
    _ASSERT(action && !action->m_ref);
    action->m_ref = ActionRef(this, action->get_uuid());
    auto position = transact_insert(*m_storage, std::move(action), t);
    m_add_action_sig(**position, t);
    return **position;
}

void Actions::remove_action(ActionUUID action_uuid, Transaction& t)
{
    auto position = m_storage->find(action_uuid);
    if (position == m_storage->end())
    {
        throw Exception(L"Action not found");
    }
    transact_erase(*m_storage, position, t);
    m_remove_action_sig(action_uuid, t);
}

bool Actions::has_action(ActionUUID uuid) const
{
    return m_storage->find(uuid) != m_storage->end();
}

const Action& Actions::get_action(ActionUUID uuid) const
{
    auto position = m_storage->find(uuid);
    if (position == m_storage->end())
    {
        throw Exception(L"Action not found");
    }
    return **position;
}

Actions::ActionRange Actions::get_actions() const
{
    return *m_storage | boost::adaptors::indirected;
}

}} //namespace TR { namespace Core {