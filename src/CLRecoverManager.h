#pragma once
#include "CLThreadHeap.h"
#include "CLTypeInfoReader.h"
#include <vector>
#include <stack>

using std::stack;
using std::vector;

class CLRecoverManager
{
	CLThreadHeap** m_threadHeapTable;

	void gatherRoots(unsigned threadHeapID, stack<STPtrEntry, vector<STPtrEntry>>& st);
	void doSearchAndRecover(unsigned threadHeapID);

public:
	CLRecoverManager(CLThreadHeap** threadHeapTable) :
		m_threadHeapTable(threadHeapTable){}

	void doRecover();
};