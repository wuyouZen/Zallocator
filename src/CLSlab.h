#pragma once
#include "CLRunCache.h"
#include "CLDoubleLinkList.h"
#include "CLRunManageStructCreator.h"
#include "CLDRAMRun.h"
#include "CLBitmapNVMRun.h"
#include "CLExtent.h"
#include "Utils.h"
#include <jemalloc/jemalloc.h>

template<typename TRun, typename TTestClass>
class _CLSlab
{
	friend TTestClass;

	CLDoubleLinkList<TRun> m_halfEmptyRuns;
	CLDoubleLinkList<TRun> m_fullRuns;
	CLRunManageStructCreator<TRun, STDoubleLinkListNode<TRun>> *m_listNodeCreator;
	unsigned m_objSize;
	unsigned m_runSize;

	_CLSlab(const _CLSlab &) = delete;
	_CLSlab & operator=(const _CLSlab &) = delete;

public:
	_CLSlab() {}

	_CLSlab(unsigned objSize, unsigned runSize,
		CLRunManageStructCreator<TRun, STDoubleLinkListNode<TRun>> *listNodeCreator) :
		m_objSize(objSize),
		m_runSize(runSize),
		m_listNodeCreator(listNodeCreator){}

	void * allocObj(void *newRun, CLExtent & extent);
	void * allocObj();
	void allocAt(void *addr, CLExtent & extent);
	void * freeObj(void *obj, void *runManageStruct);

	unsigned getRunSize() {
		return m_runSize;
	}
};

template<typename TRun, typename TTestClass>
inline void * _CLSlab<TRun, TTestClass>::allocObj()
{
	STDoubleLinkListNode<TRun> *cur = m_halfEmptyRuns.getHead();
	STDoubleLinkListNode<TRun> *nodeToTrans;

	void *res;
	while (cur)
	{
		res = cur->val.allocObj();
		if (__glibc_unlikely(cur->val.full()))
		{
			nodeToTrans = cur;
			cur = cur->m_next;
			m_halfEmptyRuns.erase(nodeToTrans);
			m_fullRuns.push(nodeToTrans);
		}

		if(res)
			return res;
	}

	return NULL;
}

template<typename TRun, typename TTestClass>
inline void _CLSlab<TRun, TTestClass>::allocAt(void *addr, CLExtent & extent)
{
	STDoubleLinkListNode<TRun>* newNode = nullptr;
	void* newRun = (void*)((unsigned long)addr & ~((unsigned long)m_runSize - 1));
	newNode = m_listNodeCreator->create(m_objSize, m_runSize, newRun);
	newNode->val.allocAt(addr);
	m_halfEmptyRuns.push(newNode);
	extent = CLExtent((void*)newNode);
}

template<typename TRun, typename TTestClass>
inline void * _CLSlab<TRun, TTestClass>::freeObj(void *obj, void *runManageStruct)
{
	STDoubleLinkListNode<TRun> *node = (STDoubleLinkListNode<TRun> *)runManageStruct;
	void *runAddr = node->val.getRunAddr();
	bool isFull = node->val.full();

	node->val.freeObj(obj);
	if (isFull)
	{
		m_fullRuns.erase(node);
		m_halfEmptyRuns.push(node);
	}
	else if (node->val.empty())
	{
		m_halfEmptyRuns.erase(node);
		m_listNodeCreator->destory(node);
		
		return runAddr;
	}

	return NULL;
}

template<typename TRun, typename TTestClass>
inline void * _CLSlab<TRun, TTestClass>::allocObj(void * newRun, CLExtent & extent)
{
	void *res;
	STDoubleLinkListNode<TRun> *newNode;

	newNode = m_listNodeCreator->create(m_objSize, m_runSize, newRun);
	if (__glibc_unlikely(newNode == NULL))
		return NULL;
	res = newNode->val.allocObj();

	m_halfEmptyRuns.push(newNode);

	extent = CLExtent((void *)newNode);
	return res;
}

template<typename TRun>
using CLSlab = _CLSlab<TRun, void>;

class CLDRAMSlab
{
	CLSlab<CLDRAMRun> m_slab;
	CLRunManageStructCreator<CLDRAMRun, STDoubleLinkListNode<CLDRAMRun>> m_listNodeCreator;

public:
	CLDRAMSlab(unsigned objSize) :
		m_slab(objSize, CLPoliciesManager::DRAM_RUN_SIZE, &m_listNodeCreator) {}

	void * allocObj() {
		void * res = m_slab.allocObj();
		void * newRun;
		CLExtent extent;

		if (!res)
		{
			newRun = mallocx(CLPoliciesManager::DRAM_RUN_SIZE, MALLOCX_ALIGN(CLPoliciesManager::DRAM_RUN_SIZE));
			
			if (newRun)
			{
				res = m_slab.allocObj(newRun, extent);
				if (__glibc_unlikely(!res))
					free(newRun);
			}
		}

		return res;
	}

	void freeObj(void * addr)
	{
		void * runManager = (void *)((unsigned long)addr & ~(CLPoliciesManager::DRAM_RUN_SIZE - 1));
		m_slab.freeObj(addr, runManager);
	}
};

template<>
class CLRunManageStructCreator<CLBitmapNVMRun, STDoubleLinkListNode<CLBitmapNVMRun>>
{
	CLDRAMSlab *m_listNodeAllocator;
	CLDRAMSlab *m_bitmapAllocator;

public:
	CLRunManageStructCreator(CLDRAMSlab *listNodeAllocator, CLDRAMSlab *bitmapAllocator) :
		m_listNodeAllocator(listNodeAllocator),
		m_bitmapAllocator(bitmapAllocator) {}

	STDoubleLinkListNode<CLBitmapNVMRun> * create(unsigned objSize, unsigned runSize, void *runAddr) {
		STDoubleLinkListNode<CLBitmapNVMRun> *res = (STDoubleLinkListNode<CLBitmapNVMRun> *)m_listNodeAllocator->allocObj();
		if (__glibc_unlikely(!res))
			return NULL;
		new (&res->val) CLBitmapNVMRun(objSize, runSize, (unsigned long)runAddr, m_bitmapAllocator);
		return res;
	}

	void destory(STDoubleLinkListNode<CLBitmapNVMRun> *manageStruct) {
		manageStruct->val.release(m_bitmapAllocator);
		m_listNodeAllocator->freeObj(manageStruct);
	}
};

typedef CLSlab<CLBitmapNVMRun> CLNVMSlab;