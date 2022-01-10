#pragma once
#include "BackgndFwd.h"
#include "BasisFwd.h"
#include "BasisRefs.h"
#include "stddecl.h"
namespace TR { namespace Core {
class SourceRef;
class ActionDelegateStorage;

struct ActionDependency
{
    ActionDependency(std::vector<ActionRef> host_action_refs, ActionRef guest_action_ref):
        m_host_action_refs(std::move(host_action_refs)),
        m_guest_action_ref(guest_action_ref)
    {
    }

    std::vector<ActionRef> m_host_action_refs;
    ActionRef m_guest_action_ref;
};

bool operator < (const ActionDependency& left, const ActionDependency& right);

class ActionDelegate
{
public:
    friend class ActionDelegates;
    using HostActionFunctor = std::function<std::unique_ptr<Content>(const XmlDoc& params)>;
    using ActionFunctor = std::function<std::unique_ptr<Content>(const XmlDoc& params, std::vector<std::shared_ptr<Content>> inputs, std::vector<HostActionFunctor> host_actions)>;

    ActionDelegate(std::vector<ReportTypeRef> input_refs, std::vector<ActionRef> host_action_refs, ActionRef guest_action_ref);
    virtual ~ActionDelegate() = default;    

    ActionDependency                        get_action_dependency() const;
    std::vector<ReportTypeRef>              get_input_refs() const;
    std::vector<ActionRef>                  get_host_action_refs() const;
    ActionRef                               get_guest_action_ref() const;

    virtual ActionFunctor                   prepare_action(SourceRef source_ref) const = 0;

protected:
    std::vector<ReportTypeRef> m_input_refs;
    std::vector<ActionRef> m_host_action_refs;
    ActionRef m_guest_action_ref;
};

class ActionDelegates
{
public:
    static const Priority ACTION_DELEGATES_PRIORITY = 165;  
    using ActionDelegateRange = boost::any_range<ActionDelegate, boost::forward_traversal_tag, ActionDelegate&, std::ptrdiff_t>;    
    
    ActionDelegates();
    ~ActionDelegates(); 
    void                                    set_report_types(ReportTypes* report_types);
    void                                    set_actions(Actions* actions);

    const ActionDelegate&                   add_action_delegate(std::unique_ptr<ActionDelegate> action_delegate, Transaction& t);
    void                                    remove_action_delegate(ActionDependency action_dependency, Transaction& t);
    const ActionDelegate&                   get_action_delegate(ActionDependency action_dependency) const;
    ActionDelegateRange                     find_by_host_action(ActionRef host_action_ref) const;
    ActionDelegateRange                     find_by_input(ReportTypeRef input_ref) const;

    template<class T>
    void connect_add_delegate(T slot, Priority priority) { m_add_delegate_sig.connect(slot, priority); }

    template<class T>
    void connect_remove_delegate(T slot, Priority priority) { m_remove_delegate_sig.connect(slot, priority); }

private:
    std::unique_ptr<ActionDelegateStorage> m_storage;
    
    OrderedSignal<void(const ActionDelegate& action_delegate, Transaction& t)> m_add_delegate_sig;
    OrderedSignal<void(ActionDependency action_dependency, Transaction& t)> m_remove_delegate_sig;
};

}}// namespace TR { namespace Core { 