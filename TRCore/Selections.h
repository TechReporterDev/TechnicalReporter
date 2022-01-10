#pragma once
#include "BackgndFwd.h"
#include "BasisFwd.h"
#include "BasisRefs.h"
#include "stddecl.h"
namespace TR { namespace Core {
class SourceRef;

struct SelectionDirection
{
    SelectionDirection(ReportTypeRef input_ref, ReportTypeRef output_ref);
    SelectionDirection(std::vector<ReportTypeRef> input_refs, ReportTypeRef output_ref);

    std::vector<ReportTypeRef> m_input_refs;
    ReportTypeRef m_output_ref;
};

bool operator < (const SelectionDirection& left, const SelectionDirection& right);
bool operator == (const SelectionDirection& left, const SelectionDirection& right);

struct SelectionInput
{
    enum class Mode {STRICT_MODE = 0, NULLABLE_MODE, OPTIONAL_MODE};

    SelectionInput(ReportTypeRef report_type_ref, Mode mode = Mode::STRICT_MODE) :
        m_report_type_ref(report_type_ref),
        m_mode(mode)
    {
    }

    bool is_optional() const
    {
        return m_mode == Mode::OPTIONAL_MODE;
    }

    bool is_strict() const
    {
        return m_mode == Mode::STRICT_MODE;
    }

    ReportTypeRef m_report_type_ref;
    Mode m_mode;
};

bool operator < (const SelectionInput& left, const SelectionInput& right);
bool operator == (const SelectionInput& left, const SelectionInput& right);

struct SelectionPlan
{
    SelectionPlan(SelectionInput input_ref, ReportTypeRef output_ref):
        SelectionPlan(std::vector<SelectionInput>{ input_ref }, output_ref)
    {
    }

    SelectionPlan(std::vector<SelectionInput> input_refs, ReportTypeRef output_ref):
        m_input_refs(std::move(input_refs)),
        m_output_ref(output_ref)
    {
    }

    std::vector<SelectionInput> m_input_refs;
    ReportTypeRef m_output_ref;
};

inline  SelectionDirection get_direction(const SelectionPlan& selection_plan)
{
    return SelectionDirection(
        stl_tools::copy_vector(selection_plan.m_input_refs | stl_tools::members(&SelectionInput::m_report_type_ref)),
        selection_plan.m_output_ref);
}

inline std::vector<ReportTypeRef> get_inputs(const SelectionPlan& selection_plan)
{
    return stl_tools::copy_vector(selection_plan.m_input_refs | stl_tools::members(&SelectionInput::m_report_type_ref));
}

inline std::vector<ReportTypeRef> get_binding_inputs(const SelectionPlan& selection_plan)
{
    auto is_bound = [](const SelectionInput& input) { return !input.is_optional(); };

    return stl_tools::copy_vector(selection_plan.m_input_refs 
        | boost::adaptors::filtered(is_bound) 
        | stl_tools::members(&SelectionInput::m_report_type_ref));
}

class Selection
{
public:
    using Functor = std::function<std::unique_ptr<Content>(std::vector<std::shared_ptr<Content>>&)>;
    
    Selection(UUID uuid):
        m_uuid(std::move(uuid))
    {
    }
    virtual ~Selection() = default;

    UUID                        get_uuid() const { return m_uuid; }
    virtual SelectionPlan       get_plan() const = 0;
    virtual Functor             prepare_functor(SourceRef source_ref) const = 0;

private:
    UUID m_uuid;
};

class Selections
{
public:
    static const Priority SELECTIONS_PRIORITY = 600;
    
    Selections();
    ~Selections();
    void                                        set_report_types(ReportTypes* report_types);

    void                                        add_selection(std::shared_ptr<Selection> selection, Transaction& t);
    void                                        remove_selection(const SelectionDirection& direction, Transaction& t);
    std::shared_ptr<Selection>                  get_selection(const SelectionDirection& direction) const;
    std::shared_ptr<Selection>                  find_selection(const SelectionDirection& direction) const;
    std::vector<std::shared_ptr<Selection>>     find_by_input(ReportTypeRef input_ref) const;
    std::vector<std::shared_ptr<Selection>>     find_by_output(ReportTypeRef output_ref) const;

    template<class T>
    void connect_add_selection(T slot, Priority priority) { m_add_selection_sig.connect(slot, priority); }

    template<class T>
    void connect_remove_selection(T slot, Priority priority) { m_remove_selection_sig.connect(slot, priority); }

private:
    class Storage;  

    OrderedSignal<void(SelectionDirection direction, Transaction& t)> m_add_selection_sig;
    OrderedSignal<void(SelectionDirection direction, Transaction& t)> m_remove_selection_sig;

    ReportTypes* m_report_types;
    std::unique_ptr<Storage> m_storage;
};

}} //namespace TR { namespace Core {
