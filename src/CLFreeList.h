#pragma once
#include <cstddef>

struct STFreeListNode
{
	STFreeListNode *m_next;
};

class CLFreeList
{
	const static unsigned long PTR_MASK		= 0x0000fffffffffffful;
	const static unsigned long VERSION_MASK	= 0xffff000000000000ul;
	const static unsigned int VERSION_SHIFT = 48u;

	STFreeListNode * m_next;

	inline STFreeListNode * makePtr(STFreeListNode * origin, STFreeListNode * updated);

public:
	CLFreeList() :m_next(NULL){}
	STFreeListNode * pop();
	void push(STFreeListNode *nodeToInsert);
};

inline STFreeListNode * CLFreeList::makePtr(STFreeListNode * origin, STFreeListNode * updated)
{
	return (STFreeListNode*)((((unsigned long)origin & VERSION_MASK) + (1ul << VERSION_SHIFT)) | (unsigned long)updated);
}

inline STFreeListNode * CLFreeList::pop()
{
	STFreeListNode *originHead, *next, *newHead;

	do
	{
		originHead = m_next;
		if (!((unsigned long)originHead & PTR_MASK))
			return NULL;

		next = ((STFreeListNode *)((unsigned long)originHead & PTR_MASK))->m_next;
		newHead = makePtr(originHead, next);
	} while (!__sync_bool_compare_and_swap(&m_next, originHead, newHead));

	return (STFreeListNode *)((unsigned long)originHead & PTR_MASK);
}

inline void CLFreeList::push(STFreeListNode * nodeToInsert)
{
	STFreeListNode *originHead, *newHead;

	do
	{
		originHead = m_next;
		nodeToInsert->m_next = (STFreeListNode*)((unsigned long)originHead & PTR_MASK);
		newHead = makePtr(originHead, nodeToInsert);
	} while (!__sync_bool_compare_and_swap(&m_next, originHead, newHead));

	return;
}
