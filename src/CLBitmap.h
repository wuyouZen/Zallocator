#pragma once
#include <cstdint>
#include <cstring>
#include "Utils.h"

template<typename TTestClass, unsigned TORDER_PER_BIT>
class _CLBitMap
{
	friend TTestClass;

	const static unsigned BITMAP_LEN = 32;

	uint64_t m_bitmaps[BITMAP_LEN];

public:
	_CLBitMap(unsigned objSize, unsigned runSize) {
		unsigned skip, invalidBits, invalidBitmap;

		invalidBits = runSize >> TORDER_PER_BIT;

		if (objSize == (1ul << TORDER_PER_BIT))
			memset(m_bitmaps, 255, BITMAP_LEN * sizeof(uint64_t));
		else
		{
			memset(m_bitmaps, 0, BITMAP_LEN * sizeof(uint64_t));

			skip = objSize >> TORDER_PER_BIT;
			for (unsigned curBit = 0, curBitmap; curBit < invalidBits; curBit += skip)
			{
				curBitmap = curBit >> 6;
				m_bitmaps[curBitmap] |= 1ul << (curBit & 63u);
			}
		}

		while (invalidBits & 63u)
		{
			m_bitmaps[invalidBits >> 6] &= ~(1ul << (invalidBits & 63u));
			invalidBits++;
		}

		invalidBitmap = invalidBits >> 6;
		if (invalidBitmap < BITMAP_LEN)
			memset(&m_bitmaps[invalidBitmap], 0, sizeof(uint64_t) * (BITMAP_LEN - invalidBitmap));
	}

	int getAndSet(unsigned lastAccessPos)
	{
		int index;

		for (unsigned i = 0; i < BITMAP_LEN; i++)
		{
			lastAccessPos++;
			if (__glibc_unlikely(lastAccessPos >= BITMAP_LEN))
				lastAccessPos = 0;

			index = _bsf_int64(m_bitmaps[lastAccessPos]);
			if (index != -1)
			{
				m_bitmaps[lastAccessPos] &= ~(1ul << index);
				return index + (lastAccessPos << 6);
			}
		}

		return -1;
	}
	
	void clearBit(unsigned index)
	{
		m_bitmaps[index >> 6] |= 1ul << (index & 63u);
	}

	void setBit(unsigned index)
	{
		m_bitmaps[index >> 6] &= ~(1ul << (index & 63u));
	}
};

#ifndef ORDER_PER_BIT
#define ORDER_PER_BIT	6u
#endif

typedef _CLBitMap<void, ORDER_PER_BIT> CLBitMap;