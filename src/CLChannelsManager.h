#pragma once
#include "CLMessageChannel.h"
#include "CLPoliciesManager.h"

class CLChannelsManager
{
	CLMessageChannel m_channels[CLPoliciesManager::MAX_THREAD_NUM - 1];
	unsigned m_threadID;
	unsigned m_curChannel;
	bool m_doneInit;

public:
	CLChannelsManager();
	CLChannelsManager(unsigned threadID);
	bool doneInit();

	bool pushBack(unsigned long val, unsigned threadID);
	unsigned long pop();
};