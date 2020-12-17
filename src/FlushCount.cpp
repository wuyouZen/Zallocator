#include "FlushCount.h"
#include <atomic>

std::atomic_ullong clflush_count;

unsigned long long get_clflush_count()
{
	return clflush_count.load();
}

void inc_clflush_count()
{
	clflush_count++;
}
