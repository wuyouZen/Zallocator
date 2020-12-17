#pragma once
#include <cstdlib>
#include <cstdint>
#include <cstring>

template<typename TVal, typename TNode, unsigned TBIT_COUNT, typename TTestClass>
class _CLRadixTreeNode
{
	friend TTestClass;

	unsigned long m_slot[1ul << TBIT_COUNT];

	unsigned long getIndexFromKey(void *key, unsigned bitCount, unsigned startBits) {
		unsigned long ulkey = (unsigned long)key;
		ulkey &= (1ul << startBits) - 1;
		ulkey >>= (startBits - TBIT_COUNT);

		return ulkey;
	}

public:
	_CLRadixTreeNode() {
		memset(m_slot, 0, sizeof(m_slot));
	}

	~_CLRadixTreeNode() {
		for (unsigned i = 0; i < 1ul << TBIT_COUNT; i++)
		{
			if (m_slot[i] != 0)
			{
				TNode *p = (TNode *)(m_slot[i] & ((1ul << 48) - 1));
				delete p;
			}
		}
	}

	bool insert(void *key, TVal val, unsigned startBits, uint16_t *count) {
		unsigned long index = getIndexFromKey(key, TBIT_COUNT, startBits);
		TNode *son;
		uint16_t *nextCount;

		if (m_slot[index] == 0)
		{
			son = new TNode;
			if (__glibc_unlikely(son == NULL))
				return false;

			(*count)++;
			m_slot[index] = (unsigned long)son;
		}
		else
			son = (TNode *)(m_slot[index] & ((1ul << 48) - 1));

		nextCount = (uint16_t *)(&m_slot[index]) + 3;

		if (son->insert(key, val, startBits - TBIT_COUNT, nextCount))
			return true;
		else
		{
			if (*nextCount == 0)
			{
				delete (TNode *)m_slot[index];
				(*count)--;
				m_slot[index] = 0;
			}
			return false;
		}

		return true;
	}

	bool remove(void *key, unsigned startBits, uint16_t *count) {
		unsigned long index = getIndexFromKey(key, TBIT_COUNT, startBits);
		TNode *son;
		uint16_t *nextCount = (uint16_t *)(&m_slot[index]) + 3;

		if (__glibc_unlikely(m_slot[index] == 0))
			return false;

		son = (TNode *)(m_slot[index] & ((1ul << 48) - 1));
		if (son->remove(key, startBits - TBIT_COUNT, nextCount))
		{
			if (*nextCount == 0)
			{
				delete (TNode *)m_slot[index];
				(*count)--;
				m_slot[index] = 0;
			}

			return true;
		}
		else
			return false;
	}

	TVal get(void *key, unsigned startBits) {
		unsigned long index = getIndexFromKey(key, TBIT_COUNT, startBits);
		TNode *son = (TNode *)(m_slot[index] & ((1ul << 48) - 1));

		if (__glibc_unlikely(son == NULL))
			return TVal(0ul);

		return son->get(key, startBits - TBIT_COUNT);
	}
};

template<typename TVal, unsigned TBIT_COUNT, typename TTestClass>
class _CLRadixTreeNode<TVal, void, TBIT_COUNT, TTestClass>
{
	friend TTestClass;

	TVal m_slot[1ul << TBIT_COUNT];

	unsigned long getIndexFromKey(void *key, unsigned bitCount, unsigned startBits) {
		unsigned long ulkey = (unsigned long)key;
		ulkey &= (1ul << startBits) - 1;
		ulkey >>= (startBits - TBIT_COUNT);

		return ulkey;
	}

public:
	_CLRadixTreeNode() {
		memset(m_slot, 0, sizeof(m_slot));
	}

	~_CLRadixTreeNode() {

	}

	bool insert(void * key, TVal val, unsigned startBits, uint16_t *count) {
		unsigned long ulkey = getIndexFromKey(key, TBIT_COUNT, startBits);

		if(m_slot[ulkey] == TVal(0ul))
			(*count)++;

		m_slot[ulkey] = val;

		return true;
	}

	bool remove(void *key, unsigned startBits, uint16_t *count) {
		unsigned long ulkey = getIndexFromKey(key, TBIT_COUNT, startBits);

		if (m_slot[ulkey] != TVal(0ul))
		{
			m_slot[ulkey] = TVal(0ul);
			(*count)--;
		}

		return true;
	}

	TVal get(void *key, unsigned startBits) {
		unsigned long ulkey = getIndexFromKey(key, TBIT_COUNT, startBits);

		return m_slot[ulkey];
	}
};

template<typename TVal, typename TNode, unsigned TBIT_COUNT>
using CLRadixTreeNode = _CLRadixTreeNode<TVal, TNode, TBIT_COUNT, void>;