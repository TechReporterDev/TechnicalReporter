#include "stdafx.h"
#include "boost/algorithm/string.hpp"
#include "TextDiffModel.h"
#include "Application.h"

TextDiffNodeRow::TextDiffNodeRow(State state, std::wstring line):
    m_state(state),
    m_line(std::move(line))
{
}

TextDiffModel* TextDiffNodeRow::getDiffModel() const
{
    return static_cast<TextDiffModel*>(getTreeModel());
}
    
TextDiffNodeRow::State TextDiffNodeRow::getDiffState() const
{
    return m_state;
}

const std::wstring& TextDiffNodeRow::getLine() const
{
    return m_line;
}

TextDiffDiffColumn::TextDiffDiffColumn(Side side):
    m_side(side)
{
}

std::wstring TextDiffDiffColumn::getName() const
{
    return m_side == SIDE_LEFT ? L"Previous" : L"Next";
}

std::wstring TextDiffDiffColumn::getString(const BaseTreeModelRow& row) const
{
    auto& diffNodeRow = dynamic_cast<const TextDiffNodeRow&>(row);
    switch (diffNodeRow.getDiffState())
    {
    case TextDiffNodeRow::STATE_ADDED:
        return m_side == SIDE_RIGHT ? diffNodeRow.getLine() : L"";

    case TextDiffNodeRow::STATE_REMOVED:
        return m_side == SIDE_LEFT ? diffNodeRow.getLine() : L"";

    case TextDiffNodeRow::STATE_NONE:
        return diffNodeRow.getLine();

    default:
        _ASSERT(false);
    }
    return L"";
}

TextDiffModel::TextDiffModel(const std::wstring& oldText, const std::vector<TR::Core::DiffLine>& diffLines)
{
    addColumn(std::make_unique<TextDiffDiffColumn>(TextDiffDiffColumn::SIDE_LEFT));
    addColumn(std::make_unique<TextDiffDiffColumn>(TextDiffDiffColumn::SIDE_RIGHT));
    build(oldText, diffLines);  
}

size_t TextDiffModel::getFirstDiff() const
{
    return getNextDiff(MAX_ROW_POS);
}

size_t TextDiffModel::getLastDiff() const
{
    return getPrevDiff(MAX_ROW_POS);
}

size_t TextDiffModel::getNextDiff(size_t currentPosition) const
{
    size_t startPosition = (currentPosition == MAX_ROW_POS) ? 0: currentPosition + 1;
    for (size_t position = startPosition; position < getRowCount(); ++position)
    { 
        auto& diffRow = getDiffRow(position);
        if (diffRow.getDiffState() != TextDiffNodeRow::STATE_NONE)
        {
            return position;
        }
    }

    return MAX_ROW_POS;
}


size_t TextDiffModel::getPrevDiff(size_t currentPosition) const
{
    auto startPosition = (currentPosition == MAX_ROW_POS) ? getRowCount() : currentPosition;
    for (size_t position = startPosition; position > 0; --position)
    { 
        auto& diffRow = getDiffRow(position - 1);
        if (diffRow.getDiffState() != TextDiffNodeRow::STATE_NONE)
        {
            return position - 1;
        }
    }
    return MAX_ROW_POS;
}

const TextDiffNodeRow& TextDiffModel::getDiffRow(size_t position) const
{
    return static_cast<const TextDiffNodeRow&>(getRow(position));
}

TextDiffNodeRow& TextDiffModel::getDiffRow(size_t position)
{
    return static_cast<TextDiffNodeRow&>(getRow(position));
}

const TextDiffDiffColumn& TextDiffModel::getColumn(size_t position) const
{
    return dynamic_cast<TextDiffDiffColumn&>(BaseTreeModel::getColumn(position));
}

void TextDiffModel::build(const std::wstring& oldText, const std::vector<TR::Core::DiffLine>& diffLines)
{
    std::vector<std::wstring> textLines;
    boost::split(textLines, oldText, boost::is_any_of(L"\n"));
    for (auto& line: textLines)
    {
        addRow(std::make_unique<TextDiffNodeRow>(TextDiffNodeRow::STATE_NONE, std::move(line)));
    }

    size_t oldPosition = 0;
    size_t newPosition = 0;
    size_t position = 0;

    for (auto diffLine : diffLines)
    {
        for (;;)
        {
            if (diffLine.m_diff_state == TR::Core::DIFF_ADD && newPosition == diffLine.m_line_pos)
            {
                _ASSERT(position <= getRowCount());             
                addRow(std::make_unique<TextDiffNodeRow>(TextDiffNodeRow::STATE_ADDED, diffLine.m_line_str), position);
                ++newPosition;
                ++position;
                break;
            }

            if (position >= getRowCount())
            {
                throw std::logic_error("Invalid text diff found");
            }

            if (diffLine.m_diff_state == TR::Core::DIFF_DELETE && oldPosition == diffLine.m_line_pos)
            {
                static_cast<TextDiffNodeRow&>(getRow(position)).m_state = TextDiffNodeRow::STATE_REMOVED;
                ++oldPosition;
                ++position;
                break;
            }

            ++newPosition;
            ++oldPosition;
            ++position;
        }
    }   
}