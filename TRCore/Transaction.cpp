#include "stdafx.h"
#include "Transaction.h"

namespace TR { namespace Core {

class TransactionSlot
{
public:
    TransactionSlot(std::function<void()> slot, bool activate):
        m_slot(slot),
        m_activator(activate)
    {
    }

    void operator()()
    {
        if (m_activator.get_value())
        {
            m_slot();
        }       
    }

    Activator get_activator()
    {
        return m_activator;
    }

private:
    std::function<void()> m_slot;
    Activator m_activator;
};

Activator::Activator(bool value):
    m_value(std::make_shared<bool>(value))
{
}

void Activator::activate(bool value) noexcept
{
    *m_value = value;
}

bool Activator::get_value() const
{
    return *m_value;
}

Transaction::Transaction(Database& db):
    m_t(db.begin())
{
}

Transaction::~Transaction()
{
    if (!m_t.finalized())
    {
        rollback();
    }
}

void Transaction::commit()
{
    m_t.commit();

    no_except([this]
    {
        m_commit_sig();
    });
}

void Transaction::rollback()
{
    m_t.rollback();

    no_except([this]
    {
        m_rollback_sig();
    });
}

Activator Transaction::connect_commit(std::function<void()> slot, bool activate)
{
    TransactionSlot t_slot(slot, activate); 
    auto activator = t_slot.get_activator();
    m_commit_sig.connect(std::move(t_slot));
    return activator;
}

Activator Transaction::connect_rollback(std::function<void()> slot, bool activate)
{
    TransactionSlot t_slot(slot, activate);
    auto activator = t_slot.get_activator();
    m_rollback_sig.connect(std::move(t_slot), boost::signals2::connect_position::at_front);
    return activator;
}

ReadOnlyTransaction::ReadOnlyTransaction(Database& db)
{
    if(!odb::core::transaction::has_current())
    {
        m_t.reset(db.begin());
    }
}

}} //namespace TR { namespace Core {