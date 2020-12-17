#pragma once

class CLNVMAddrSpaceBuilder
{
public:
	virtual ~CLNVMAddrSpaceBuilder();
	virtual void * buildAddrSpace(unsigned long & len) = 0;
};