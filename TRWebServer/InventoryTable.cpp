#include "stdafx.h"
#include "InventoryTable.h"
#include "Layout.h"
#include "WidgetHolder.h"
#include "Application.h"

InventoryTable::InventoryTable():   
    m_emptyMessage(L"Empty inventory"),
    m_hold(false),
    m_table(nullptr)
{
    setOverflow(Wt::Overflow::Auto);
    showEmpty();
}

void InventoryTable::addTool(Wt::WString name, std::function<void(int position)> functor, bool default_)
{
    m_tools.push_back({std::move(name), [functor](int position, boost::any){functor(position);}, default_});
}

void InventoryTable::addToolEx(Wt::WString name, std::function<void(int position, boost::any)> functor, bool default_)
{
    m_tools.push_back({std::move(name), functor, default_});
}

void InventoryTable::setNameEditor(std::function<void(int position)> functor)
{
    m_nameEditor = functor;
}

void InventoryTable::setEmptyMessage(Wt::WString emptyMessage)
{
    m_emptyMessage = std::move(emptyMessage);
    if (!m_table)
    {
        showEmpty();
    }
}

void InventoryTable::setItem(int position, const Wt::WString& name, const Wt::WString& description, boost::any any)
{
    showTable();

    auto infoCell = m_table->elementAt(position, 0);
    infoCell->clear();

    auto title = infoCell->addWidget(std::make_unique<Wt::WContainerWidget>());
    title->addWidget(std::make_unique<Wt::WText>(L"<b>" + name + L"</b>"));

    if (m_nameEditor)
    {
        auto editBtn = title->addWidget(std::make_unique<Wt::WText>(L" (edit)"));
        editBtn->clicked().connect(std::bind(&InventoryTable::onEditClick, this, position));
        auto ds = editBtn->decorationStyle();
        ds.setCursor(Wt::Cursor::Arrow);
        editBtn->setDecorationStyle(ds);
    }

    if (!description.empty())
    {
        infoCell->addWidget(std::make_unique<Wt::WText>(description));
    }
    else
    {
        infoCell->setPadding(20);
    }

    if (!infoCell->clicked().isConnected())
    {
        infoCell->clicked().connect([this, infoCell, any](Wt::WMouseEvent){
            if (m_hold)
            {
                return;
            }

            auto position = infoCell->row();
            if (onMoveMode())
            {
                m_moveFunctor(position);
                cancelMoveMode();
                return;
            }

            LoadingIndicatorLock loadingLock;
            for (auto& tool : m_tools)
            {
                if (tool.m_default)
                {
                    tool.m_functor(position, any);
                    break;
                }
            }
        });     
    }

    auto toolsCell = m_table->elementAt(position, 1);
    toolsCell->clear();

    for (auto& tool : m_tools | boost::adaptors::reversed)
    {
        auto toolBtn = toolsCell->addWidget(std::make_unique<Wt::WPushButton>(tool.m_name));
        toolBtn->setStyleClass(L"tr_table_button");
        toolBtn->clicked().connect([this, &tool, infoCell, any](Wt::WMouseEvent){           
            if (onMoveMode())
            {
                cancelMoveMode();
            }   
            auto position = infoCell->row();
            tool.m_functor(position, any);
        });
    }
}

void InventoryTable::insertItem(int position, const Wt::WString& name, const Wt::WString& description, boost::any any)
{
    showTable();
    m_table->insertRow(0);
    setItem(position, name, description, any);
}

void InventoryTable::deleteItem(int position)
{
    _ASSERT(m_table);
    m_table->removeRow(position);
    
    if (m_table->rowCount() == 0)
    {
        showEmpty();
    }
}

void InventoryTable::resize(int size)
{
    if (size == 0)
    {
        showEmpty();
        return;
    }

    showTable();
    while (m_table->rowCount() > size)
    {
        m_table->removeRow(m_table->rowCount() - 1);
    }
}

int InventoryTable::itemCount() const
{
    return m_table->rowCount();
}

void InventoryTable::setMoveMode(std::function<void(int position)> functor)
{
    _ASSERT(!m_moveFunctor);
    m_moveFunctor = functor;
    m_table->addStyleClass("tr-move-mode");
}

bool InventoryTable::onMoveMode() const
{
    return m_moveFunctor != nullptr;
}

void InventoryTable::cancelMoveMode()
{
    _ASSERT(m_moveFunctor);
    m_moveFunctor = nullptr;
    m_table->removeStyleClass("tr-move-mode");
}

void InventoryTable::onEditClick(int position)
{
    _ASSERT(m_nameEditor);
    if (onMoveMode())
    {
        cancelMoveMode();
    }
    m_nameEditor(position);
    
    m_hold = true;
    Application::instance()->post(std::bind(&InventoryTable::resetHold, this));
}

void InventoryTable::resetHold()
{
    m_hold = false;
}

void InventoryTable::showTable()
{
    if (m_table)
        return;

    clear();
    auto container = addWidget(std::make_unique<Wt::WContainerWidget>());
    //container->setPadding(5);

    m_table = container->addWidget(std::make_unique<Wt::WTable>());
    m_table->addStyleClass("tr-inventory table form-inline table-hover");
    m_table->setWidth(Wt::WLength("100%"));
    m_table->columnAt(1)->setWidth(Wt::WLength(250));
}

void InventoryTable::showEmpty()
{
    clear();
    m_table = nullptr;
    auto container = addWidget(std::make_unique<Wt::WContainerWidget>());
    container->setHeight(Wt::WLength(100, Wt::LengthUnit::Percentage));

    auto center = centerLayout(container);
    center->addWidget(std::make_unique<Wt::WText>(m_emptyMessage)); 
}