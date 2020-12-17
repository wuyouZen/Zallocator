#pragma once
#include <cstdint>

class CLExtent
{
	unsigned long m_data;
	const static unsigned long TYPE_INDEX_MASK = 0xffff000000000000ul;
	const static unsigned TYPE_INDEX_SHIFT = 48u;

public:
	CLExtent() {
		m_data = 0;
	}

	CLExtent(unsigned long size, bool isLargeObj = false) {
		if (size)
		{
			if (isLargeObj)
				m_data = size | 3ul;
			else
				m_data = size;
		}
		else
			m_data = 0;
	}

	CLExtent(void *pRunManageStruct) {
		m_data = (unsigned long)pRunManageStruct | 2ul;
	}

	bool isLargeObj() {
		return (m_data & 3ul) == 3;
	}

	bool isFree() {
		return (m_data & 3ul) == 0;
	}

	void *getRunManageStruct() {
		return (void *)((m_data & ~3ul) & ~TYPE_INDEX_MASK);
	}

	void setTypeIndex(uint16_t typeIndex) {
		m_data |= (unsigned long)typeIndex << TYPE_INDEX_SHIFT;
	}

	uint16_t getTypeIndex() {
		return (m_data & TYPE_INDEX_MASK) >> TYPE_INDEX_SHIFT;
	}

	unsigned long getLen() {
		return m_data & ~3ul;
	}

	bool isValid() {
		return m_data != 0;
	}

	bool operator!=(const CLExtent & rhs) {
		return m_data != rhs.m_data;
	}

	bool operator==(const CLExtent & rhs) {
		return m_data == rhs.m_data;
	}
};