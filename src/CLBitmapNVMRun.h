#pragma once
#include "CLBitmap.h"
#include "CLRunCache.h"
#include "Utils.h"
#include <cstdlib>
#include <new>

template<typename TAllocator, typename TTestClass>
class _CLBitmapNVMRun
{
	friend TTestClass;

	unsigned long m_pBitmap;
	unsigned long m_NVMRunAddr;

	const static unsigned FREE_COUNT_SHIFT = 48u;
	const static unsigned long PTR_MASK = 0x0000ffffffffffc0ul;
	const static unsigned long LAST_ACCESS_POS_MASK = 0x1f;
	const static unsigned long NVM_PTR_MASK		= 0x0000ffffffff0000ul;
	const static unsigned long OTHER_INFO_MASK	= 0x000000000000fffful;

	inline void incFreeCount(unsigned long *p)
	{
		*p += (1ul << FREE_COUNT_SHIFT);
	}

	inline void decFreeCount(unsigned long *p)
	{
		*p -= (1ul << FREE_COUNT_SHIFT);
	}

	inline void setFreeCount(unsigned long *p, unsigned long count)
	{
		*p = (count << FREE_COUNT_SHIFT) | (*p & (PTR_MASK | LAST_ACCESS_POS_MASK));
	}

	inline void setLastAccessPos(unsigned long *p, unsigned pos)
	{
		*p = (*p & ~LAST_ACCESS_POS_MASK) | (pos & LAST_ACCESS_POS_MASK);
	}

public:
	_CLBitmapNVMRun() :m_pBitmap(NULL) {}

	_CLBitmapNVMRun(unsigned int objSize, unsigned int runSize, unsigned long NVMRunAddr, TAllocator *allocator) :
		m_NVMRunAddr(NVMRunAddr)
	{
		CLBitMap *pBitmap = (CLBitMap *)allocator->allocObj();
		new (pBitmap) CLBitMap(objSize, runSize);
		unsigned long maxFreeCount = runSize / objSize;
		if (pBitmap)
		{
			m_pBitmap = (unsigned long)pBitmap;
			setFreeCount(&m_pBitmap, maxFreeCount);
			setLastAccessPos(&m_pBitmap, 31);

			m_NVMRunAddr |= (maxFreeCount << FREE_COUNT_SHIFT);
		}
	}

	void * allocObj() {
		CLBitMap *pBitmap = (CLBitMap*)(m_pBitmap & PTR_MASK);
		int index = pBitmap->getAndSet(m_pBitmap & LAST_ACCESS_POS_MASK);
		if (index != -1)
		{
			setLastAccessPos(&m_pBitmap, index >> 6);
			decFreeCount(&m_pBitmap);
			return (void *)((m_NVMRunAddr & NVM_PTR_MASK) + (index << 6));
		}
		else
			return NULL;
	}

	void allocAt(void* addr) {
		CLBitMap* pBitmap = (CLBitMap*)(m_pBitmap & PTR_MASK);
		unsigned long index = ((unsigned long)addr - (m_NVMRunAddr & NVM_PTR_MASK)) >> 6;
		pBitmap->setBit(index);

		decFreeCount(&m_pBitmap);

		return;
	}
	
	void freeObj(void *obj) {
		CLBitMap *pBitmap = (CLBitMap*)(m_pBitmap & PTR_MASK);
		unsigned long index = ((unsigned long)obj - (m_NVMRunAddr & NVM_PTR_MASK)) >> 6;
		pBitmap->clearBit(index);

		incFreeCount(&m_pBitmap);

		return;
	}

	void *getRunAddr() {
		return (void *)(m_NVMRunAddr & NVM_PTR_MASK);
	}

	void setOtherInfo(uint16_t info) {
		m_NVMRunAddr |= info;
	}

	uint16_t getOtherInfo() {
		return (uint16_t)(m_NVMRunAddr & OTHER_INFO_MASK);
	}
	
	bool empty() {
		return m_pBitmap >> FREE_COUNT_SHIFT == m_NVMRunAddr >> FREE_COUNT_SHIFT;
	}
	
	bool full() {
		return (m_pBitmap >> FREE_COUNT_SHIFT) == 0;
	}
	
	bool doneInit() {
		return m_pBitmap;
	}

	void release(TAllocator *allocator) {
		allocator->freeObj((void *)(m_pBitmap & PTR_MASK));
	}
};

class CLDRAMSlab;

typedef _CLBitmapNVMRun<CLDRAMSlab, void> CLBitmapNVMRun;