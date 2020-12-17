#pragma once
#include "CLExtentFinder.h"
#include "CLExtentRadixTree.h"

template<typename TTestClass>
class _CLNVMExtentFinder : public CLExtentFinder
{
	friend TTestClass;

	CLExtentRadixTree m_extentTree;

	bool insertExtentHeadTail(void *addr, unsigned long len, bool isLargeObj);
	void removeExtentHeadTail(void *addr, unsigned long len);

public:
	bool init();
	void destroy();

	bool insertExtentOfRun(CLExtent extent, void *addr, unsigned long len);
	void removeExtentOfRun(void *addr, unsigned long len);
	bool insertExtentOfLargeObj(void *start, unsigned long len);
	void removeExtentOfLargeObj(void *addr, unsigned long len);
	bool insertExtentOfFreeSpace(void *start, unsigned long len);
	void removeExtentOfFreeSpace(void *start, unsigned long len);
	CLExtent getExtent(void *addr);
};

typedef _CLNVMExtentFinder<void> CLNVMExtentFinder;