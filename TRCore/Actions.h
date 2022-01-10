#pragma once
#include "BackgndFwd.h"
#include "BasisFwd.h"
#include "BasisRefs.h"
#include "stddecl.h"
namespace TR { namespace Core {
class ActionStorage;

class Action
{
public:
    friend class Actions;
    using MakeContent = std::function<std::unique_ptr<Content>(Blob blob, const XmlDoc& params)>;

    Action(ActionUUID uuid, std::wstring name, std::unique_ptr<XmlPropertiesDef> params_def);
    virtual ~Action() = default;

    ActionUUID                              get_uuid() const;
    std::wstring                            get_name() const;

    std::shared_ptr<const XmlPropertiesDef> get_params_def() const;
    std::unique_ptr<XmlPropertiesDoc>       get_default_params() const;

    ActionRef                               get_ref() const;
    operator ActionRef() const;

    // override
    virtual MakeContent                     prepare_make_content() const;
    virtual std::unique_ptr<ReportType>     make_report_type(UUID uuid, std::wstring name, const XmlDoc& params) const;

protected:
    Action(const Action&) = default;
    Action& operator = (const Action&) = default;

    ActionUUID m_uuid;
    std::wstring m_name;
    std::shared_ptr<XmlPropertiesDef> m_params_def;
    boost::optional<ActionRef> m_ref;
};

class Actions
{
public:
    static const Priority ACTIONS_PRIORITY = 150;
    using ActionRange = boost::any_range<const Action, boost::forward_traversal_tag>;

    Actions();
    ~Actions();
    
    const Action&           add_action(std::unique_ptr<Action> action, Transaction& t);
    void                    remove_action(ActionUUID action_uuid, Transaction& t);
    bool                    has_action(ActionUUID action_uuid) const;
    const Action&           get_action(ActionUUID action_uuid) const;
    ActionRange             get_actions() const;

    template<class T>
    void connect_add_action(T slot, Priority priority) { m_add_action_sig.connect(slot, priority); }

    template<class T>
    void connect_remove_action(T slot, Priority priority) { m_remove_action_sig.connect(slot, priority); }

private:
    ReportTypes* m_report_types;
    std::unique_ptr<ActionStorage> m_storage;

    OrderedSignal<void(ActionRef action_ref, Transaction& t)> m_add_action_sig;
    OrderedSignal<void(ActionUUID action_uuid, Transaction& t)> m_remove_action_sig;
};

}} //namespace TR { namespace Core {