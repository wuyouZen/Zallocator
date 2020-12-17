#pragma once
#include "Utils.h"

struct STNVMSegment
{
	unsigned ownerThreadHeapID;
};

struct STNVMHeader
{
	unsigned segmentNum;
	STNVMSegment segments[1];
};

extern const unsigned SEGMENT_FREE;

template<typename TTestClass>
class _CLThreadHeapInfoRecorder
{
	friend TTestClass;

	STNVMHeader* m_NVMHeader;

	static _CLThreadHeapInfoRecorder* m_instance;

	_CLThreadHeapInfoRecorder(void* NVMAddrSpaceStart) {
		m_NVMHeader = (STNVMHeader*)NVMAddrSpaceStart;
	}

	void _markAllocated(int threadHeapID, unsigned segment) {
		if (segment >= m_NVMHeader->segmentNum)
			return;
		m_NVMHeader->segments[segment].ownerThreadHeapID = threadHeapID;
		clflush(&(m_NVMHeader->segments[segment].ownerThreadHeapID));
	}

	void _markFree(unsigned segment) {
		if (segment >= m_NVMHeader->segmentNum)
			return;
		m_NVMHeader->segments[segment].ownerThreadHeapID = SEGMENT_FREE;
		clflush(&(m_NVMHeader->segments[segment].ownerThreadHeapID));
	}

	unsigned long _getHeaderSize() {
		return ((unsigned long)((m_NVMHeader->segmentNum + 1) * sizeof(int)) + 7ul) & ~7ul;
	}

public:
	class CLNVMSegmentIter
	{
		unsigned m_curSegment{ 0 };
		STNVMHeader* m_NVMHeader;

	public:
		CLNVMSegmentIter(STNVMHeader* header = nullptr):m_NVMHeader(header){}

		CLNVMSegmentIter& operator++() {
			if (m_curSegment < m_NVMHeader->segmentNum)
				m_curSegment++;
			return *this;
		}

		CLNVMSegmentIter operator++(int) {
			CLNVMSegmentIter res = *this;
			++(*this);
			return res;
		}

		int getThreadHeapID() {
			return m_NVMHeader->segments[m_curSegment].ownerThreadHeapID;
		}

		bool reachEnd() {
			if(m_NVMHeader)
				return m_curSegment == m_NVMHeader->segmentNum;
			return true;
		}
	};

private:
	CLNVMSegmentIter _begin() {
		return CLNVMSegmentIter(m_NVMHeader);
	}

public:
	static bool init(void* NVMAddrSpaceStart) {
		if (!m_instance)
			m_instance = new _CLThreadHeapInfoRecorder(NVMAddrSpaceStart);
		return m_instance;
	}

	static void markAllocated(int threadHeapID, unsigned segment) {
		if (m_instance)
			m_instance->_markAllocated(threadHeapID, segment);
	}

	static void markFree(unsigned segment) {
		if (m_instance)
			m_instance->_markFree(segment);
	}

	static CLNVMSegmentIter begin() {
		if (m_instance)
			return m_instance->_begin();
		return CLNVMSegmentIter();
	}

	static unsigned long getHeaderSize() {
		if (m_instance)
			return m_instance->_getHeaderSize();
		return 0;
	}
};

typedef _CLThreadHeapInfoRecorder<void> CLThreadHeapInfoRecorder;