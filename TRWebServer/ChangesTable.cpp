#include "stdafx.h"
#include "ChangesTable.h"
#include "CurrentReportPage.h"
#include "StreamPage.h"
#include "Application.h"

namespace {

struct GetSourceKey: public boost::static_visitor<TR::SourceKey>
{
    GetSourceKey() = default;

    TR::SourceKey operator()(const TR::CurrentReportInfo& reportInfo) const
    {
        return reportInfo.m_source_key;
    }

    TR::SourceKey operator()(const TR::StreamInfo& streamInfo) const
    {
        return streamInfo.m_source_key;
    }
};

struct GetResourceName: public boost::static_visitor<Wt::WString>
{
    GetResourceName(const ReportTypeInfoByUUID& reportTypes, const StreamTypeInfoByUUID& streamTypes):
        m_reportTypes(reportTypes),
        m_streamTypes(streamTypes)
    {
    }

    Wt::WString operator()(const TR::CurrentReportInfo& reportInfo) const
    {
        auto position = m_reportTypes.find(reportInfo.m_report_type_uuid);
        if (position != m_reportTypes.end())
        {
            return position->second.m_name;
        }
        return L"Unknown";
    }

    Wt::WString operator()(const TR::StreamInfo& streamInfo) const
    {
        auto position = m_streamTypes.find(streamInfo.m_stream_type_uuid);
        if (position != m_streamTypes.end())
        {
            return position->second.m_name;
        }
        return L"Unknown";
    }

    const ReportTypeInfoByUUID& m_reportTypes;
    const StreamTypeInfoByUUID& m_streamTypes;
};

struct GetModifiedDate: public boost::static_visitor<time_t>
{
    time_t operator()(const TR::CurrentReportInfo& reportInfo) const
    {
        return reportInfo.m_modified_time;
    }

    time_t operator()(const TR::StreamInfo& streamInfo) const
    {
        return streamInfo.m_time == 0;
    }
};

} // namespace

std::wstring ChangeTimeColumn::getName() const
{
    return L"Time";
}

std::wstring ChangeTimeColumn::getString(const ChangesTableRow& row) const
{
    auto changeTime = boost::apply_visitor(GetModifiedDate(), row.getData());
    return Application::formatTime(changeTime);
}

ChangeSourceColumn::ChangeSourceColumn(const std::map<TR::SourceKey, TR::SourceInfo>& sourcesInfo):
    m_sourcesInfo(sourcesInfo)
{
}

std::wstring ChangeSourceColumn::getName() const
{
    return L"Source";
}

std::wstring ChangeSourceColumn::getString(const ChangesTableRow& row) const
{
    auto sourceKey = boost::apply_visitor(GetSourceKey(), row.getData());
    auto position = m_sourcesInfo.find(sourceKey);
    if (position == m_sourcesInfo.end())
    {
        _ASSERT(false);
        return L"Unknown";
    }
    return position->second.m_name;
}

ChangeResourceColumn::ChangeResourceColumn(const std::map<TR::ReportTypeUUID, TR::ReportTypeInfo>& reportTypesInfo, const std::map<TR::StreamTypeUUID, TR::StreamTypeInfo>& streamTypesInfo):
    m_reportTypesInfo(reportTypesInfo),
    m_streamTypesInfo(streamTypesInfo)
{
}

std::wstring ChangeResourceColumn::getName() const
{
    return L"Resource";
}

std::wstring ChangeResourceColumn::getString(const ChangesTableRow& row) const
{
    return boost::apply_visitor(GetResourceName(m_reportTypesInfo, m_streamTypesInfo), row.getData());
}

ChangesTable::ChangesTable()
{
    setSelectionMode(Wt::SelectionMode::Single);

    doubleClicked().connect(std::bind([this](Wt::WModelIndex index){
        if (!index.isValid())
        {
            return;
        }

        auto& client = Application::instance()->getClient();
        auto change = getData(index.row());
        if (auto currentReportInfo = boost::get<TR::CurrentReportInfo>(&change))
        {
            Application::pushPage(createCurrentReportPage(*currentReportInfo));
        }
        else if (auto streamInfo = boost::get<TR::StreamInfo>(&change))
        {
            auto now = time(nullptr);
            Application::pushPage(std::make_unique<StreamPage>(*streamInfo, now - ONE_DAY, now, true));
        }
        else
        {
            _ASSERT(false);
        }
    }, std::placeholders::_1));
}

TR::SourceKey ChangesTable::getSourceKey(size_t row) const
{
    _ASSERT(getRowCount() > row);
    return boost::apply_visitor(GetSourceKey(), getData(row));
}