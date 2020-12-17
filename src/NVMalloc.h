#pragma once
#include <cstddef>
#include <string>
#include "CLNVMAddrSpaceBuilder.h"

using std::string;

void * nvmalloc(size_t objSize);
void nvfree(void * obj);
bool buildNVMAddrSpace(CLNVMAddrSpaceBuilder * spaceBuilder);
bool bindThreadHeap(unsigned ThreadHeapID);
bool registRoot(const string& rootName, const string& typeName, void* addr);
bool setRootNodeAddr(const string& rootName, void* addr);
void deleteRoot(const string& rootName);
bool rootExisted(const string& rootName);
void* findRoot(const string& rootName);

#ifdef COUNT_FLUSH
unsigned long long get_clflush_count();
#endif