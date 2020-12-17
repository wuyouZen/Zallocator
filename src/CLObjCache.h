#pragma once
#include <cstdlib>

class CLObjCache
{
	unsigned m_head, m_tail;
	unsigned long m_maxElementNum;
	void** m_buffer;
	unsigned long m_padding;

public:
	CLObjCache() {}

	CLObjCache(unsigned long elementSize, unsigned long cacheSize) :
		m_head(0), m_tail(0)
	{
		m_maxElementNum = cacheSize / elementSize;
		if(m_maxElementNum)
			m_buffer = (void**)malloc(m_maxElementNum * sizeof(void*));
	}
	
	bool insert(void* objAddr) {
		if (__glibc_unlikely(m_maxElementNum == 0))
			return false;

		unsigned newTail = m_tail + 1;
		if (__glibc_unlikely(newTail > m_maxElementNum - 1))
			newTail = 0;

		if (__glibc_unlikely(newTail == m_head))
			return false;

		m_buffer[m_tail] = objAddr;
		m_tail = newTail;
		return true;
	}

	void* get() {
		void* res;

		if (__glibc_unlikely(m_head == m_tail))
			return NULL;

		res = m_buffer[m_head];
		m_head++;
		if (__glibc_unlikely(m_head > m_maxElementNum - 1))
			m_head = 0;

		return res;
	}
};