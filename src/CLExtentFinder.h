#pragma once
#include "CLExtent.h"

class CLExtentFinder
{
public:
	virtual bool insertExtentOfRun(CLExtent extent, void *addr, unsigned long len) = 0;
	virtual void removeExtentOfRun(void *addr, unsigned long len) = 0;
	virtual ~CLExtentFinder();
};