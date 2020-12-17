#include "CLThreadHeap.h"
#include <new>

CLChannelsManager CLThreadHeap::m_channelsManagers[CLPoliciesManager::MAX_THREAD_NUM];

bool CLThreadHeap::doChannelClean()
{
	CLNVMExtentFinder extentFinder;
	void * obj = (void *)m_channelsManagers[m_thearHeapID - 1].pop();
	while (obj)
	{
		CLCentralHeap::getThreadHeapID(obj, extentFinder);

		if (localFree(obj, extentFinder))
		{
			obj = (void *)m_channelsManagers[m_thearHeapID - 1].pop();
			continue;
		}
		else
			return false;
	}

	return true;
}

void CLThreadHeap::doRecoverChannelClean()
{
	CLNVMExtentFinder extentFinder;
	STPtrEntry obj;
	*(unsigned long*)&obj = m_channelsManagers[m_thearHeapID - 1].pop();
	while (obj.address)
	{
		CLCentralHeap::getThreadHeapID((void*)obj.address, extentFinder);
		localAllocAt(obj, extentFinder);
		*(unsigned long*)&obj = m_channelsManagers[m_thearHeapID - 1].pop();
	}
}

void * CLThreadHeap::allocFromSlab(size_t objSize)
{
	unsigned typeIndex, runSize;
	void * res;
	void * newRun;
	CLExtent runExtent;
	bool fromCache = true;

	typeIndex = CLPoliciesManager::getTypeIndexBySize(objSize);
	runSize = m_smallObjAllocContexts[typeIndex].m_slab.getRunSize();

	res = m_smallObjAllocContexts[typeIndex].m_objCache.get();
	if (res)
		return res;

	res = m_smallObjAllocContexts[typeIndex].m_slab.allocObj();
	if (res)
		return res;

	newRun = m_NVMRunCache->get();

	if (!newRun)
	{
		fromCache = false;

		newRun = allocFromLargeObjAllocator(runSize);
		if (!newRun)
			return NULL;
	}

	res = m_smallObjAllocContexts[typeIndex].m_slab.allocObj(newRun, runExtent);
	if (res)
	{
		CLNVMExtentFinder extentFinder;
		CLCentralHeap::getThreadHeapID(newRun, extentFinder);

		runExtent.setTypeIndex(typeIndex);

		if (extentFinder.insertExtentOfRun(runExtent, newRun, runSize))
			return res;
		else
			m_smallObjAllocContexts[typeIndex].m_slab.freeObj(res, runExtent.getRunManageStruct());
	}

	if (fromCache)
		m_NVMRunCache->insert(newRun);
	else
		tryMergeAndFree(newRun, runSize);
	return NULL;
}

void * CLThreadHeap::allocFromLargeObjAllocator(size_t alignedSize)
{
	unsigned long objOffsetInNVM, NVMAddrSpaceStart = CLCentralHeap::getNVMAddrSpaceStart();
	void *res, *newSegment;
	bool doneClean = false;
	CLNVMExtentFinder extentFinder;

	while (true)
	{
		if (m_largeObjAllocator.alloc(alignedSize, objOffsetInNVM))
		{
			res = (void *)(objOffsetInNVM + NVMAddrSpaceStart);
			CLCentralHeap::getThreadHeapID(res, extentFinder);
			extentFinder.removeExtentOfFreeSpace(res, alignedSize);
			return res;
		}

		if (!doneClean)
		{
			if (!doChannelClean())
				return NULL;
			doneClean = true;
			m_mallocCount = 0;
			continue;
		}
		else
			break;
	}

	newSegment = CLCentralHeap::allocSegment(m_thearHeapID);
	if (!newSegment)
		return NULL;

	CLCentralHeap::getThreadHeapID(newSegment, extentFinder);
	if (!extentFinder.insertExtentOfFreeSpace(newSegment, CLPoliciesManager::NVM_SEGMENT_SIZE))
	{
		CLCentralHeap::freeSegment(newSegment);
		return NULL;
	}

	CLThreadHeapInfoRecorder::markAllocated(m_thearHeapID, CLCentralHeap::getSegmentID(newSegment));

	if (m_largeObjAllocator.alloc(alignedSize, (unsigned long)newSegment - NVMAddrSpaceStart, objOffsetInNVM))
	{
		res = (void *)(objOffsetInNVM + NVMAddrSpaceStart);
		extentFinder.removeExtentOfFreeSpace(res, alignedSize);
		return res;
	}

	extentFinder.removeExtentOfFreeSpace(newSegment, CLPoliciesManager::NVM_SEGMENT_SIZE);
	CLCentralHeap::freeSegment(newSegment);
	return NULL;
}

bool CLThreadHeap::localFree(void * obj, CLNVMExtentFinder extentFinder)
{
	unsigned long pageAlignedAddr = (unsigned long)obj & ~(CLPoliciesManager::NVM_PAGE_SIZE - 1);
	CLExtent extent = extentFinder.getExtent((void *)pageAlignedAddr);

	if (!extent.isValid())
		return false;

	if (extent.isFree())
		return false;

	if (extent.isLargeObj())
	{
		extentFinder.removeExtentOfLargeObj((void *)pageAlignedAddr, extent.getLen());
		return tryMergeAndFree((void *)pageAlignedAddr, extent.getLen());
	}
	else
	{
		STDoubleLinkListNode<CLBitmapNVMRun> * runManageStruct = (STDoubleLinkListNode<CLBitmapNVMRun> *)extent.getRunManageStruct();
		unsigned typeIndex = extent.getTypeIndex();

		if (m_smallObjAllocContexts[typeIndex].m_objCache.insert(obj))
			return true;

		void * freeRun = m_smallObjAllocContexts[typeIndex].m_slab.freeObj(obj, runManageStruct);
		if (!freeRun)
			return true;
		else
		{
			if (m_NVMRunCache->insert(freeRun))
				return true;
			extentFinder.removeExtentOfRun(freeRun, m_smallObjAllocContexts[typeIndex].m_slab.getRunSize());
			return tryMergeAndFree(freeRun, m_smallObjAllocContexts[typeIndex].m_slab.getRunSize());
		}
	}
}

void CLThreadHeap::localAllocAt(STPtrEntry obj, CLNVMExtentFinder extentFinder)
{
	void* addr = (void*)obj.address;
	unsigned long pageAlignedAddr = (unsigned long)addr & ~(CLPoliciesManager::NVM_PAGE_SIZE - 1);
	CLExtent extent = extentFinder.getExtent((void*)pageAlignedAddr);

	if (extent.isValid()) 
	{
		if (!extent.isLargeObj())
		{
			STDoubleLinkListNode<CLBitmapNVMRun>* runManageStruct = (STDoubleLinkListNode<CLBitmapNVMRun>*)extent.getRunManageStruct();
			runManageStruct->val.allocAt(addr);
			return;
		}
		else
			return;
	}
	else
	{
		unsigned long objSize = m_typeInfoReader->getTypeInfoByID(obj.typeID)->objectSize;
		if (objSize <= CLPoliciesManager::MAX_SMALL_OBJ_SIZE)
		{
			CLExtent newExtent;
			unsigned sizeTypeIndex = CLPoliciesManager::getTypeIndexBySize(objSize);
			m_smallObjAllocContexts[sizeTypeIndex].m_slab.allocAt(addr, newExtent);

			newExtent.setTypeIndex(sizeTypeIndex);

			unsigned long runSize = m_smallObjAllocContexts[sizeTypeIndex].m_slab.getRunSize();
			unsigned long runAddr = (unsigned long)addr & ~(runSize - 1);

			m_tracer->trace(runSize, (void*)runAddr);
			extentFinder.insertExtentOfRun(newExtent, (void*)runAddr, runSize);
		}
		else
		{
			objSize = (objSize + CLPoliciesManager::NVM_PAGE_SIZE - 1) & ~(CLPoliciesManager::NVM_PAGE_SIZE - 1);
			m_tracer->trace(objSize, addr);
			extentFinder.insertExtentOfLargeObj(addr, objSize);
		}
	}
}

bool CLThreadHeap::tryMergeAndFree(void * extentAddr, unsigned long extentLen)
{
	unsigned long curSegment = (unsigned long)extentAddr & ~(CLPoliciesManager::NVM_SEGMENT_SIZE - 1);
	unsigned long prevExtentTail = (unsigned long)extentAddr - CLPoliciesManager::NVM_PAGE_SIZE;
	unsigned long nextExtentHead = (unsigned long)extentAddr + extentLen;
	unsigned long prevExtentHead, NVMAddrSpaceStart = CLCentralHeap::getNVMAddrSpaceStart();
	CLNVMExtentFinder extentFinder;
	CLExtent prevExtent, nextExtent;
	bool mergePrev = false, mergeNext = false;
	size_t objSize = extentLen;

	CLCentralHeap::getThreadHeapID((void *)curSegment, extentFinder);

	if ((prevExtentTail & ~(CLPoliciesManager::NVM_SEGMENT_SIZE - 1)) == curSegment)
		prevExtent = extentFinder.getExtent((void *)prevExtentTail);
	if ((nextExtentHead & ~(CLPoliciesManager::NVM_SEGMENT_SIZE - 1)) == curSegment)
		nextExtent = extentFinder.getExtent((void *)nextExtentHead);

	if (prevExtent.isValid() && prevExtent.isFree())
	{
		mergePrev = true;
		prevExtentHead = (unsigned long)extentAddr - prevExtent.getLen();
		extentFinder.removeExtentOfFreeSpace((void *)prevExtentHead, prevExtent.getLen());
	}

	if (nextExtent.isValid() && nextExtent.isFree())
	{
		mergeNext = true;
		objSize = nextExtent.getLen();
		extentFinder.removeExtentOfFreeSpace((void *)nextExtentHead, objSize);
	}

	if (m_largeObjAllocator.free(mergePrev, mergeNext, prevExtentHead - NVMAddrSpaceStart, (unsigned long)extentAddr - NVMAddrSpaceStart, nextExtentHead - NVMAddrSpaceStart, objSize))
	{
		unsigned long newHead, newTail;
		newHead = mergePrev ? prevExtentHead : (unsigned long)extentAddr;
		newTail = mergeNext ? nextExtentHead + nextExtent.getLen() : nextExtentHead;
		if (newTail - newHead == CLPoliciesManager::NVM_SEGMENT_SIZE)
		{
			CLCentralHeap::freeSegment((void *)newHead);
			return true;
		}
		else
			return extentFinder.insertExtentOfFreeSpace((void *)newHead, newTail - newHead);
	}

	return false;
}

CLThreadHeap::CLThreadHeap(unsigned threadHeapID):
	m_listNodeSlab(sizeof(STDoubleLinkListNode<CLBitmapNVMRun>)),
	m_treeNodeSlab(sizeof(CLIntervalTreeNode)),
	m_bitmapSlab(sizeof(CLBitMap)),
	m_runManageStructCreator(&m_listNodeSlab, &m_bitmapSlab),
	m_largeObjAllocator(&m_treeNodeSlab),
	m_thearHeapID(threadHeapID),
	m_mallocCount(0),
	m_tracer(nullptr),
	m_doneInit(false)
{
	m_typeInfoReader = CLTypeInfoReader::getInstance();

	m_NVMRunCache = new CLNVMRunCache;
	if (!m_NVMRunCache)
		return;

	for (unsigned i = 0; i < CLPoliciesManager::SMALL_OBJ_CATEORY_NUM; i++)
	{
		new (&m_smallObjAllocContexts[i].m_slab) CLNVMSlab(CLPoliciesManager::TYPE_TO_SIZE_MAP[i], CLPoliciesManager::NVM_RUN_SIZE, &m_runManageStructCreator);
		new (&m_smallObjAllocContexts[i].m_objCache) CLObjCache(CLPoliciesManager::TYPE_TO_SIZE_MAP[i], CLPoliciesManager::SMALL_OBJ_CACHE_SIZES[i]);
	}

	new (&m_channelsManagers[threadHeapID - 1]) CLChannelsManager(threadHeapID);

	if (!m_channelsManagers[threadHeapID - 1].doneInit())
		return;

	m_doneInit = true;
}

void * CLThreadHeap::alloc(size_t objSize)
{
	if (objSize == 0)
		return NULL;

	
	if (++m_mallocCount == CLPoliciesManager::CHANNEL_CLEAN_CIRCLE)
	{
		if (!doChannelClean())
			return NULL;
		m_mallocCount = 0;
	}

	if (objSize <= CLPoliciesManager::MAX_SMALL_OBJ_SIZE)
		return allocFromSlab(objSize);
	else if (objSize < CLPoliciesManager::NVM_SEGMENT_SIZE)
	{
		size_t alignedSize = (objSize + CLPoliciesManager::NVM_PAGE_SIZE - 1) & ~(CLPoliciesManager::NVM_PAGE_SIZE - 1);
		void * res = allocFromLargeObjAllocator(alignedSize);
		if (res)
		{
			CLNVMExtentFinder extentFinder;
			CLCentralHeap::getThreadHeapID(res, extentFinder);
			if (extentFinder.insertExtentOfLargeObj(res, alignedSize))
				return res;
			else
				tryMergeAndFree(res, alignedSize);
		}

		return NULL;
	}
	else
		return NULL;
}

void CLThreadHeap::allocAt(STPtrEntry obj)
{
	if (++m_mallocCount == CLPoliciesManager::CHANNEL_CLEAN_CIRCLE)
	{
		doRecoverChannelClean();
		m_mallocCount = 0;
	}

	void* addr = (void*)(obj.address);

	CLNVMExtentFinder extentFinder;
	unsigned targetThreadHeap = CLCentralHeap::getThreadHeapID(addr, extentFinder);

	if (targetThreadHeap) 
	{
		if (targetThreadHeap == m_thearHeapID)
			localAllocAt(obj, extentFinder);
		else
			m_channelsManagers[targetThreadHeap - 1].pushBack(*(unsigned long*)&obj, m_thearHeapID);
	}
}

bool CLThreadHeap::beginRecover()
{
	m_tracer = new CLAllocatedSpaceTracer;
	return m_tracer;
}

bool CLThreadHeap::endRecover()
{
	unsigned long prevEnd, nextStart;
	unsigned long maxAddr;
	CLNVMExtentFinder extentFinder;

	if (m_thearHeapID == CLPoliciesManager::FIRST_THREADHEAP)
	{
		m_tracer->trace(CLPoliciesManager::META_AREA_SIZE, (void*)(CLCentralHeap::getNVMAddrSpaceStart()));
		CLCentralHeap::getThreadHeapID((void*)CLCentralHeap::getNVMAddrSpaceStart(), extentFinder);
		extentFinder.insertExtentOfLargeObj((void*)CLCentralHeap::getNVMAddrSpaceStart(), CLPoliciesManager::META_AREA_SIZE);
	}

	auto allocatedSpaceIter = m_tracer->begin();
	auto lastAllocatedSpace = m_tracer->rbegin();
	auto endIter = m_tracer->end();

	if (allocatedSpaceIter != endIter)
	{
		prevEnd = allocatedSpaceIter->spaceStart & ~(CLPoliciesManager::NVM_SEGMENT_SIZE - 1);
		maxAddr = (lastAllocatedSpace->spaceStart + lastAllocatedSpace->spaceLen + CLPoliciesManager::NVM_SEGMENT_SIZE - 1) & ~(CLPoliciesManager::NVM_SEGMENT_SIZE - 1);

		while (allocatedSpaceIter != endIter)
		{
			nextStart = allocatedSpaceIter->spaceStart;

			if (prevEnd == nextStart)
			{
				prevEnd = nextStart + allocatedSpaceIter->spaceLen;
				++allocatedSpaceIter;
				continue;
			}

			unsigned long prevSegment = prevEnd & ~(CLPoliciesManager::NVM_SEGMENT_SIZE - 1);
			unsigned long nextSegment = nextStart & ~(CLPoliciesManager::NVM_SEGMENT_SIZE - 1);

			if (prevSegment == nextSegment)
			{
				CLCentralHeap::getThreadHeapID((void*)prevSegment, extentFinder);
				extentFinder.insertExtentOfFreeSpace((void*)prevEnd, nextStart - prevEnd);
				m_largeObjAllocator.free(false, false, 0, prevEnd, 0, nextStart - prevEnd);
			}
			else
			{
				CLCentralHeap::getThreadHeapID((void*)prevSegment, extentFinder);
				extentFinder.insertExtentOfFreeSpace((void*)prevEnd, prevSegment + CLPoliciesManager::NVM_SEGMENT_SIZE - prevEnd);
				m_largeObjAllocator.free(false, false, 0, prevEnd, 0, prevSegment + CLPoliciesManager::NVM_SEGMENT_SIZE - prevEnd);

				CLCentralHeap::getThreadHeapID((void*)nextSegment, extentFinder);
				if (nextStart > nextSegment)
				{
					extentFinder.insertExtentOfFreeSpace((void*)nextSegment, nextStart - nextSegment);
					m_largeObjAllocator.free(false, false, 0, nextSegment, 0, nextStart - nextSegment);
				}
			}

			prevEnd = nextStart + allocatedSpaceIter->spaceLen;
			allocatedSpaceIter++;
		}

		if (maxAddr - prevEnd)
		{
			CLCentralHeap::getThreadHeapID((void*)prevEnd, extentFinder);
			extentFinder.insertExtentOfFreeSpace((void*)prevEnd, maxAddr - prevEnd);
			m_largeObjAllocator.free(false, false, 0, prevEnd, 0, maxAddr - prevEnd);
		}
	}

	auto it = CLThreadHeapInfoRecorder::begin();
	unsigned long NVMAddrSpaceStart = CLCentralHeap::getNVMAddrSpaceStart();
	unsigned long index = 0;
	while (!it.reachEnd())
	{
		if (it.getThreadHeapID() == m_thearHeapID)
		{
			void* segmentStart = (void*)(NVMAddrSpaceStart + (index << CLPoliciesManager::NVM_SEGMENT_SHIFT));
			CLCentralHeap::getThreadHeapID(segmentStart, extentFinder);
			
			CLExtent extent = extentFinder.getExtent(segmentStart);
			if (!extent.isValid())
			{
				CLThreadHeapInfoRecorder::markFree(index);
				CLCentralHeap::freeSegment(segmentStart);
			}
		}

		++it;
		++index;
	}

	delete m_tracer;
	m_tracer = nullptr;

	return true;
}

bool CLThreadHeap::free(void * obj)
{
	CLNVMExtentFinder extentFinder;
	unsigned targetThreadHeap = CLCentralHeap::getThreadHeapID(obj, extentFinder);
	if (!targetThreadHeap)
		return false;

	if (targetThreadHeap == m_thearHeapID)
		return localFree(obj, extentFinder);
	else
		return m_channelsManagers[targetThreadHeap - 1].pushBack((unsigned long)obj, m_thearHeapID);
}

bool CLThreadHeap::doneInit()
{
	return m_doneInit;
}
