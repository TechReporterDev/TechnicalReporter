#pragma once
#include "TRCore\ComposedFeature.h"
#include "XmlFeatureDef.h"
#include "stddecl.h"
namespace TR { namespace External {

class ExternalSourceTypeFactory: public SourceTypeFactory
{
public:
    ExternalSourceTypeFactory(const std::wstring& feature_path, const XmlSourceTypeDef& source_type_def);
    virtual std::unique_ptr<SourceType> create_source_type(const CoreImpl& core_impl) const override;

private:
    const XmlSourceTypeDef& m_source_type_def;
};

class ExternalReportTypeFactory: public ReportTypeFactory
{
public:
    ExternalReportTypeFactory(const std::wstring& file_system_path, const XmlReportTypeDef& report_type_def);
    virtual std::unique_ptr<ReportType> create_report_type(const CoreImpl& core_impl) const override;

private:
    const XmlReportTypeDef& m_report_type_def;
};

class ExternalStreamTypeFactory: public StreamTypeFactory
{
public:
    ExternalStreamTypeFactory(const std::wstring& file_system_path, const XmlStreamTypeDef& stream_type_def);
    virtual std::unique_ptr<StreamType> create_stream_type(const CoreImpl& core_impl) const override;

private:
    const XmlStreamTypeDef& m_stream_type_def;
};

class ExternalActionFactory: public ActionFactory
{
public:
    ExternalActionFactory(const std::wstring& file_system_path, const XmlActionDef& action_def);
    virtual std::unique_ptr<Action> create_action(const CoreImpl& core_impl) const override;

private:
    const XmlActionDef& m_action_def;
};

class ExternalActionDelegateFactory: public ActionDelegateFactory
{
public:
    ExternalActionDelegateFactory(const std::wstring& file_system_path, const XmlActionDelegateDef& action_delegate_def);
    virtual std::unique_ptr<ActionDelegate> create_action_delegate(const CoreImpl& core_impl) const override;

private:
    const XmlActionDelegateDef& m_action_delegate_def;
};

class ExternalShortcutOutputFactory: public ReportTypeFactory
{
public:
    ExternalShortcutOutputFactory(const std::wstring& file_system_path, const XmlActionShortcutDef& action_shortcut_def);
    virtual std::unique_ptr<ReportType> create_report_type(const CoreImpl& core_impl) const override;

private:
    const XmlActionShortcutDef& m_action_shortcut_def;
};

class ExternalActionShortcutFactory: public ActionShortcutFactory
{
public:
    ExternalActionShortcutFactory(const std::wstring& file_system_path, const XmlActionShortcutDef& action_shortcut_def);
    virtual std::unique_ptr<ActionShortcut> create_action_shortcut(const CoreImpl& core_impl) const override;

private:
    const XmlActionShortcutDef& m_action_shortcut_def;
};

class ExternalSelectionFactory: public SelectionFactory
{
public:
    ExternalSelectionFactory(const std::wstring& file_system_path, const XmlSelectionDef& selection_def);
    virtual std::unique_ptr<Selection> create_selection(const CoreImpl& core_impl) const override;

private:
    const XmlSelectionDef& m_selection_def;
};

class ExternalFeature: public ComposedFeature
{
public:
    ExternalFeature(const std::wstring& file_system_path);
    
private:
    ExternalFeature(const std::wstring& file_system_path, std::unique_ptr<XmlFeatureDef> feature_def);
    std::unique_ptr<XmlFeatureDef> m_feature_def;
};

}} //namespace TR { namespace External {
