#include "stdafx.h"
#include "SourceBrowseDlg.h"
#include "StandardModel.h"
#include "Application.h"

namespace {
using SourceRow = StandardModelRow<TR::SourceInfo>;
using SourcesTreeModel = StandardModel<TR::SourceInfo>;

class SourceNameColumn: public StandardModelColumn<TR::SourceInfo>
{
public:
    virtual std::wstring getName() const
    {
        return L"Name";
    }

    virtual std::wstring getString(const SourceRow& row) const override
    {
        return row.getData().m_name;
    }   
};

class SourcesTreeBuilder
{
public:
    SourcesTreeBuilder(TR::SourceKey excludedKey = TR::SourceKey(0)):
        m_excludedKey(excludedKey)
    {
        auto& client = Application::instance()->getClient();
        for (auto& sourceInfo : client.getSourcesInfo())
        {
            if (sourceInfo.m_key == 1)
            {
                m_rootInfo = sourceInfo;
                continue;
            }
            m_sourceInfoByParentKey.insert(std::make_pair(sourceInfo.m_parent_key, sourceInfo));            
        }
    }

    void build(SourcesTreeModel& model)
    {
        _ASSERT(m_rootInfo);
        auto& rootRow = model.addRow(std::make_unique<SourceRow>(*m_rootInfo));
        build(rootRow);
    }

private:
    void build(SourceRow& parentRow)
    {
        for (auto& childInfo : m_sourceInfoByParentKey.equal_range(parentRow.getData().m_key) | boost::adaptors::map_values)
        {
            if (childInfo.m_key == m_excludedKey)
            {
                continue;
            }

            auto& childRow = parentRow.addChildRow(std::make_unique<SourceRow>(childInfo));
            build(childRow);
        }
    }

    TR::SourceKey m_excludedKey;
    std::multimap<TR::SourceKey, TR::SourceInfo> m_sourceInfoByParentKey;
    boost::optional<TR::SourceInfo> m_rootInfo; 
};

SourceRow* findRow(SourceRow& topRow, TR::SourceKey sourceKey)
{
    if (topRow.getData().m_key == sourceKey)
    {
        return &topRow;
    }

    for (size_t position = 0; position < topRow.getChildCount(); ++position)
    {
        if (auto foundRow = findRow(topRow.getChildRow(position), sourceKey))
        {
            return foundRow;
        }
    }

    return nullptr;
}

} // namespace {

SourceBrowseDlg::SourceBrowseDlg(TR::SourceKey selected, TR::SourceKey excluded):
    m_treeView(nullptr)
{
    setWindowTitle(L"Sources");
    setWidth(400);
    setHeight(480);
    setButtons(stl_tools::flag | BTN_OK | BTN_CANCEL);

    contents()->setPadding(10);
    contents()->setPadding(0, Wt::Side::Bottom);
    auto vbox = contents()->setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);

    auto model = std::make_shared<SourcesTreeModel>();
    model->addColumn(std::make_unique<SourceNameColumn>());
    SourcesTreeBuilder(excluded).build(*model);

    m_treeView = vbox->addWidget(std::make_unique<Wt::WTreeView>());
    m_treeView->setModel(model);
    m_treeView->setSelectionMode(Wt::SelectionMode::Single);
    m_treeView->expandToDepth(3);

    if (auto selectedRow = findRow(model->getRow(0), selected))
    {
        m_treeView->select(model->getIndexFromRow(*selectedRow));
    }   
}

void SourceBrowseDlg::connectApply(OnApply onApply)
{
    _ASSERT(!m_onApply);
    m_onApply = onApply;
}

void SourceBrowseDlg::apply()
{
    auto model = static_cast<SourcesTreeModel*>(m_treeView->model().get());
    for (auto index : m_treeView->selectedIndexes())
    {       
        auto& selectedRow = static_cast<SourceRow&>(model->getRowFromIndex(index));
        if (m_onApply)
        {
            m_onApply(selectedRow.getData());
        }
    }
}