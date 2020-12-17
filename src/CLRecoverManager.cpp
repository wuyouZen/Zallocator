#include "CLRecoverManager.h"
#include "CLRootInfoManager.h"
#include "CLTypeInfoReader.h"
#include "CLThreadHeapInfoRecorder.h"
#include "CLCountDownLatch.h"
#include <thread>
#include <atomic>
#include <cassert>

std::atomic_uint searchingThreadNum(0);
CLCountDownLatch canStart;
CLCountDownLatch canFinish;

void CLRecoverManager::gatherRoots(unsigned threadHeapID, stack<STPtrEntry, vector<STPtrEntry>>& st)
{
	auto it = CLRootInfoManager::begin();
	auto typeInfoReader = CLTypeInfoReader::getInstance();
	while (!it.reachEnd())
	{
		unsigned long rootAddress;
		CLNVMExtentFinder extentFinder;
		void* rootInfo = it.get();
		rootAddress = *(unsigned long*)rootInfo;
		if (CLCentralHeap::getThreadHeapID((void*)rootAddress, extentFinder) == threadHeapID)
		{
			STPtrEntry entry;

			char* rootName = (char*)rootInfo + sizeof(unsigned long);
			char* typeName = rootName + strlen(rootName) + 1;
			entry.typeID = typeInfoReader->convertNameToID(typeName);
			entry.address = rootAddress;

			st.push(entry);
		}

		++it;
	}
}

void CLRecoverManager::doSearchAndRecover(unsigned threadHeapID)
{
	canStart.wait();

	vector<STPtrEntry> vct;
	vct.reserve(CLPoliciesManager::DFS_STACK_SIZE);
	stack<STPtrEntry, vector<STPtrEntry>> st(vct);

	gatherRoots(threadHeapID, st);

	CLThreadHeap* threadHeap = m_threadHeapTable[threadHeapID - 1];
	if (threadHeap == nullptr)
	{
		threadHeap = new CLThreadHeap(threadHeapID);
		assert(threadHeap != nullptr);
		assert(threadHeap->doneInit());
		m_threadHeapTable[threadHeapID - 1] = threadHeap;
	}

	CLTypeInfoReader* typeInfoReader = CLTypeInfoReader::getInstance();

	threadHeap->beginRecover();

	while (!st.empty())
	{
		STPtrEntry entry = st.top();
		st.pop();

		threadHeap->allocAt(entry);

		unsigned long objAddr = entry.address;
		STTypeInfo* typeInfo = typeInfoReader->getTypeInfoByID(entry.typeID);

		for (int i = 0; i < typeInfo->ptrsNum; i++)
		{
			STPtrEntry newEntry;
			newEntry.address = *(unsigned long*)(objAddr + typeInfo->ptrs[i].ptrOffset);
			newEntry.typeID = typeInfo->ptrs[i].ptrTypeID;

			if(newEntry.address)
				st.push(newEntry);
		}
	}

	--searchingThreadNum;

	while (searchingThreadNum.load() != 0)
	{
		threadHeap->doRecoverChannelClean();
	}

	threadHeap->doRecoverChannelClean();
	threadHeap->endRecover();

	canFinish.countDown();
	return;
}

void CLRecoverManager::doRecover()
{
	bool threadHeapNeedRecover[CLPoliciesManager::MAX_THREAD_NUM] = { false };

	auto it = CLThreadHeapInfoRecorder::begin();
	int index = 0;
	while (!it.reachEnd())
	{
		if (it.getThreadHeapID() != SEGMENT_FREE)
			threadHeapNeedRecover[it.getThreadHeapID() - 1] = true;
		++it; ++index;
	}

	unsigned recoverThreadNum = 0;

	for (int i = 0; i < CLPoliciesManager::MAX_THREAD_NUM; i++)
		if (threadHeapNeedRecover[i])
			recoverThreadNum++;

	canStart.setCount(recoverThreadNum);
	canFinish.setCount(recoverThreadNum);

	for (int i = 0; i < CLPoliciesManager::MAX_THREAD_NUM; i++)
	{
		if (threadHeapNeedRecover[i])
		{
			std::thread t(&CLRecoverManager::doSearchAndRecover, this, i + 1);
			t.detach();

			++searchingThreadNum;
			canStart.countDown();
		}
	}

	canFinish.wait();

	return;
}