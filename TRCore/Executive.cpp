#include "stdafx.h"
#include "Executive.h"
namespace TR { namespace Core {

Executive::Executive(std::shared_ptr<Executor> control, std::shared_ptr<Executor> processing, std::shared_ptr<Executor> io):
    m_control(control),
    m_processing(processing),
    m_io(io),
    m_strand_io(std::make_shared<SerialExecutors<int>>(*m_io))
{
}

std::unique_ptr<Executive> branch(const Executive& executive)
{
    return std::make_unique<Executive>(
        std::make_shared<ExpandingExecutor>(*executive.m_control),
        std::make_shared<ExpandingExecutor>(*executive.m_processing),
        std::make_shared<ExpandingExecutor>(*executive.m_io));
}

void stop(const Executive& executive)
{
    executive.m_io->stop();
    executive.m_processing->stop();
    executive.m_control->stop();
}

}} //namespace TR { namespace Core {