#include "stdafx.h"
#include "StreamMessage.h"
namespace TR {namespace Core {

StreamMessage::StreamMessage(Severety severety, std::wstring _event, time_t time, std::shared_ptr<RegularContent> body):
    m_severety(severety),
    m_event(std::move(_event)),
    m_time(time),
    m_body(body)
{
}

}}//namespace TR {namespace Core {
