#pragma once
#include "BasisFwd.h"
#include "BasisRefs.h"
#include "Features.h"
#include "stddecl.h"
namespace TR { namespace Core {

class SourceTypeFactory
{
public:
    SourceTypeFactory(SourceTypeUUID source_type_uuid, std::wstring source_type_name);
    virtual ~SourceTypeFactory() = default;

    SourceTypeUUID                          get_source_type_uuid() const;
    std::wstring                            get_source_type_name() const;
    virtual std::unique_ptr<SourceType>     create_source_type(const CoreImpl& core_impl) const = 0;

private:
    SourceTypeUUID m_source_type_uuid;
    std::wstring m_source_type_name;
};

class CustomSourceTypeFactory: public SourceTypeFactory
{
public:
    using Factory = std::function<std::unique_ptr<SourceType>(const CoreImpl& core_impl)>;

    CustomSourceTypeFactory(SourceTypeUUID source_type_uuid, std::wstring source_type_name, Factory factory);
    virtual std::unique_ptr<SourceType> create_source_type(const CoreImpl& core_impl) const override;

private:
    Factory m_factory;
};

class ReportTypeFactory
{
public:
    ReportTypeFactory(ReportTypeUUID report_type_uuid, std::wstring report_type_name);
    virtual ~ReportTypeFactory() = default;

    ReportTypeUUID                          get_report_type_uuid() const;
    std::wstring                            get_report_type_name() const;
    virtual std::unique_ptr<ReportType>     create_report_type(const CoreImpl& core_impl) const = 0;

private:
    ReportTypeUUID m_report_type_uuid;
    std::wstring m_report_type_name;
};

class StreamTypeFactory
{
public:
    StreamTypeFactory(StreamTypeUUID stream_type_uuid, std::wstring stream_type_name);
    virtual ~StreamTypeFactory() = default;

    StreamTypeUUID                          get_stream_type_uuid() const;
    std::wstring                            get_stream_type_name() const;
    virtual std::unique_ptr<StreamType>     create_stream_type(const CoreImpl& core_impl) const = 0;

private:
    StreamTypeUUID m_stream_type_uuid;
    std::wstring m_stream_type_name;
};

class CustomReportTypeFactory: public ReportTypeFactory
{
public:
    CustomReportTypeFactory(ReportTypeUUID report_type_uuid, std::wstring report_type_name, std::function<std::unique_ptr<ReportType>(const CoreImpl& core_impl)> factory);
    virtual std::unique_ptr<ReportType> create_report_type(const CoreImpl& core_impl) const override;

private:
    std::function<std::unique_ptr<ReportType>(const CoreImpl& core_impl)> m_factory;
};

class ActionFactory
{
public:
    ActionFactory(ActionUUID action_uuid, std::wstring action_name, std::string params_def);
    virtual ~ActionFactory() = default;

    ActionUUID                              get_action_uuid() const;
    std::wstring                            get_action_name() const;
    std::string                             get_params_def() const;

    virtual std::unique_ptr<Action>         create_action(const CoreImpl& core_impl) const = 0;

private:
    ActionUUID m_action_uuid;
    std::wstring m_action_name;
    std::string m_params_def;
};

class ActionDelegateFactory
{
public:
    ActionDelegateFactory(std::vector<ActionUUID> host_action_uuids, ActionUUID guest_action_uuid);
    virtual ~ActionDelegateFactory() = default;

    std::vector<ActionUUID>                     get_host_action_uuids() const;
    ActionUUID                                  get_guest_action_uuid() const;
    virtual std::unique_ptr<ActionDelegate>     create_action_delegate(const CoreImpl& core_impl) const = 0;

private:
    std::vector<ActionUUID> m_host_action_uuids;
    ActionUUID m_guest_action_uuid;
};

class ActionShortcutFactory
{
public:
    ActionShortcutFactory(UUID uuid, std::wstring name, ActionUUID host_action_uuid);
    virtual ~ActionShortcutFactory() = default;

    UUID                                        get_uuid() const;
    std::wstring                                get_name() const;
    ActionUUID                                  get_host_action_uuid() const;
    virtual std::unique_ptr<ActionShortcut>     create_action_shortcut(const CoreImpl& core_impl) const = 0;

protected:
    UUID m_uuid;
    std::wstring m_name;
    ActionUUID m_host_action_uuid;
};

class SelectionFactory
{
public:
    SelectionFactory(std::vector<ReportTypeUUID> input_uuids, ReportTypeUUID output_uuid);

    std::vector<ReportTypeUUID>             get_input_uuids() const;
    ReportTypeUUID                          get_output_uuid() const;

    virtual std::unique_ptr<Selection>      create_selection(const CoreImpl& core_impl) const = 0;

private:
    std::vector<ReportTypeUUID> m_input_uuids; 
    ReportTypeUUID m_output_uuid;
};

class CustomSelectionFactory: public SelectionFactory
{
public:
    using Factory = std::function<std::unique_ptr<Selection>(const CoreImpl& core_impl)>;
    
    CustomSelectionFactory(std::vector<ReportTypeUUID> input_uuids, ReportTypeUUID output_uuid, Factory factory);
    virtual std::unique_ptr<Selection> create_selection(const CoreImpl& core_impl) const override;

private:
    Factory m_factory;
};

class GroupingDefinition
{
public:
    GroupingDefinition(std::wstring name, ReportTypeUUID input_uuid, ReportTypeUUID output_uuid);
    
    std::wstring                        get_name() const;
    ReportTypeUUID                      get_input_uuid() const;
    ReportTypeUUID                      get_output_uuid() const;

private:
    std::wstring m_name;
    ReportTypeUUID m_input_uuid;
    ReportTypeUUID m_output_uuid;
};

class ComplianceDefinition
{
public:
    ComplianceDefinition(std::wstring name, ReportTypeUUID input_uuid, ReportTypeUUID output_uuid);

    std::wstring                        get_name() const;
    ReportTypeUUID                      get_input_uuid() const;
    ReportTypeUUID                      get_output_uuid() const;

private:
    std::wstring m_name;
    ReportTypeUUID m_input_uuid;
    ReportTypeUUID m_output_uuid;
};

class ValidationDefinition
{
public:
    ValidationDefinition(std::wstring name, ReportTypeUUID input_uuid, ReportTypeUUID output_uuid, boost::optional<std::string> default_validator);
    ValidationDefinition(std::wstring name, ReportTypeUUID input_uuid, ReportTypeUUID output_uuid);

    std::wstring                        get_name() const;
    ReportTypeUUID                      get_input_uuid() const;
    ReportTypeUUID                      get_output_uuid() const;
    boost::optional<std::string>        get_default_validator() const;

private:
    std::wstring m_name;
    ReportTypeUUID m_input_uuid;
    ReportTypeUUID m_output_uuid;
    boost::optional<std::string> m_default_validator;
};

class ComposedFeature: public Feature
{
public:
    ComposedFeature(UUID uuid, std::wstring name);

    void            add_dependency(UUID feature_uuid);

    void            add_source_type_factory(std::unique_ptr<SourceTypeFactory> source_type_factory);
    void            add_report_type_factory(std::unique_ptr<ReportTypeFactory> report_type_factory);    
    void            add_stream_type_factory(std::unique_ptr<StreamTypeFactory> stream_type_factory);
    void            add_selection_factory(std::unique_ptr<SelectionFactory> selection_factory);
    void            add_action_factory(std::unique_ptr<ActionFactory> action_factory);
    void            add_action_delegate_factory(std::unique_ptr<ActionDelegateFactory> action_delegate_factory);
    void            add_action_shortcut_factory(std::unique_ptr<ActionShortcutFactory> action_shortcut_factory);

    void            add_grouping_definition(GroupingDefinition grouping_definition);
    void            add_compliance_definition(ComplianceDefinition compliance_definition);
    void            add_validation_definition(ValidationDefinition validation_definition);

protected:
    virtual void    install(CoreImpl& core_impl, bool restore, Transaction& t) override;
    virtual void    uninstall(CoreImpl& core_impl, Transaction& t) override;

    std::vector<std::unique_ptr<SourceTypeFactory>> m_source_type_factories;
    std::vector<std::unique_ptr<ReportTypeFactory>> m_report_type_factories;
    std::vector<std::unique_ptr<StreamTypeFactory>> m_stream_type_factories;
    std::vector<std::unique_ptr<SelectionFactory>> m_selection_factories;
    std::vector<std::unique_ptr<ActionFactory>> m_action_factories;
    std::vector<std::unique_ptr<ActionDelegateFactory>> m_action_delegate_factories;
    std::vector<std::unique_ptr<ActionShortcutFactory>> m_action_shortcut_factories;

    std::vector<GroupingDefinition> m_grouping_definitions;
    std::vector<ComplianceDefinition> m_compliance_definitions;
    std::vector<ValidationDefinition> m_validation_definitions;
};

}} //namespace TR { namespace Core {