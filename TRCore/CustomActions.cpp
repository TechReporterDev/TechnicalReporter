#include "stdafx.h"
#include "CustomActions.h"
#include "Basis.h"
#include "Transaction.h"
#include "BasisPacking.h"
#include "ServicesPacking.h"
#include "CustomActionData-odb.hxx"
namespace TR { namespace Core {

class CustomActionShortcut: public ActionShortcut
{
public:
    CustomActionShortcut(const CustomAction& custom_action):
        ActionShortcut(custom_action.get_ref().get_uuid(), custom_action.get_name(), custom_action.get_action_ref(), custom_action.get_output_ref()),
        m_params(xml_tools::clone_doc(custom_action.get_params_doc()))
    {
    }

    virtual std::unique_ptr<XmlDoc> get_action_params(SourceRef source) const override
    {
        return xml_tools::clone_doc(*m_params);
    }

private:
    std::unique_ptr<XmlDoc> m_params;
};

std::wstring CustomAction::get_name() const
{
    return m_name;
}

ActionRef CustomAction::get_action_ref() const
{
    return m_action_ref;
}

const XmlRegularDoc& CustomAction::get_params_doc() const
{
    return *m_params_doc;
}

void CustomAction::set_params_doc(std::unique_ptr<XmlRegularDoc> params_doc)
{
    m_params_doc = std::move(params_doc);
}

std::shared_ptr<const XmlDefDoc> CustomAction::get_params_def() const
{
    return m_params_def;
}

boost::optional<ReportTypeRef> CustomAction::get_output_ref() const
{
    return m_output_ref;
}

CustomActionRef CustomAction::get_ref() const
{
    _ASSERT(m_ref);
    if (!m_ref)
    {
        throw Exception(L"Custom action is not persist");
    }
    return *m_ref;
}

CustomAction::operator CustomActionRef() const
{
    return get_ref();
}

CustomAction::CustomAction(boost::optional<CustomActionRef> ref, std::wstring name, ActionRef action_ref,
    std::unique_ptr<XmlRegularDoc> params_doc, std::shared_ptr<const XmlDefDoc> params_def, boost::optional<ReportTypeRef> output_ref):
    m_ref(ref),
    m_name(std::move(name)),
    m_action_ref(action_ref),
    m_params_doc(std::move(params_doc)),
    m_params_def(params_def),
    m_output_ref(output_ref)
{
}

CustomActions::CustomActions(Database& db, Basis& basis):
    m_db(db),
    m_report_types(basis.m_report_types),
    m_actions(basis.m_actions),
    m_action_shortcuts(basis.m_action_shortcuts)
{
    m_actions.connect_add_action([&](ActionRef action_ref, Transaction& t){
        for (auto& custom_action_data : m_db.query<CustomActionData>(odb::query<CustomActionData>::action_uuid.value == action_ref.get_uuid()))
        {
            install_custom_action(unpack_custom_action(custom_action_data), t);
        }
    }, CUSTOM_ACTIONS_PRIORITY);
}

void CustomActions::restore(Transaction& t)
{   
}

CustomAction CustomActions::create_custom_action(std::wstring name, ActionRef action_ref)
{
    auto& action = *action_ref;
    return CustomAction(
        boost::none, 
        std::move(name), 
        action_ref, 
        action.get_default_params(), 
        action.get_params_def(), 
        boost::none
    );
}

void CustomActions::add_custom_action(CustomAction& custom_action, Transaction& t)
{ 
    _ASSERT(!custom_action.m_ref);

    auto& action = *custom_action.m_action_ref; 
    if (auto output = action.make_report_type(stl_tools::gen_uuid(), custom_action.get_name(), custom_action.get_params_doc()))
    {
        custom_action.m_output_ref = m_report_types.add_report_type(std::move(output), t);
    }

    custom_action.m_ref = CustomActionRef(this, stl_tools::gen_uuid());
    m_action_shortcuts.add_shortcut(std::make_unique<CustomActionShortcut>(custom_action), t);
    
    m_db.persist<CustomActionData>(pack_custom_action(custom_action));
}

CustomAction CustomActions::get_custom_action(UUID custom_action_uuid) const
{
    ReadOnlyTransaction t(m_db);
    CustomActionData custom_action_data;
    m_db.load<CustomActionData>(custom_action_uuid, custom_action_data);
    return unpack_custom_action(custom_action_data);
}

void CustomActions::remove_custom_action(UUID custom_action_uuid, Transaction& t)
{
    uninstall_custom_action(custom_action_uuid, t);
    m_db.erase<CustomActionData>(custom_action_uuid);
}

CustomActionData CustomActions::pack_custom_action(const CustomAction& custom_action) const
{
    return CustomActionData(
        pack(custom_action.m_name),
        custom_action.get_ref().m_uuid,
        pack(custom_action.m_action_ref),
        xml_tools::as_string(*custom_action.m_params_doc),
        xml_tools::as_string(*custom_action.m_params_def),
        pack(custom_action.m_output_ref)
    );
}

CustomAction CustomActions::unpack_custom_action(const CustomActionData& custom_action_data) const
{
    auto params_def = XML::parse_definition(custom_action_data.m_params_def);
    auto params_doc = XML::parse_regular(custom_action_data.m_params_doc, *params_def);

    return CustomAction(
        CustomActionRef(this, custom_action_data.m_uuid),
        unpack(custom_action_data.m_name),
        unpack(custom_action_data.m_action_uuid, &m_actions),
        std::move(params_doc),
        std::move(params_def),
        unpack(custom_action_data.m_output_uuid, &m_report_types)
    );
}

void CustomActions::install_custom_action(const CustomAction& custom_action, Transaction& t)
{
    auto& action = *custom_action.m_action_ref;
    if (custom_action.m_output_ref)
    {
        m_report_types.add_report_type(action.make_report_type(custom_action.m_output_ref->get_uuid(), custom_action.get_name(), custom_action.get_params_doc()), t);
    }

    m_action_shortcuts.add_shortcut(std::make_unique<CustomActionShortcut>(custom_action), t);
}

void CustomActions::uninstall_custom_action(UUID custom_action_uuid, Transaction& t)
{
    CustomActionData custom_action_data;
    m_db.load<CustomActionData>(custom_action_uuid, custom_action_data);

    m_action_shortcuts.remove_shortcut(custom_action_data.m_uuid, t);
    if (custom_action_data.m_output_uuid)
    {
        m_report_types.remove_report_type(*custom_action_data.m_output_uuid, t);
    }
}

}} //namespace TR { namespace Core {