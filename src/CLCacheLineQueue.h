#pragma once
#include "CLFreeList.h"
#include <atomic>

struct STCacheLineListNode
{
	union
	{
		STCacheLineListNode *m_next;
		STFreeListNode m_freeListNode;
	};
	unsigned long m_vals[7];
};

template<typename TTestClass>
class _CLCacheLineQueue
{
	friend TTestClass;
	const static unsigned long OFFSET_IN_CACHE_LINE_MASK = 63ul;

	std::atomic_ulong m_head;
	std::atomic_ulong m_tail;

public:
	_CLCacheLineQueue();
	
	bool pushBack(unsigned long val);
	bool pushBack(unsigned long val, STCacheLineListNode * newNode);
	STCacheLineListNode * popFront(unsigned long & res);
};

typedef _CLCacheLineQueue<void> CLCacheLineQueue;