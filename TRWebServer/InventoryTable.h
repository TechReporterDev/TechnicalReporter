#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class InventoryTable: public Wt::WContainerWidget
{
public:
    struct Tool
    {
        Tool(Wt::WString name, std::function<void(int position, boost::any)> functor, bool default_ = false):
            m_name(std::move(name)),
            m_functor(std::move(functor)),
            m_default(default_)
        {
        }

        Wt::WString m_name;
        std::function<void(int position, boost::any)> m_functor;
        bool m_default;
    };

    InventoryTable();

    void    addTool(Wt::WString name, std::function<void(int position)> functor, bool default_ = false);
    void    addToolEx(Wt::WString name, std::function<void(int position, boost::any)> functor, bool default_ = false);
    void    setNameEditor(std::function<void(int position)> functor);
    void    setEmptyMessage(Wt::WString emptyMessage);
    
    void    setItem(int position, const Wt::WString& name, const Wt::WString& description, boost::any any = nullptr);
    void    insertItem(int position, const Wt::WString& name, const Wt::WString& description, boost::any any = nullptr);    
    void    deleteItem(int position);
    void    resize(int size);
    int     itemCount() const;

    void    setMoveMode(std::function<void(int position)> functor);
    bool    onMoveMode() const;
    void    cancelMoveMode();

private:
    void    onEditClick(int position);
    void    resetHold();
    void    showTable();
    void    showEmpty();

    std::vector<Tool> m_tools;
    std::function<void(int position)> m_moveFunctor;
    std::function<void(int position)> m_nameEditor;
    Wt::WString m_emptyMessage;
    bool m_hold;

    Wt::WTable* m_table;
};