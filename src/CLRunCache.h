#pragma once
#include <cstddef>
#include <cstring>
#include "CLPoliciesManager.h"

template<unsigned TCACHE_SIZE, typename TTestClass>
class _CLRunCache
{
	friend TTestClass;

	unsigned m_head, m_tail;
	void * m_cachedRuns[TCACHE_SIZE - 1];

	_CLRunCache(const _CLRunCache &) = delete;
	_CLRunCache & operator=(const _CLRunCache &) = delete;

public:
	_CLRunCache() {
		m_head = m_tail = 0;
	}

	bool insert(void *runAddr) {
		unsigned newTail = m_tail + 1;
		if (__glibc_unlikely(newTail > TCACHE_SIZE - 2))
			newTail = 0;

		if (__glibc_unlikely(newTail == m_head))
			return false;

		m_cachedRuns[m_tail] = runAddr;
		m_tail = newTail;
		return true;
	}

	void * get() {
		void *res;

		if (__glibc_unlikely(m_head == m_tail))
			return NULL;

		res = m_cachedRuns[m_head];
		m_head++;
		if (__glibc_unlikely(m_head > TCACHE_SIZE - 2))
			m_head = 0;

		return res;
	}
};

template<unsigned TCACHE_SIZE>
using CLRunCache = _CLRunCache<TCACHE_SIZE, void>;

typedef CLRunCache<CLPoliciesManager::NVM_RUN_CACHE_SIZE> CLNVMRunCache;