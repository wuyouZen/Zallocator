#pragma once
#include "CLFreeList.h"
#include "CLNVMExtentFinder.h"
#include "CLPoliciesManager.h"
#include <mutex>

const unsigned INVALID_SEGMENT_ID = 0xffffffff;
extern const unsigned SEGMENT_FREE;

template<typename TTestClass>
class _CLCentralHeap
{
	friend TTestClass;

	typedef CLFreeList FreeSegmentList;
	
	struct Segment {
		STFreeListNode m_freeListNode;
		unsigned m_threadHeapID;
		CLNVMExtentFinder m_extentFinder;
	};

	FreeSegmentList m_freeSegments;
	Segment *m_segments;
	unsigned m_segmentNum;
	unsigned long m_addrSpaceStart;

	template<typename TIterator>
	_CLCentralHeap(void* NVMAddrSpaceStart, unsigned long size, TIterator iter):
		m_segments(NULL),
		m_segmentNum(0),
		m_addrSpaceStart((unsigned long)NVMAddrSpaceStart)
	{
		m_segmentNum = size >> CLPoliciesManager::NVM_SEGMENT_SHIFT;
		if (!m_segmentNum)
			return;

		m_segments = (Segment*)malloc(sizeof(Segment) * m_segmentNum);
		if (!m_segments)
			return;

		for (int i = 0; i < m_segmentNum; ++i, ++iter) {
			if (iter.reachEnd()) {
				delete m_segments;
				m_segments = nullptr;
				return;
			}

			m_segments[i].m_threadHeapID = iter.getThreadHeapID();
			if (m_segments[i].m_threadHeapID != SEGMENT_FREE) {
				if (!m_segments[i].m_extentFinder.init()) {
					delete m_segments;
					m_segments = nullptr;
					return;
				}
			}
			else
				m_freeSegments.push(&m_segments[i].m_freeListNode);
		}

		return;
	}

	_CLCentralHeap(const _CLCentralHeap &) = delete;
	_CLCentralHeap & operator=(const _CLCentralHeap &) = delete;

	void * AllocSegment(unsigned threadHeapID);
	void FreeSegment(void * addr);
	unsigned GetThreadHeapID(void * addr, CLNVMExtentFinder & extentFinder);
	unsigned GetSegmentID(void* addr);

	static _CLCentralHeap * m_instance;

public:
	template<typename TIterator>
	static bool init(void* NVMAddrSpaceStart, unsigned long size, TIterator segmentInfoIter)
	{
		if (m_instance)
			return true;
		else
		{
			m_instance = new _CLCentralHeap<TTestClass>(NVMAddrSpaceStart, size, segmentInfoIter);
			if (!m_instance->m_segments)
			{
				delete m_instance;
				m_instance = NULL;
				return false;
			}
			return true;
		}
	}

	static void * allocSegment(unsigned threadHeapID);
	static void freeSegment(void * addr);
	static unsigned getThreadHeapID(void * addr, CLNVMExtentFinder & extentFinder);
	static unsigned long getNVMAddrSpaceStart();
	static unsigned getSegmentID(void* addr);
};

typedef _CLCentralHeap<void> CLCentralHeap;