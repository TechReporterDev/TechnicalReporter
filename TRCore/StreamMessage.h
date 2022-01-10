#pragma once
#include "stddecl.h"
#include "Content.h"
namespace TR { namespace Core {

class StreamMessage
{
public:
    enum Severety { INFO = 0, WARNING, FAILED };
    StreamMessage(Severety severety, std::wstring _event, time_t time, std::shared_ptr<RegularContent> body);
    
    Severety                            m_severety;
    std::wstring                        m_event;
    time_t                              m_time;
    std::shared_ptr<RegularContent>     m_body;
};

}} //namespace TR { namespace Core {