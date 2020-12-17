#include "CLChannelsManager.h"
#include <new>

CLChannelsManager::CLChannelsManager():
	m_doneInit(false)
{
}

CLChannelsManager::CLChannelsManager(unsigned threadID):
	m_threadID(threadID),
	m_doneInit(false),
	m_curChannel(0)
{
	for (unsigned i = 0; i < CLPoliciesManager::MAX_THREAD_NUM - 1; i++)
	{
		new (&m_channels[i]) CLMessageChannel(true);

		if (!m_channels[i].doneInit())
			return;
	}

	m_doneInit = true;
}

bool CLChannelsManager::doneInit()
{
	return m_doneInit;
}

bool CLChannelsManager::pushBack(unsigned long val, unsigned threadID)
{
	if (threadID > m_threadID)
		threadID--;

	return m_channels[--threadID].pushBack(val);
}

unsigned long CLChannelsManager::pop()
{
	unsigned startChannel = m_curChannel;
	unsigned long res = m_channels[m_curChannel].popFront();
	while (!res) 
	{
		m_curChannel++;
		if (m_curChannel == CLPoliciesManager::MAX_THREAD_NUM - 1)
			m_curChannel = 0;

		if (m_curChannel == startChannel)
			break;

		res = m_channels[m_curChannel].popFront();
	}

	return res;
}

