#pragma once
#include "BackgndFwd.h"
#include "BasisFwd.h"
#include "BasisRefs.h"
#include "stddecl.h"
namespace TR { namespace Core {
class ActionShortcutStorage;
class SourceRef;

class ActionShortcut
{
public:
    friend class ActionShortcuts;

    ActionShortcut(UUID uuid, std::wstring name, ActionRef action_ref, boost::optional<ReportTypeRef> output_ref);
    virtual ~ActionShortcut() = default;

    UUID                                    get_uuid() const;
    std::wstring                            get_name() const;
    
    ActionRef                               get_action_ref() const;
    boost::optional<ReportTypeRef>          get_output_ref() const;
    
    ActionShortcutRef                       get_ref() const;
    operator ActionShortcutRef() const;

    // override
    virtual std::unique_ptr<XmlDoc>         get_action_params(SourceRef source) const = 0;

private:
    UUID m_uuid;
    std::wstring m_name;
    ActionRef m_action_ref; 
    boost::optional<ReportTypeRef> m_output_ref;
    boost::optional<ActionShortcutRef> m_ref;
};

class ActionShortcuts
{
public:
    static const Priority ACTION_SHORTCUTS_PRIORITY = 175;
    using ActionShortcutRange = boost::any_range<const ActionShortcut, boost::forward_traversal_tag>;

    ActionShortcuts();
    ~ActionShortcuts();
    void                                    set_report_types(ReportTypes* report_types);
    void                                    set_actions(Actions* actions);

    const ActionShortcut&                   add_shortcut(std::unique_ptr<ActionShortcut> action_shortcut, Transaction& t);
    void                                    remove_shortcut(UUID uuid, Transaction& t);
    bool                                    has_shortcut(UUID uuid) const;
    const ActionShortcut&                   get_shortcut(UUID uuid) const;
    ActionShortcutRange                     get_shortcuts() const;
    ActionShortcutRange                     find_shortcuts(ActionRef action_ref) const;

    template<class T>
    void connect_add_shortcut(T slot, Priority priority) { m_add_shortcut_sig.connect(slot, priority); }

    template<class T>
    void connect_remove_shortcut(T slot, Priority priority) { m_remove_shortcut_sig.connect(slot, priority); }

private:
    std::unique_ptr<ActionShortcutStorage> m_storage;

    OrderedSignal<void(const ActionShortcut& action_shortcut, Transaction& t)> m_add_shortcut_sig;
    OrderedSignal<void(UUID action_shortcut_uuid, Transaction& t)> m_remove_shortcut_sig;
};

}}// namespace TR { namespace Core { 