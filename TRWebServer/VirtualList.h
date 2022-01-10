#pragma once

class VirtualList: public Wt::WTableView
{
public:
    friend class VirtualListDelegate;
    VirtualList(int size);
    void reset(int size);

protected:
    virtual std::unique_ptr<Wt::WContainerWidget> createWidget(int position) = 0;
};