#pragma once
#include "CLDRAMRun.h"
#include "CLBitmapNVMRun.h"
#include "CLDoubleLinkList.h"
#include <new>

template<typename TRun, typename TManageStruct>
class CLRunManageStructCreator
{
public:
	TManageStruct *create(unsigned objSize, unsigned runSize, void *runAddr) {
		return NULL;
	}

	void destory(TManageStruct *manageStruct) {

	}
};

template <>
class CLRunManageStructCreator<CLDRAMRun, STDoubleLinkListNode<CLDRAMRun>>
{
public:
	STDoubleLinkListNode<CLDRAMRun> *create(unsigned objSize, unsigned runSize, void *runAddr) {
		STDoubleLinkListNode<CLDRAMRun> *res = (STDoubleLinkListNode<CLDRAMRun> *)runAddr;
		new (&res->val) CLDRAMRun(objSize);
		return res;
	}

	void destory(STDoubleLinkListNode<CLDRAMRun> *manageStruct) {

	}
};