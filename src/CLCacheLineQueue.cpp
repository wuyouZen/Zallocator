#include "CLCacheLineQueue.h"
#include "Utils.h"
#include <cstddef>

template<typename TTestClass>
_CLCacheLineQueue<TTestClass>::_CLCacheLineQueue():
	m_head(0),
	m_tail(0)
{
}

template<typename TTestClass>
bool _CLCacheLineQueue<TTestClass>::pushBack(unsigned long val)
{
	unsigned long tail = m_tail.load(std::memory_order_relaxed);

	if (tail & OFFSET_IN_CACHE_LINE_MASK)
	{
		*(unsigned long *)tail = val;
		tail += 8;
		m_tail.store(tail, std::memory_order_release);
		return true;
	}
	else
		return false;
}

template<typename TTestClass>
bool _CLCacheLineQueue<TTestClass>::pushBack(unsigned long val, STCacheLineListNode * newNode)
{
	unsigned long tail = m_tail.load(std::memory_order_relaxed);
	STCacheLineListNode *lastNode = NULL;

	if (tail)
	{
		lastNode = (STCacheLineListNode *)(tail & ~OFFSET_IN_CACHE_LINE_MASK) - 1;
		lastNode->m_next = newNode;
	}

	newNode->m_next = NULL;
	newNode->m_vals[0] = val;
	tail = (unsigned long)(&(newNode->m_vals[1]));
	m_tail.store(tail, std::memory_order_release);

	if (!lastNode)
		m_head.store((unsigned long)(&(newNode->m_vals[0])), std::memory_order_release);

	return true;
}

template<typename TTestClass>
STCacheLineListNode * _CLCacheLineQueue<TTestClass>::popFront(unsigned long & res)
{
	STCacheLineListNode *firstNode, *nextNode;
	unsigned long tail = m_tail.load(std::memory_order_acquire);
	unsigned long head = m_head.load(std::memory_order_acquire);
	if (head == tail || !head)
	{
		res = 0;
		return NULL;
	}

	if (head & OFFSET_IN_CACHE_LINE_MASK)
	{
		res = *(unsigned long *)head;
		head += 8;
		m_head.store(head, std::memory_order_relaxed);
		return NULL;
	}
	else
	{
		firstNode = (STCacheLineListNode *)(head & ~OFFSET_IN_CACHE_LINE_MASK) - 1;
		nextNode = firstNode->m_next;
		res = nextNode->m_vals[0];
		head = (unsigned long)(&(nextNode->m_vals[1]));
		m_head.store(head, std::memory_order_relaxed);
		return firstNode;
	}
}

template class _CLCacheLineQueue<void>;

class CLTestCacheLineQueue;
template class _CLCacheLineQueue<CLTestCacheLineQueue>;