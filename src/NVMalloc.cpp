#include "NVMalloc.h"
#include "CLThreadHeap.h"
#include "CLRootInfoManager.h"
#include "CLRecoverManager.h"
#include <mutex>

static CLNVMAddrSpaceBuilder * spaceBuilder = NULL;
static void * NVMAddrSpaceStart = NULL;
static unsigned long NVMAddrSpaceLen = 0;

static CLThreadHeap * threadHeapTable[CLPoliciesManager::MAX_THREAD_NUM] = { NULL };
static bool threadHeapBound[CLPoliciesManager::MAX_THREAD_NUM] = { false };
static std::mutex threadHeapTableLock;

static __thread CLThreadHeap * localHeap = NULL;

static void createLocalHeap()
{
	std::lock_guard<std::mutex> guard(threadHeapTableLock);
	unsigned threadHeapID;
	for (threadHeapID = 0; threadHeapID < CLPoliciesManager::MAX_THREAD_NUM; threadHeapID++)
		if (!threadHeapBound[threadHeapID])
			break;

	if (threadHeapID < CLPoliciesManager::MAX_THREAD_NUM)
	{
		if (NVMAddrSpaceLen)
			localHeap = new CLThreadHeap(threadHeapID + 1);
		else
			return;
	}
	else
		return;

	if (!localHeap)
		return;
	if (!localHeap->doneInit())
	{
		localHeap = NULL;
		return;
	}

	threadHeapTable[threadHeapID] = localHeap;
	threadHeapBound[threadHeapID] = true;
}

void * nvmalloc(size_t objSize)
{
	if (localHeap)
		return localHeap->alloc(objSize);
	else
	{
		createLocalHeap();
		if (localHeap)
			return localHeap->alloc(objSize);
		else
			return NULL;
	}
}

void nvfree(void * obj)
{
	if (obj)
	{
		if (localHeap)
			localHeap->free(obj);
		else
		{
			createLocalHeap();
			if (localHeap)
				localHeap->free(obj);
		}
	}
}

bool buildNVMAddrSpace(CLNVMAddrSpaceBuilder * builder)
{
	spaceBuilder = builder;
	if (!spaceBuilder)
		return false;
	NVMAddrSpaceStart = spaceBuilder->buildAddrSpace(NVMAddrSpaceLen);
	if (!NVMAddrSpaceLen)
		return false;

	if (!CLThreadHeapInfoRecorder::init(NVMAddrSpaceStart))
		return false;

	if (!CLCentralHeap::init(NVMAddrSpaceStart, NVMAddrSpaceLen, CLThreadHeapInfoRecorder::begin()))
		return false;

	if (!CLTypeInfoReader::init(CLPoliciesManager::TYPE_INFO_FILE_PATH))
		return false;

	unsigned long rootSpaceStart = (unsigned long)NVMAddrSpaceStart + CLThreadHeapInfoRecorder::getHeaderSize();
	unsigned long rootSpaceLen = CLPoliciesManager::META_AREA_SIZE - CLThreadHeapInfoRecorder::getHeaderSize();
	if (!CLRootInfoManager::init((void*)rootSpaceStart, rootSpaceLen))
		return false;

	CLRecoverManager recoverManager(&threadHeapTable[0]);
	recoverManager.doRecover();

	return true;
}

bool bindThreadHeap(unsigned threadHeapID)
{
	std::lock_guard<std::mutex> guard(threadHeapTableLock);

	--threadHeapID;
	if (threadHeapID >= CLPoliciesManager::MAX_THREAD_NUM)
		return false;

	if (threadHeapBound[threadHeapID])
		return false;

	if (!threadHeapTable[threadHeapID]) 
	{
		auto pHead = new CLThreadHeap(threadHeapID + 1);
		if (!pHead)
			return false;
		if (!pHead->doneInit()) 
		{
			delete pHead;
			return false;
		}
		threadHeapTable[threadHeapID] = pHead;
	}

	localHeap = threadHeapTable[threadHeapID];
	threadHeapBound[threadHeapID] = true;

	return true;
}

bool registRoot(const string& rootName, const string& typeName, void* addr)
{
	return CLRootInfoManager::registRoot(rootName, typeName, addr);
}

bool setRootNodeAddr(const string& rootName, void* addr)
{
	return CLRootInfoManager::setRootNodeAddr(rootName, addr);
}

void deleteRoot(const string& rootName)
{
	CLRootInfoManager::deleteRoot(rootName);
}

bool rootExisted(const string& rootName)
{
	return CLRootInfoManager::findRoot(rootName);
}

void* findRoot(const string& rootName)
{
	void* rootInfo = CLRootInfoManager::findRoot(rootName);
	if (!rootInfo)
		return nullptr;
	return *(void**)rootInfo;
}
