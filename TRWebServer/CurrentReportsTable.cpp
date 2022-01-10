#include "stdafx.h"
#include "CurrentReportsTable.h"
#include "BaseTreeModel.h"
#include "CurrentReportPage.h"
#include "OpenStreamDlg.h"
#include "Application.h"

struct GetResourceName: public boost::static_visitor<Wt::WString>
{
    GetResourceName(std::map<TR::ReportTypeUUID, TR::ReportTypeInfo>& reportTypes, std::map<TR::StreamTypeUUID, TR::StreamTypeInfo>& streamTypes):
        m_reportTypes(reportTypes),
        m_streamTypes(streamTypes)
    {
    }

    Wt::WString operator()(const TR::CurrentReportInfo& reportInfo) const
    {
        return m_reportTypes[reportInfo.m_report_type_uuid].m_name;
    }

    Wt::WString operator()(const TR::StreamInfo& streamInfo) const
    {
        return m_streamTypes[streamInfo.m_stream_type_uuid].m_name;
    }

    std::map<TR::ReportTypeUUID, TR::ReportTypeInfo>& m_reportTypes;
    std::map<TR::StreamTypeUUID, TR::StreamTypeInfo>& m_streamTypes;    
};

struct GetModifiedDate: public boost::static_visitor<Wt::WString>
{
    Wt::WString operator()(const TR::CurrentReportInfo& reportInfo) const
    {
        if (reportInfo.m_deferred)
        {
            return L"deferred";
        }

        if (reportInfo.m_modified_time == 0)
        {
            return L"empty";
        }

        struct tm tm = {0};
        localtime_s(&tm, &reportInfo.m_modified_time);
        return boost::lexical_cast<std::wstring>(std::put_time(&tm, L"%c"));
    }

    Wt::WString operator()(const TR::StreamInfo& streamInfo) const
    {
        if (streamInfo.m_time == 0)
        {
            return L"empty";
        }

        struct tm tm = {0};
        localtime_s(&tm, &streamInfo.m_time);
        return boost::lexical_cast<std::wstring>(std::put_time(&tm, L"%c"));
    }
};

struct GetUpdatedDate: public boost::static_visitor<Wt::WString>
{
    Wt::WString operator()(const TR::CurrentReportInfo& reportInfo) const
    {
        if (reportInfo.m_deferred)
        {
            return L"deferred";
        }

        if (reportInfo.m_update_time == 0)
        {
            return L"empty";
        }

        struct tm tm = {0};
        localtime_s(&tm, &reportInfo.m_update_time);
        return boost::lexical_cast<std::wstring>(std::put_time(&tm, L"%c"));
    }

    Wt::WString operator()(const TR::StreamInfo& streamInfo) const
    {
        if (streamInfo.m_time == 0)
        {
            return L"empty";
        }

        struct tm tm = {0};
        localtime_s(&tm, &streamInfo.m_time);
        return boost::lexical_cast<std::wstring>(std::put_time(&tm, L"%c"));
    }
};

CurrentReportNameColumn::CurrentReportNameColumn(std::map<TR::ReportTypeUUID, TR::ReportTypeInfo>& reportTypes, std::map<TR::StreamTypeUUID, TR::StreamTypeInfo>& streamTypes):
    m_reportTypes(reportTypes),
    m_streamTypes(streamTypes)
{
}

std::wstring CurrentReportNameColumn::getName() const
{
    return L"Name";
}

std::wstring CurrentReportNameColumn::getString(const CurrentReportRow& row) const
{
    return boost::apply_visitor(GetResourceName(m_reportTypes, m_streamTypes), row.getData());  
}

std::wstring CurrentReportModifiedDateColumn::getName() const
{
    return L"Modified";
}

std::wstring CurrentReportModifiedDateColumn::getString(const CurrentReportRow& row) const
{
    return boost::apply_visitor(GetModifiedDate(), row.getData());
}


std::wstring CurrentReportUpdatedDateColumn::getName() const
{
    return L"Updated";
}

std::wstring CurrentReportUpdatedDateColumn::getString(const CurrentReportRow& row) const
{
    return boost::apply_visitor(GetUpdatedDate(), row.getData());
}

CurrentReportsTable::CurrentReportsTable()
{
    auto& client = Application::instance()->getClient();
    for (auto& reportTypeInfo : client.getReportTypesInfo())
    {
        m_reportTypes[reportTypeInfo.m_uuid] = reportTypeInfo;
    }

    for (auto& streamTypeInfo : client.getStreamTypesInfo())
    {
        m_streamTypes[streamTypeInfo.m_uuid] = streamTypeInfo;
    }

    setSelectionMode(Wt::SelectionMode::Single);
    doubleClicked().connect(std::bind([this](Wt::WModelIndex index){
        if (!index.isValid())
        {
            return;
        }

        auto& resourceInfo = getData(index.row());
        if (auto reportInfo = boost::get<const TR::CurrentReportInfo>(&resourceInfo))
        {
            Application::showPage(createCurrentReportPage(*reportInfo));
        }
        else if (auto streamInfo = boost::get<const TR::StreamInfo>(&resourceInfo))
        {
            auto openStreamDlg = new OpenStreamDlg(*streamInfo);
            openStreamDlg->show();
        }
        else
        {
            _ASSERT(false);
        }
    }, std::placeholders::_1));

    addColumn(std::make_unique<CurrentReportNameColumn>(m_reportTypes, m_streamTypes));
    addColumn(std::make_unique<CurrentReportModifiedDateColumn>());
    addColumn(std::make_unique<CurrentReportUpdatedDateColumn>());
}