#pragma once
#include "TRWebServer.h"
#include "PropertyDialog.h"
#include "StandardTable.h"

class TaskStateWidget;
class TaskDlg: public Dialog
{
public:
    TaskDlg(const Wt::WString& title, const Wt::WString& message, Wt::WObject* parent = 0);
    
    void setRunning(const Wt::WString& message);
    void setFailed(const Wt::WString& message, const Wt::WString& details = "");
    void setCompleted(const Wt::WString& message);
    
    Wt::WWidget* expand(std::unique_ptr<Wt::WWidget> widget);

    template<class Widget>
    Widget* expand(std::unique_ptr<Widget> widget)
    {
        auto pointer = widget.get();
        expand(std::unique_ptr<Wt::WWidget>(std::move(widget)));
        return pointer;
    }

private:
    TaskStateWidget* m_state;
};

class TaskBundleDlg: public TaskDlg
{
public:
    struct Task;
    using TaskDlg::setFailed;
    using TaskDlg::setCompleted;

    TaskBundleDlg(const Wt::WString& title, const Wt::WString& message, Wt::WObject* parent = 0);

    void    addTask(size_t id, const Wt::WString& name, const Wt::WString& message);
    void    setFailed(size_t id, const Wt::WString& message, const Wt::WString& details = "");
    void    setCompleted(size_t id, const Wt::WString& message);

    int     getFailedCount() const;
    int     getRunningCount() const;

private:
    size_t  findTask(size_t id) const;
    StandardTable<Task>* m_taskTable;
};