#pragma once
#include <cstdint>
#include <emmintrin.h>

#define CACHE_LINE_SIZE		64ul
#define PAGE_SIZE			4096ul
#define offset_of(type, member) ((size_t)&((type *)0)->member)
#define container_of(ptr, type, member) (type *)((char *)(ptr) - offset_of(type, member))

#ifdef COUNT_FLUSH
#include "FlushCount.h"
#endif

inline int8_t _bsr_int64(uint64_t num) {
	int64_t count;
	__asm__(
		"bsrq %1, %0;jnz 1f; movq $-1,%0;1:"
		:"=r"(count) : "m"(num));
	return count;
}

inline int8_t _bsf_int64(uint64_t num) {
	int64_t count;
	__asm__(
		"bsfq %1, %0;jnz 1f; movq $-1,%0;1:"
		:"=r"(count) : "m"(num));
	return count;
}

inline uint64_t _rdtsc() {
	uint32_t low, high;
	__asm__(
		"rdtsc":"=a"(low), "=d"(high)
	);
	return ((uint64_t)high << 32) | low;
}

inline void sfence()
{
#ifdef HAVE_SFENCE
	asm volatile("sfence":::"memory");
#endif // HAVE_SFENCE
}

inline void mfence()
{
	_mm_mfence();
}

inline void clflush(const void* ptr)
{
	mfence();
	_mm_clflush(ptr);
	mfence();

#ifdef COUNT_FLUSH
	inc_clflush_count();
#endif
}

inline void clflushRange(const void* ptr, uint64_t len)
{
	uintptr_t cur = (uintptr_t)ptr & ~(CACHE_LINE_SIZE - 1);
	for (; cur < (uintptr_t)ptr + len; cur += CACHE_LINE_SIZE)
		clflush((void*)cur);
}