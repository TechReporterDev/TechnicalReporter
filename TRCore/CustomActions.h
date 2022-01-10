#pragma once
#include "ServicesDecl.h"
namespace TR { namespace Core {
struct CustomActionData;

class CustomAction
{
public:
    friend class CustomActions;

    std::wstring                        get_name() const;
    ActionRef                           get_action_ref() const;

    const XmlRegularDoc&                get_params_doc() const;
    void                                set_params_doc(std::unique_ptr<XmlRegularDoc> params_doc);
    std::shared_ptr<const XmlDefDoc>    get_params_def() const;
    boost::optional<ReportTypeRef>      get_output_ref() const;

    CustomActionRef                     get_ref() const;
    operator CustomActionRef() const;

private:
    CustomAction(boost::optional<CustomActionRef> ref, std::wstring name, ActionRef action_ref,
        std::unique_ptr<XmlRegularDoc> params_doc, std::shared_ptr<const XmlDefDoc> params_def, 
        boost::optional<ReportTypeRef> output_ref);

    boost::optional<CustomActionRef> m_ref;
    std::wstring m_name;    
    ActionRef m_action_ref;
    std::shared_ptr<const XmlRegularDoc> m_params_doc;
    std::shared_ptr<const XmlDefDoc> m_params_def;
    boost::optional<ReportTypeRef> m_output_ref;
};

class CustomActions
{
public:
    static const Priority CUSTOM_ACTIONS_PRIORITY = 2000;

    CustomActions(Database& db, Basis& basis);

    void                                restore(Transaction& t);

    CustomAction                        create_custom_action(std::wstring name, ActionRef action_ref);
    void                                add_custom_action(CustomAction& custom_action, Transaction& t);
    CustomAction                        get_custom_action(UUID custom_action_uuid) const;
    void                                remove_custom_action(UUID custom_action_uuid, Transaction& t);

private:
    CustomActionData                    pack_custom_action(const CustomAction& custom_action) const;
    CustomAction                        unpack_custom_action(const CustomActionData& custom_action_data) const;

    void                                install_custom_action(const CustomAction& custom_action, Transaction& t);
    void                                uninstall_custom_action(UUID custom_action_uuid, Transaction& t);

    Database& m_db;
    ReportTypes& m_report_types;
    Actions& m_actions;
    ActionShortcuts& m_action_shortcuts;
};

}} // namespace TR { namespace Core {