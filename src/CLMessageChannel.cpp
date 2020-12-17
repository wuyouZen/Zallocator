#include "CLMessageChannel.h"

template<typename TTestClass>
_CLMessageChannel<TTestClass>::_CLMessageChannel(bool doRealInit):
	m_queueNodeAllocator(NULL)
{
	if (doRealInit)
		m_queueNodeAllocator = new CLDRAMSlab(CACHE_LINE_SIZE);
}

template<typename TTestClass>
bool _CLMessageChannel<TTestClass>::doneInit()
{
	return m_queueNodeAllocator;
}

template<typename TTestClass>
bool _CLMessageChannel<TTestClass>::pushBack(unsigned long val)
{
	STCacheLineListNode * newNode;

	if (m_queue.pushBack(val))
		return true;
	else
	{
		newNode = (STCacheLineListNode *)m_nodesPool.pop();
		if (!newNode)
			newNode = (STCacheLineListNode *)m_queueNodeAllocator->allocObj();
		if (newNode)
			return m_queue.pushBack(val, newNode);
		else
			return false;
	}
}

template<typename TTestClass>
unsigned long _CLMessageChannel<TTestClass>::popFront()
{
	STCacheLineListNode * freeNode;
	unsigned long res;

	freeNode = m_queue.popFront(res);

	if (freeNode)
		m_nodesPool.push((STFreeListNode *)freeNode);

	return res;
}

template class _CLMessageChannel<void>;

class CLTestMessageChannel;
template class _CLMessageChannel<CLTestMessageChannel>;