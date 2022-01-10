#pragma once
#include "BasisValues.hxx"
#include "RegistryValues.hxx"

namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4068)

struct SummaryData
{
	SummaryData():
		m_key(0),
		m_user_key(0),
		m_time(0),
		m_last(false)
	{
	}

	SummaryData(Key key, Key user_key, time_t time, bool last):
		m_key(key),
		m_user_key(user_key),
		m_time(time),
		m_last(last)
	{
	}

	Key		m_key;
	Key		m_user_key;
	time_t	m_time;
	bool	m_last;
};

struct SummaryContentData
{
	SummaryContentData():
		m_key(0)
	{
	}

	SummaryContentData(Key key, Blob content) :
		m_key(key),
		m_content(std::move(content))
	{
	}

	Key		m_key;
	Blob	m_content;
};

#pragma db value(Blob) type("LONGBLOB")
#pragma db object(SummaryData)
#pragma db member(SummaryData::m_key) id auto
#pragma db object(SummaryContentData)
#pragma db member(SummaryContentData::m_key) id

#pragma warning(pop)
}} //namespace CI { namespace Core {