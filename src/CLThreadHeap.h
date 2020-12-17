#pragma once
#include "CLSlab.h"
#include "CLNVMLargeObjAllocator.h"
#include "CLCentralHeap.h"
#include "CLThreadHeapInfoRecorder.h"
#include "CLNVMExtentFinder.h"
#include "CLChannelsManager.h"
#include "CLPoliciesManager.h"
#include "CLObjCache.h"
#include "CLAllocatedSpaceTracer.h"
#include "CLTypeInfoReader.h"

struct STSmallObjAllocContext
{
	CLNVMSlab m_slab;
	CLObjCache m_objCache;
};

class CLThreadHeap
{
	static CLChannelsManager m_channelsManagers[CLPoliciesManager::MAX_THREAD_NUM];

	STSmallObjAllocContext m_smallObjAllocContexts[CLPoliciesManager::SMALL_OBJ_CATEORY_NUM];
	CLNVMLargeObjAllocator m_largeObjAllocator;

	CLDRAMSlab m_listNodeSlab;
	CLDRAMSlab m_bitmapSlab;
	CLDRAMSlab m_treeNodeSlab;
	CLRunManageStructCreator<CLBitmapNVMRun, STDoubleLinkListNode<CLBitmapNVMRun>> m_runManageStructCreator;

	CLNVMRunCache * m_NVMRunCache;

	unsigned m_thearHeapID;
	unsigned m_mallocCount;

	CLAllocatedSpaceTracer* m_tracer;
	CLTypeInfoReader* m_typeInfoReader;

	bool m_doneInit;

	bool doChannelClean();
	void * allocFromSlab(size_t objSize);
	void * allocFromLargeObjAllocator(size_t objSize);
	bool localFree(void * obj, CLNVMExtentFinder extentFinder);
	void localAllocAt(STPtrEntry obj, CLNVMExtentFinder extentFinder);
	bool tryMergeAndFree(void * extentAddr, unsigned long extentLen);

public:
	CLThreadHeap(unsigned threadHeapID);
	void * alloc(size_t objSize);
	void allocAt(STPtrEntry obj);
	void doRecoverChannelClean();
	bool beginRecover();
	bool endRecover();
	bool free(void * obj);
	bool doneInit();
};