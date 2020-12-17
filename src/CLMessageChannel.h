#pragma once
#include "CLCacheLineQueue.h"
#include "CLFreeList.h"
#include "CLSlab.h"

template<typename TTestClass>
class _CLMessageChannel
{
	friend TTestClass;
	typedef CLFreeList QueueNodesPool;

	CLDRAMSlab * m_queueNodeAllocator;
	QueueNodesPool m_nodesPool;

	CLCacheLineQueue m_queue;

public:
	_CLMessageChannel(bool doRealInit = false);
	bool doneInit();
	bool pushBack(unsigned long val);
	unsigned long popFront();
};

typedef _CLMessageChannel<void> CLMessageChannel;