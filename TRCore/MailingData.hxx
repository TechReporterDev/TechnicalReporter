#pragma once
#include "BasisValues.hxx"
#include "RegistryValues.hxx"
#include "ServicesValues.hxx"
#include "AppValues.hxx"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)
	
struct MailingData
{
	MailingData():
		m_user_key(0)
	{
	}

	MailingData(Key user_key, std::string user_email):
		m_user_key(user_key),
		m_user_email(std::move(user_email))
	{
	}

	Key				m_user_key;
	std::string		m_user_email;
};

struct SentMailData
{
	SentMailData() :
		m_user_key(0),
		m_sent_summary_time(0)
	{
	}

	SentMailData(Key user_key, time_t sent_summary_time) :
		m_user_key(user_key),
		m_sent_summary_time(sent_summary_time)
	{
	}

	Key									m_user_key;
	time_t								m_sent_summary_time;
};

#pragma db object(MailingData)
#pragma db member(MailingData::m_user_key) id

#pragma db object(SentMailData)
#pragma db member(SentMailData::m_user_key) id
#pragma warning(pop)

}} //namespace CI { namespace Core {