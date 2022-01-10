#pragma once
#include "TRWebServer.h"
#include "TextDiffModel.h"

class TextDiffTableView: public Wt::WTableView
{
public:
    TextDiffTableView(const std::wstring& oldText, const std::vector<TR::Core::DiffLine>& diffLines);
    ~TextDiffTableView();

    void    selectNext();
    void    selectPrev();
    size_t  getSelected() const;

private:
    std::shared_ptr<TextDiffModel> m_model;
};