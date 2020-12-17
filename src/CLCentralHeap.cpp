#include "CLCentralHeap.h"
#include <cstdlib>

template<typename TTestClass>
void * _CLCentralHeap<TTestClass>::AllocSegment(unsigned threadHeapID)
{
	void * res = nullptr;
	Segment *freeSegment;

	freeSegment = (Segment *)m_freeSegments.pop();
	if (freeSegment) {
		unsigned index = freeSegment - m_segments;
		if (!m_segments[index].m_extentFinder.init()) {
			m_freeSegments.push(&m_segments[index].m_freeListNode);
			return nullptr;
		}
		res = (void *)(m_addrSpaceStart + ((unsigned long)index << CLPoliciesManager::NVM_SEGMENT_SHIFT));
		freeSegment->m_threadHeapID = threadHeapID;
	}

	return res;
}

template<typename TTestClass>
void _CLCentralHeap<TTestClass>::FreeSegment(void * addr)
{
	unsigned index = ((unsigned long)addr - m_addrSpaceStart) >> CLPoliciesManager::NVM_SEGMENT_SHIFT;
	if (index >= m_segmentNum)
		return;
	
	m_segments[index].m_extentFinder.destroy();
	m_freeSegments.push(&m_segments[index].m_freeListNode);
}

template<typename TTestClass>
unsigned _CLCentralHeap<TTestClass>::GetThreadHeapID(void * addr, CLNVMExtentFinder & extentFinder)
{
	unsigned long segmentAlignedAddr = (unsigned long)addr & ~(CLPoliciesManager::NVM_SEGMENT_SIZE - 1);
	unsigned index = ((unsigned long)segmentAlignedAddr - m_addrSpaceStart) >> CLPoliciesManager::NVM_SEGMENT_SHIFT;

	if (index >= m_segmentNum)
		return 0;

	extentFinder = m_segments[index].m_extentFinder;
	return m_segments[index].m_threadHeapID;
}

template<typename TTestClass>
unsigned _CLCentralHeap<TTestClass>::GetSegmentID(void* addr)
{
	unsigned long segmentAlignedAddr = (unsigned long)addr & ~(CLPoliciesManager::NVM_SEGMENT_SIZE - 1);
	unsigned index = ((unsigned long)segmentAlignedAddr - m_addrSpaceStart) >> CLPoliciesManager::NVM_SEGMENT_SHIFT;

	return index;
}

template<typename TTestClass>
void * _CLCentralHeap<TTestClass>::allocSegment(unsigned threadHeapID)
{
	if (m_instance)
		return m_instance->AllocSegment(threadHeapID);
	return NULL;
}

template<typename TTestClass>
void _CLCentralHeap<TTestClass>::freeSegment(void * addr)
{
	if (m_instance)
		m_instance->FreeSegment(addr);
}

template<typename TTestClass>
unsigned _CLCentralHeap<TTestClass>::getThreadHeapID(void * addr, CLNVMExtentFinder & extentFinder)
{
	if (m_instance)
		return m_instance->GetThreadHeapID(addr, extentFinder);
	return 0;
}

template<typename TTestClass>
unsigned long _CLCentralHeap<TTestClass>::getNVMAddrSpaceStart()
{
	if (m_instance)
		return m_instance->m_addrSpaceStart;
	return 0;
}

template<typename TTestClass>
unsigned _CLCentralHeap<TTestClass>::getSegmentID(void* addr)
{
	if (m_instance)
		return m_instance->GetSegmentID(addr);
	return INVALID_SEGMENT_ID;
}

template<typename TTestClass>
_CLCentralHeap<TTestClass> * _CLCentralHeap<TTestClass>::m_instance = NULL;

template class _CLCentralHeap<void>;

class CLTestCentralHeap;
template class _CLCentralHeap<CLTestCentralHeap>;