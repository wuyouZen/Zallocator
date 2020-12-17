#include "CLExtentRadixTree.h"

#define START_BIT		33

bool CLExtentRadixTree::init()
{
	m_root = new CLExtentRadixTreeRoot;
	return m_root;
}

void CLExtentRadixTree::destroy()
{
	CLExtentRadixTreeRoot* root = (CLExtentRadixTreeRoot*)((unsigned long)m_root & ((1ul << 48) - 1));
	delete root;
	m_root = NULL;
}

bool CLExtentRadixTree::insert(void * key, CLExtent extent)
{
	uint16_t *count = (uint16_t *)(&m_root) + 3;
	CLExtentRadixTreeRoot *root = (CLExtentRadixTreeRoot *)((unsigned long)m_root & ((1ul << 48) - 1));

	return root->insert(key, extent, START_BIT, count);
}

bool CLExtentRadixTree::remove(void * key)
{
	uint16_t *count = (uint16_t *)(&m_root) + 3;
	CLExtentRadixTreeRoot *root = (CLExtentRadixTreeRoot *)((unsigned long)m_root & ((1ul << 48) - 1));

	return root->remove(key, START_BIT, count);
}

CLExtent CLExtentRadixTree::get(void * key)
{
	CLExtentRadixTreeRoot *root = (CLExtentRadixTreeRoot *)((unsigned long)m_root & ((1ul << 48) - 1));

	return root->get(key, START_BIT);
}