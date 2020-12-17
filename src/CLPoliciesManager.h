#pragma once
#include <cstddef>
#include <string>

using std::string;

class CLPoliciesManager
{
public:
	const static unsigned long NVM_PAGE_SIZE = 1ul << 14;
	const static unsigned long NVM_SEGMENT_SIZE = 1ul << 33;
	const static unsigned NVM_PAGE_SHIFT = 14u;
	const static unsigned NVM_SEGMENT_SHIFT = 33u;
	const static unsigned NVM_RUN_CACHE_SIZE = 256u;
	const static unsigned MAX_THREAD_NUM = 128u;
	const static unsigned SMALL_OBJ_CATEORY_NUM = 32u;
	const static unsigned TYPE_TO_SIZE_MAP[32];
	const static unsigned NVM_RUN_SIZE = 1u << 17;
	const static unsigned long DRAM_RUN_SIZE = 1ul << 14;
	const static unsigned long MAX_SMALL_OBJ_SIZE = 1ul << 15;
	const static unsigned CHANNEL_CLEAN_CIRCLE = 100u;
	const static unsigned SMALL_OBJ_CACHE_SIZES[32];
	const static string TYPE_INFO_FILE_PATH;
	const static unsigned META_AREA_SIZE = 1ul << 21;
	const static unsigned FIRST_THREADHEAP = 1u;
	const static unsigned DFS_STACK_SIZE = 1024u;

	static unsigned getTypeIndexBySize(size_t objSize);
};