#include "CLNVMLargeObjAllocator.h"
#include "CLPoliciesManager.h"

template<typename TTestClass>
inline _CLNVMLargeObjAllocator<TTestClass>::_CLNVMLargeObjAllocator(CLDRAMSlab * treeNodeAllocator):
	m_treeNodeAllocator(treeNodeAllocator)
{
}

template<typename TTestClass>
bool _CLNVMLargeObjAllocator<TTestClass>::alloc(size_t size, unsigned long & offset)
{
	CLIntervalTreeNode * node = NULL;
	unsigned long len;

	node = m_intervalTree.getAndRemove(size >> CLPoliciesManager::NVM_PAGE_SHIFT);

	if (node)
	{
		offset = node->getIntervalStart() << CLPoliciesManager::NVM_PAGE_SHIFT;
		len = node->getIntervalLen() << CLPoliciesManager::NVM_PAGE_SHIFT;

		if (len > size)
		{
			node->setInterval((offset + size) >> CLPoliciesManager::NVM_PAGE_SHIFT, (len - size) >> CLPoliciesManager::NVM_PAGE_SHIFT);
			m_intervalTree.insert(node);
		}
		else
			m_treeNodeAllocator->freeObj(node);

		return true;
	}
	else
		return false;
}

template<typename TTestClass>
bool _CLNVMLargeObjAllocator<TTestClass>::alloc(size_t size, unsigned long newSegmentOffset, unsigned long & offset)
{
	CLIntervalTreeNode * node = (CLIntervalTreeNode *)m_treeNodeAllocator->allocObj();
	if (!node)
		return false;

	offset = newSegmentOffset;
	new (node) CLIntervalTreeNode((newSegmentOffset + size) >> CLPoliciesManager::NVM_PAGE_SHIFT, (CLPoliciesManager::NVM_SEGMENT_SIZE - size) >> CLPoliciesManager::NVM_PAGE_SHIFT);
	m_intervalTree.insert(node);

	return true;
}

/*
template<typename TTestClass>
bool _CLNVMLargeObjAllocator<TTestClass>::free(void * obj, size_t objSize, void **segmentToReturn)
{
	unsigned long curSegment = (unsigned long)obj & ~(CLPoliciesManager::NVM_SEGMENT_SIZE - 1);
	unsigned long newHead = (unsigned long)obj, newTail = (unsigned long)obj + objSize;
	unsigned long prevExtentTail, nextExtentHead;
	unsigned long prevExtentLen, nextExtentLen;
	CLExtent prevExtent, nextExtent;
	CLIntervalTreeNode *prevNode = NULL, *nextNode = NULL;
	CLIntervalTreeNode *newNode;

	auto iter = m_intervalTrees.find(curSegment);
	if (__glibc_unlikely(iter == m_intervalTrees.end()))
		return false;

	prevExtentTail = (unsigned long)obj - (1ul << CLPoliciesManager::NVM_PAGE_SHIFT);
	nextExtentHead = (unsigned long)obj + objSize;

	if((prevExtentTail & ~(CLPoliciesManager::NVM_SEGMENT_SIZE - 1)) == curSegment)
		prevExtent = m_extentFinder->getExtent((void *)prevExtentTail);
	if((nextExtentHead & ~(CLPoliciesManager::NVM_SEGMENT_SIZE - 1)) == curSegment)
		nextExtent = m_extentFinder->getExtent((void *)nextExtentHead);

	if (prevExtent.isValid() && prevExtent.isFree())
	{
		prevExtentLen = prevExtent.getLen();
		newHead = newHead - prevExtentLen;
		prevNode = iter->second.getAndRemove(prevExtent.getLen() >> CLPoliciesManager::NVM_PAGE_SHIFT, (newHead - curSegment) >> CLPoliciesManager::NVM_PAGE_SHIFT);
		m_extentFinder->removeExtentOfFreeSpace((void *)newHead, prevExtentLen);
	}

	if (nextExtent.isValid() && nextExtent.isFree())
	{
		nextExtentLen = nextExtent.getLen();
		newTail = newTail + nextExtentLen;
		nextNode = iter->second.getAndRemove(nextExtentLen >> CLPoliciesManager::NVM_PAGE_SHIFT, (nextExtentHead - curSegment) >> CLPoliciesManager::NVM_PAGE_SHIFT);
		m_extentFinder->removeExtentOfFreeSpace((void *)nextExtentHead, nextExtentLen);
	}

	if (prevNode)
	{
		newNode = prevNode;

		if (nextNode)
			m_treeNodeAllocator->freeObj(nextNode);
	}
	else if (nextNode)
		newNode = nextNode;
	else
	{
		newNode = (CLIntervalTreeNode *)m_treeNodeAllocator->allocObj();
		if (!newNode)
			return false;
	}

	if (newTail - newHead < CLPoliciesManager::NVM_SEGMENT_SIZE)
	{
		newNode->setInterval((newHead - curSegment) >> CLPoliciesManager::NVM_PAGE_SHIFT, (newTail - newHead) >> CLPoliciesManager::NVM_PAGE_SHIFT);
		iter->second.insert(newNode);
		m_extentFinder->insertExtentOfFreeSpace((void *)newHead, newTail - newHead);
		*segmentToReturn = NULL;
	}
	else
	{
		m_treeNodeAllocator->freeObj(newNode);
		m_intervalTrees.erase(iter);
		m_extentFinder->removeSegment((void *)curSegment);
		*segmentToReturn = (void *)curSegment;
	}

	return true;
}
*/

template<typename TTestClass>
bool _CLNVMLargeObjAllocator<TTestClass>::free(bool mergePrev, bool mergeNext, unsigned long prevExtent, unsigned long obj, unsigned long nextExtent, size_t objSize)
{
	unsigned long newHead = mergePrev ? prevExtent : obj, newTail = mergeNext ? nextExtent + objSize : obj + objSize;
	CLIntervalTreeNode *prevNode = NULL, *nextNode = NULL;
	CLIntervalTreeNode *newNode;

	if (mergePrev)
	{
		prevNode = m_intervalTree.getAndRemove((obj - newHead) >> CLPoliciesManager::NVM_PAGE_SHIFT, newHead >> CLPoliciesManager::NVM_PAGE_SHIFT);
		if (!prevNode)
			return false;
	}

	if (mergeNext)
	{
		nextNode = m_intervalTree.getAndRemove(objSize >> CLPoliciesManager::NVM_PAGE_SHIFT, nextExtent >> CLPoliciesManager::NVM_PAGE_SHIFT);
		if (!nextNode)
			return false;
	}

	if (prevNode)
	{
		newNode = prevNode;

		if (nextNode)
			m_treeNodeAllocator->freeObj(nextNode);
	}
	else if (nextNode)
		newNode = nextNode;
	else
	{
		newNode = (CLIntervalTreeNode *)m_treeNodeAllocator->allocObj();
		if (!newNode)
			return false;
	}

	if (newTail - newHead < CLPoliciesManager::NVM_SEGMENT_SIZE)
	{
		newNode->setInterval(newHead >> CLPoliciesManager::NVM_PAGE_SHIFT, (newTail - newHead) >> CLPoliciesManager::NVM_PAGE_SHIFT);
		m_intervalTree.insert(newNode);
	}
	else
		m_treeNodeAllocator->freeObj(newNode);

	return true;
}

template class _CLNVMLargeObjAllocator<void>;

class CLTestNVMLargeObjAllocator;
template class _CLNVMLargeObjAllocator<CLTestNVMLargeObjAllocator>;