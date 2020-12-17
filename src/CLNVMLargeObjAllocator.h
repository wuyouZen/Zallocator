#pragma once
#include <cstddef>
#include <cstdlib>
#include <unordered_map>
#include "CLNVMExtentFinder.h"
#include "CLIntervalTree.h"
#include "CLSlab.h"

using std::unordered_map;

template<typename TTestClass>
class _CLNVMLargeObjAllocator
{
	friend TTestClass;

	CLIntervalTree m_intervalTree;
	CLDRAMSlab * m_treeNodeAllocator;

public:
	_CLNVMLargeObjAllocator(CLDRAMSlab * treeNodeAllocator);
	bool alloc(size_t size, unsigned long & offset);
	bool alloc(size_t size, unsigned long newSegmentOffset, unsigned long & offset);
	bool free(bool mergePrev, bool mergeNext, unsigned long prevExtent, unsigned long extent, unsigned long nextExtent, size_t objSize);
};

typedef _CLNVMLargeObjAllocator<void> CLNVMLargeObjAllocator;