#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"
#include "BaseTreeModel.h"

class ArchiveTableModel;
class ArchiveReportRow: public BaseTreeModelRow
{
public:
    ArchiveReportRow(const TR::ArchivedReportInfo& archivedReportInfo);
    const TR::ArchivedReportInfo&   getArchivedReportInfo() const;
    ArchiveTableModel&              getArchiveTableModel() const;

private:
    TR::ArchivedReportInfo m_archivedReportInfo;
};

class ArchivedDiffRow: public BaseTreeModelRow
{
public:
    enum State {STATE_NONE = 0, STATE_ADD, STATE_DELETE};
    ArchivedDiffRow(size_t position);

    const ArchiveReportRow&         getParentRow() const;
    size_t                          getPosition() const;
    virtual State                   getState() const = 0;
    virtual Wt::WString             getString() const = 0;

protected:
    size_t m_position;
};


class PlainTextDiffLineRow: public ArchivedDiffRow
{
public:
    PlainTextDiffLineRow(size_t position);

    virtual State                   getState() const override;
    virtual Wt::WString             getString() const override;
    const TR::Core::DiffLine&       getDiffLine() const;
};

class PlainXmlDiffPointRow: public ArchivedDiffRow
{
public:
    PlainXmlDiffPointRow(size_t position);

    virtual State                   getState() const override;
    virtual Wt::WString             getString() const override;
    const TR::Core::XmlDiffPoint&   getDiffPoint() const;
};

class RegularDiffPointRow: public PlainXmlDiffPointRow
{
public:
    RegularDiffPointRow(size_t pointPos);
    virtual Wt::WString             getString() const override;
};

class ArchiveTableModel: public BaseTreeModel
{
public:
    friend class ArchiveReportFirstColumn;
    ArchiveTableModel();

    void addArchivedReportInfo(const TR::ArchivedReportInfo& archivedReportInfo);

private:
    std::map<TR::ReportTypeUUID, TR::ReportTypeInfo> m_reportTypes;
    //std::map<TR::SourceKey, std::wstring> m_sourceNames;
};

class ArchiveTable: public Wt::WTreeView
{
public:
    ArchiveTable();

    void addArchivedReportInfo(const TR::ArchivedReportInfo& archivedReportInfo);

private:
    std::shared_ptr<ArchiveTableModel> m_archiveTableModel;
};