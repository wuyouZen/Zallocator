#pragma once
#include "CLExtent.h"
#include "CLRadixTreeNode.h"

typedef CLRadixTreeNode<CLExtent, CLRadixTreeNode<CLExtent, void, 9>, 10> CLExtentRadixTreeRoot;

class CLExtentRadixTree
{
	CLExtentRadixTreeRoot *m_root;

public:
	bool init();
	void destroy();
	bool insert(void *key, CLExtent extent);
	bool remove(void *key);
	CLExtent get(void *key);
};