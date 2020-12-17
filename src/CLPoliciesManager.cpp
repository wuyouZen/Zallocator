#include "CLPoliciesManager.h"
#include "Utils.h"

const unsigned CLPoliciesManager::TYPE_TO_SIZE_MAP[32] = {
	64, 128, 192, 256, 320, 384, 448, 512,
	640, 768, 896, 1024,
	1280, 1536, 1792, 2048,
	2560, 3072, 3584, 4096,
	5 * 1024, 6 * 1024, 7 * 1024, 8 * 1024,
	10 * 1024, 12 * 1024, 14 * 1024, 16 * 1024,
	20 * 1024, 24 * 1024, 28 * 1024, 32 * 1024
};

const unsigned CLPoliciesManager::SMALL_OBJ_CACHE_SIZES[32] = {
	4096, 4096, 4096, 4096, 8192, 8192, 8192, 8192,
	16384, 16384, 16384, 16384,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0
};

const string CLPoliciesManager::TYPE_INFO_FILE_PATH = "./typeinfo/typeinfo.json";

unsigned CLPoliciesManager::getTypeIndexBySize(size_t objSize)
{
	unsigned typeIndex;
	unsigned long offset;
	unsigned firstTrueBit = _bsr_int64(objSize);
	if (firstTrueBit < 9)
		typeIndex = ((objSize + 63) >> 6) - 1;
	else
	{
		offset = objSize - (1ul << firstTrueBit);
		if (!offset)
			typeIndex = 7 + ((firstTrueBit - 9) << 2);
		else
			typeIndex = (offset >> (firstTrueBit - 2)) + 8 + ((firstTrueBit - 9) << 2);
	}

	return typeIndex;
}
