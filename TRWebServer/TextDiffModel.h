#pragma once
#include "TRWebServer.h"
#include "BaseTreeModel.h"

class TextDiffNodeRow: public BaseTreeModelRow
{
public:
    friend class TextDiffModel;

    enum State { STATE_NONE = 0, STATE_ADDED, STATE_REMOVED };
    TextDiffNodeRow(State state, std::wstring line);

    TextDiffModel*              getDiffModel() const;   
    
    State                       getDiffState() const;
    const std::wstring&         getLine() const;
    
protected:  
    State m_state;
    std::wstring m_line;
};

class TextDiffDiffColumn: public BaseTreeModelColumn
{
public:
    enum Side { SIDE_LEFT = 0, SIDE_RIGHT };

    TextDiffDiffColumn(Side side);
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const BaseTreeModelRow& row) const override;

private:
    Side m_side;
};

class TextDiffModel: public BaseTreeModel
{
public:
    TextDiffModel(const std::wstring& oldText, const std::vector<TR::Core::DiffLine>& diffLines);
    virtual ~TextDiffModel() = default;

    TextDiffModel(const TextDiffModel&) = delete;
    TextDiffModel& operator = (const TextDiffModel&) = delete;

    size_t                      getFirstDiff() const;
    size_t                      getLastDiff() const;
    size_t                      getNextDiff(size_t currentPosition) const;
    size_t                      getPrevDiff(size_t currentPosition) const;
    const TextDiffNodeRow&      getDiffRow(size_t position) const;
    TextDiffNodeRow&            getDiffRow(size_t position);

    const TextDiffDiffColumn&   getColumn(size_t position) const;

private:
    void build(const std::wstring& oldText, const std::vector<TR::Core::DiffLine>& diffLines);
};