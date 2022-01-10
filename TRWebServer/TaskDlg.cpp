#include "stdafx.h"
#include "TaskDlg.h"
#include "PropertyBox.h"
#include "Application.h"

namespace {

static auto completedImage = "icons/sun01.png";
static auto failedImage = "icons/snow.png";
static auto runningImage = "icons/w_cloud.png";

enum TaskState { RUNNING_STATE, COMPLETED_STATE, FAILED_STATE };

const char* getImageName(TaskState state)
{
    switch (state)
    {
    case RUNNING_STATE:
        return runningImage;

    case COMPLETED_STATE:
        return completedImage;

    case FAILED_STATE:
        return failedImage;

    default:
        _ASSERT(false);
    }
    return runningImage;
}

class TaskDetailsLink: public Wt::WAnchor
{
public:
    TaskDetailsLink(const Wt::WString& details):
        Wt::WAnchor(Wt::WLink("javascript:void(0);"), "'details'")
    {
        clicked().connect([details](Wt::WMouseEvent){
            auto detailsBox = new PropertyBox(Dialog::BTN_CLOSE, L"Details", L"");
            detailsBox->addText(L"", L"", details, true);
            detailsBox->show();
        });
    }
};

} //namespace {

class TaskStateWidget: public Wt::WContainerWidget
{
public:
    TaskStateWidget()
    {
        auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>()); 
        vbox->setContentsMargins(0, 0, 0, 0);
    }

    void setState(TaskState state, const Wt::WString& message, const Wt::WString& details = "")
    {       
        auto vbox = Wt::clear(this);

        auto stateLayout = vbox->addLayout(std::make_unique<Wt::WHBoxLayout>());
        vbox->addSpacing(3);

        auto imageLayout = stateLayout->addLayout(std::make_unique<Wt::WVBoxLayout>());
        imageLayout->setSpacing(0);
        imageLayout->addStretch(1);
        auto image = imageLayout->addWidget(std::make_unique<Wt::WImage>(Wt::WLink(getImageName(state))));
        imageLayout->addStretch(1);
        
        auto messageLayout = stateLayout->addLayout(std::make_unique<Wt::WVBoxLayout>(), 1);
        messageLayout->setSpacing(0);
        messageLayout->addStretch(1);
        messageLayout->addWidget(std::make_unique<Wt::WText>(message));
        messageLayout->addStretch(1);

        if (!details.empty())
        {
            auto detailsLine = vbox->addWidget(std::make_unique<Wt::WTemplate>("Click ${link} to see more info"));
            detailsLine->bindWidget("link", std::make_unique<TaskDetailsLink>(details));
        }
    }
};

TaskDlg::TaskDlg(const Wt::WString& title, const Wt::WString& message, Wt::WObject* parent):
    Dialog(parent),
    m_state(nullptr)
{
    setWindowTitle(title);  
    setWidth(300);  

    contents()->setPadding(20);
    footer()->setMargin(0, Wt::Side::Top);

    auto vbox = contents()->setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);

    m_state = vbox->addWidget(std::make_unique<TaskStateWidget>()); 
    setRunning(message);
}

void TaskDlg::setRunning(const Wt::WString& message)
{
    m_state->setState(RUNNING_STATE, message);
    setButtons(BTN_CANCEL);
}

void TaskDlg::setFailed(const Wt::WString& message, const Wt::WString& details)
{
    m_state->setState(FAILED_STATE, message, details);
    setButtons(BTN_CLOSE);
}

void TaskDlg::setCompleted(const Wt::WString& message)
{
    m_state->setState(COMPLETED_STATE, message);
    setButtons(BTN_OK);
}

Wt::WWidget* TaskDlg::expand(std::unique_ptr<Wt::WWidget> widget)
{
    auto vbox = static_cast<Wt::WVBoxLayout*>(contents()->layout());
    return vbox->addWidget(std::move(widget));
}

struct TaskBundleDlg::Task
{
    int             m_index;
    std::wstring    m_name;
    TaskState       m_state;        
    std::wstring    m_message;
    std::wstring    m_details;
};

using TaskRow = StandardModelRow<TaskBundleDlg::Task>;
class TaskColumn: public StandardTableColumn<TaskBundleDlg::Task>
{
public:
    TaskColumn()
    {
    }
};

class TaskMessageColumn: public TaskColumn
{
public:
    virtual std::wstring getName() const override
    {
        return L"Message";
    }

    virtual std::wstring getString(const TaskRow& row) const override
    {
        return row.getData().m_message;
    }

    virtual void create(const TaskRow& row, Wt::WContainerWidget& container, Wt::WFlags<Wt::ViewItemRenderFlag> flags) override
    {       
        auto& details = row.getData().m_details;
        if (details.empty())
        {
            container.addWidget(std::make_unique<Wt::WText>(getString(row)));
            return;
        }

        auto message = container.addWidget(std::make_unique<Wt::WTemplate>("${message} (see ${link})"));
        message->setInline(true);
        message->bindWidget("message", std::make_unique<Wt::WText>(getString(row)));
        message->bindWidget("link", std::make_unique<TaskDetailsLink>(details));
    }
};

class TaskNameColumn: public TaskColumn
{
public: 
    virtual std::wstring getName() const override
    {
        return L"Name";
    }

    virtual std::wstring getString(const TaskRow& row) const override
    {
        return row.getData().m_name;
    }

    virtual void create(const TaskRow& row, Wt::WContainerWidget& container, Wt::WFlags<Wt::ViewItemRenderFlag> flags) override
    {
        auto image = container.addWidget(std::make_unique<Wt::WImage>(Wt::WLink(getImageName(row.getData().m_state))));
        image->resize(16, 16);
        image->setMargin(3, Wt::Side::Right);
        container.addWidget(std::make_unique<Wt::WText>(getString(row)));
    }
};

TaskBundleDlg::TaskBundleDlg(const Wt::WString& title, const Wt::WString& message, Wt::WObject* parent):
    TaskDlg(title, message, parent),
    m_taskTable(nullptr)
{
    setWidth(350);
    m_taskTable = expand(std::make_unique<StandardTable<Task>>());
    m_taskTable->setHeight(200);
    m_taskTable->setRowHeight(23);
    m_taskTable->addColumn(std::make_unique<TaskNameColumn>());
    m_taskTable->addColumn(std::make_unique<TaskMessageColumn>());
}

void TaskBundleDlg::addTask(size_t index, const Wt::WString& name, const Wt::WString& message)
{
    m_taskTable->addRow({
        int(index),
        std::wstring(name),
        RUNNING_STATE,
        std::wstring(message)
    });
}

void TaskBundleDlg::setFailed(size_t id, const Wt::WString& message, const Wt::WString& details)
{
    auto position = findTask(id);
    auto runningTask = m_taskTable->getData(position);
    runningTask.m_state = FAILED_STATE;
    runningTask.m_message = message;
    runningTask.m_details = details;
    m_taskTable->updateRow(position, runningTask);
}

void TaskBundleDlg::setCompleted(size_t id, const Wt::WString& message)
{
    auto position = findTask(id);
    auto runningTask = m_taskTable->getData(position);
    runningTask.m_state = COMPLETED_STATE;
    runningTask.m_message = message;
    runningTask.m_details.clear();
    m_taskTable->updateRow(position, runningTask);
}

int TaskBundleDlg::getFailedCount() const
{
    int counter = 0;
    for (size_t position = 0; position < m_taskTable->getRowCount(); ++position)
    {
        auto runningTask = m_taskTable->getData(position);
        if (runningTask.m_state == FAILED_STATE)
        {
            ++counter;
        }
    }
    return counter;
}

int TaskBundleDlg::getRunningCount() const
{
    int counter = 0;
    for (size_t position = 0; position < m_taskTable->getRowCount(); ++position)
    {
        auto runningTask = m_taskTable->getData(position);
        if (runningTask.m_state == RUNNING_STATE)
        {
            ++counter;
        }
    }
    return counter;
}

size_t TaskBundleDlg::findTask(size_t id) const
{
    for (size_t position = 0; position < m_taskTable->getRowCount(); ++position)
    {
        auto runningTask = m_taskTable->getData(position);
        if (runningTask.m_index == id)
        {
            return position;
        }
    }
    _ASSERT(false);
    return 0;
}