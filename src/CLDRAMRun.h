#pragma once
#include "CLFreeList.h"
#include "Utils.h"
#include "CLPoliciesManager.h"

template<typename TTestClass>
class _CLDRAMRun
{
	friend TTestClass;

	CLFreeList m_freeChunks;
	unsigned int m_tail, m_objSize, m_freeChunkCount, m_maxFreeChunkNum;

public:
	_CLDRAMRun() {}

	_CLDRAMRun(unsigned int objSize) :
		m_tail(CACHE_LINE_SIZE),
		m_objSize(objSize)
	{
		m_maxFreeChunkNum = m_freeChunkCount = (CLPoliciesManager::DRAM_RUN_SIZE - CACHE_LINE_SIZE) / m_objSize;
	}
	
	void * allocObj() {
		unsigned long res;

		if (m_tail + m_objSize <= CLPoliciesManager::DRAM_RUN_SIZE)
		{
			res = ((unsigned long)this & ~(CLPoliciesManager::DRAM_RUN_SIZE - 1)) + m_tail;
			m_tail += m_objSize;
		}
		else
			res = (unsigned long)m_freeChunks.pop();

		if (__glibc_likely(res))
			m_freeChunkCount--;

		return (void *)res;
	}
	
	void freeObj(void *obj) {
		m_freeChunks.push((STFreeListNode *)obj);
		m_freeChunkCount++;
		return;
	}
	
	bool full() {
		return m_freeChunkCount == 0;
	}
	
	bool empty() {
		return m_freeChunkCount == m_maxFreeChunkNum;
	}

	void *getRunAddr() {
		return (void *)((unsigned long)this & ~(CLPoliciesManager::DRAM_RUN_SIZE - 1));
	}
};

typedef _CLDRAMRun<void> CLDRAMRun;