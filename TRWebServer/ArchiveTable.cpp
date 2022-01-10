#include "stdafx.h"
#include "ArchiveTable.h"
#include "ArchivedReportPage.h"
#include "Application.h"

ArchiveReportRow::ArchiveReportRow(const TR::ArchivedReportInfo& archivedReportInfo):
    m_archivedReportInfo(archivedReportInfo)
{
    if (auto plainTextDiff = std::dynamic_pointer_cast<TR::PlainTextDiff>(m_archivedReportInfo.m_diff))
    {
        for (size_t i = 0; i < plainTextDiff->get_diff_lines().size(); ++i)
        {
            addChildRow(std::make_unique<PlainTextDiffLineRow>(i));
        }
    }
    else if (auto regularDiff = std::dynamic_pointer_cast<TR::RegularDiff>(m_archivedReportInfo.m_diff))
    {
        for (size_t i = 0; i < regularDiff->get_diff_points().size(); ++i)
        {
            addChildRow(std::make_unique<RegularDiffPointRow>(i));
        }       
    }
    else if (auto plainXmlDiff = std::dynamic_pointer_cast<TR::PlainXmlDiff>(m_archivedReportInfo.m_diff))
    {
        for (size_t i = 0; i < plainXmlDiff->get_diff_points().size(); ++i)
        {
            addChildRow(std::make_unique<PlainXmlDiffPointRow>(i));
        }
    }
    else
    {
        _ASSERT(false);
    }
}

const TR::ArchivedReportInfo& ArchiveReportRow::getArchivedReportInfo() const
{
    return m_archivedReportInfo;
}

ArchiveTableModel& ArchiveReportRow::getArchiveTableModel() const
{
    return dynamic_cast<ArchiveTableModel&>(*getTreeModel());
}

ArchivedDiffRow::ArchivedDiffRow(size_t position):
    m_position(position)
{
}

const ArchiveReportRow& ArchivedDiffRow::getParentRow() const
{
    return dynamic_cast<const ArchiveReportRow&>(*BaseTreeModelRow::getParentRow());
}

size_t ArchivedDiffRow::getPosition() const
{
    return m_position;
}

PlainTextDiffLineRow::PlainTextDiffLineRow(size_t position):
    ArchivedDiffRow(position)
{
}

ArchivedDiffRow::State PlainTextDiffLineRow::getState() const
{
    switch (getDiffLine().m_diff_state)
    {
    case TR::Core::DIFF_ADD:
        return STATE_ADD;

    case TR::Core::DIFF_DELETE:
        return STATE_DELETE;

    default:
        _ASSERT(false);
    }

    return STATE_NONE;
}

Wt::WString PlainTextDiffLineRow::getString() const
{
    return getDiffLine().m_line_str;
}

const TR::Core::DiffLine& PlainTextDiffLineRow::getDiffLine() const
{
    auto plainTextDiff = std::dynamic_pointer_cast<TR::PlainTextDiff>(getParentRow().getArchivedReportInfo().m_diff);
    return plainTextDiff->get_diff_lines().at(m_position);
}

PlainXmlDiffPointRow::PlainXmlDiffPointRow(size_t position):
    ArchivedDiffRow(position)
{
}

ArchivedDiffRow::State PlainXmlDiffPointRow::getState() const
{
    switch (getDiffPoint().m_state)
    {
    case TR::Core::XmlDiffPoint::STATE_INSERTED:
        return STATE_ADD;

    case TR::Core::XmlDiffPoint::STATE_REMOVED:
        return STATE_DELETE;

    default:
        _ASSERT(false);
    }

    return STATE_NONE;
}

Wt::WString PlainXmlDiffPointRow::getString() const
{
    Wt::WString path;
    auto& diffPoint = getDiffPoint();
    for (auto& nodeData : diffPoint.m_path)
    {
        if (!path.empty())
        {
            path += L" / ";
        }

        path += nodeData.m_name;
        if (nodeData.m_id)
        {
            path += L":";
            path += stl_tools::utf8_to_ucs(*nodeData.m_id);
        }

        if (nodeData.m_value)
        {
            path += L" = ";
            path += stl_tools::utf8_to_ucs(*nodeData.m_value);
        }
    }
    return path;
}

const TR::Core::XmlDiffPoint& PlainXmlDiffPointRow::getDiffPoint() const
{
    auto xmlDiff = std::dynamic_pointer_cast<TR::PlainXmlDiff>(getParentRow().getArchivedReportInfo().m_diff);
    return xmlDiff->get_diff_points().at(m_position);
}

RegularDiffPointRow::RegularDiffPointRow(size_t position):
    PlainXmlDiffPointRow(position)
{
}

Wt::WString RegularDiffPointRow::getString() const
{
    auto regularDiff = std::dynamic_pointer_cast<TR::RegularDiff>(getParentRow().getArchivedReportInfo().m_diff);
    return regularDiff->format_path(m_position);
}

class ArchiveReportFirstColumn: public BaseTreeModelColumn
{
public:
    virtual std::wstring getName() const override
    {
        return L"Report Name";
    }

    virtual std::wstring getString(const BaseTreeModelRow& row) const
    {
        if (auto archivedReportRow = dynamic_cast<const ArchiveReportRow*>(&row))
        {
            auto& archiveTable = archivedReportRow->getArchiveTableModel();
            auto reportTypeUUID = archivedReportRow->getArchivedReportInfo().m_report_type_uuid;
            return archiveTable.m_reportTypes[reportTypeUUID].m_name;           
        }

        return L"";
    }
};

class ArchiveReportSecondColumn: public BaseTreeModelColumn
{
public:
    virtual std::wstring getName() const override
    {
        return L"Date";
    }

    virtual std::wstring getString(const BaseTreeModelRow& row) const
    {
        if (auto archivedReportRow = dynamic_cast<const ArchiveReportRow*>(&row))
        {
            auto& archiveTableModel = archivedReportRow->getArchiveTableModel();
            auto time = archivedReportRow->getArchivedReportInfo().m_time;
            struct tm tm = {0};
            localtime_s(&tm, &time);
            return boost::lexical_cast<std::wstring>(std::put_time(&tm, L"%c"));
        }

        return L"";
    }
};

ArchiveTableModel::ArchiveTableModel()
{
    auto& client = Application::instance()->getClient();
    for (auto& reportTypeInfo : client.getReportTypesInfo())
    {
        m_reportTypes[reportTypeInfo.m_uuid] = reportTypeInfo;
    }

    addColumn(std::make_unique<ArchiveReportFirstColumn>());
    addColumn(std::make_unique<ArchiveReportSecondColumn>());
}

void ArchiveTableModel::addArchivedReportInfo(const TR::ArchivedReportInfo& archivedReportInfo)
{
    addRow(std::make_unique<ArchiveReportRow>(archivedReportInfo));
}

class ArchiveTableDelegate: public Wt::WAbstractItemDelegate
{
public:
    ArchiveTableDelegate()      
    {
    }

    virtual std::unique_ptr<Wt::WWidget> update(Wt::WWidget *widget, const Wt::WModelIndex& index, Wt::WFlags<Wt::ViewItemRenderFlag> flags)
    {
        if (widget)
        {
            return nullptr;
        }

        auto container = std::make_unique<Wt::WContainerWidget>();
        auto& row = getRowFromIndex(index);
        if (auto archiveReportRow = dynamic_cast<const ArchiveReportRow*>(&row))
        {
            container->addWidget(std::make_unique<Wt::WText>(Wt::cpp17::any_cast<std::string>(index.data(Wt::ItemDataRole::Display))));
        }
        else if (auto diffRow = dynamic_cast<const ArchivedDiffRow*>(&row))
        {
            if (index.column() != 0)
            {
                return container;
            }
            
            wchar_t diffSign = L'=';
            switch (diffRow->getState())
            {
            case ArchivedDiffRow::STATE_ADD:
                container->addStyleClass("tr-add-diff-line");
                diffSign = L'+';
                break;

            case ArchivedDiffRow::STATE_DELETE:
                container->addStyleClass("tr-del-diff-line");
                diffSign = L'-';
                break;

            default:
                _ASSERT(false);
            }                       

            auto diffString = boost::wformat(L"%1%: %2%") % diffSign % diffRow->getString();
            container->addWidget(std::make_unique<Wt::WText>(diffString.str()));
        }
    
        return container;
    }
};


ArchiveTable::ArchiveTable():
    m_archiveTableModel(std::make_shared<ArchiveTableModel>())
{   
    setSelectionMode(Wt::SelectionMode::Single);

    doubleClicked().connect(std::bind([this](Wt::WModelIndex index){
        if (!index.isValid())
        {
            return;
        }

        auto& row = getRowFromIndex(index);
        if (auto archiveReportRow = dynamic_cast<const ArchiveReportRow*>(&row))
        {
            Application::showPage(std::make_unique<ArchivedReportPage>(archiveReportRow->getArchivedReportInfo()));
        }       
    }, std::placeholders::_1));
    
    setModel(m_archiveTableModel);  
    setItemDelegate(std::make_shared<ArchiveTableDelegate>());
    setColumnWidth(0, 300);
    setColumnWidth(1, 300);
}

void ArchiveTable::addArchivedReportInfo(const TR::ArchivedReportInfo& archivedReportInfo)
{
    m_archiveTableModel->addArchivedReportInfo(archivedReportInfo);
}

