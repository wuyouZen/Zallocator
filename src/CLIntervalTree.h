#pragma once
#include <cstddef>

enum ENodeColor
{
	red, black
};

template<typename TTestClass>
class _CLIntervalTreeNode;

class CLNodeInfo
{
	template<typename TTestClass>
	friend class _CLIntervalTreeNode;

	unsigned long m_info;

	const static unsigned long START_MASK = 0x7ffffffful;
	const static unsigned LENGTH_SHIFT = 31u;
	const static unsigned COLOR_SHIFT = 62u;

	CLNodeInfo(unsigned long intervalStatr, unsigned long intervalLen, ENodeColor color) {
		m_info = intervalStatr | (intervalLen << LENGTH_SHIFT) | ((unsigned long)color << COLOR_SHIFT);
	}

	unsigned long getIntervalStart() {
		return m_info & START_MASK;
	}

	unsigned long getIntervalLen() {
		return (m_info >> LENGTH_SHIFT) & START_MASK;
	}

	void setInterval(unsigned long start, unsigned long len) {
		m_info &= ~((1ul << COLOR_SHIFT) - 1);
		m_info |= start | (len << LENGTH_SHIFT);
	}

	ENodeColor getColor() {
		return (ENodeColor)(m_info >> COLOR_SHIFT);
	}

	void setColor(ENodeColor color) {
		m_info &= (1ul << COLOR_SHIFT) - 1;
		m_info |= ((unsigned long)color << COLOR_SHIFT);
	}

	bool operator<(CLNodeInfo other) {
		unsigned long len1, len2;

		len1 = getIntervalLen();
		len2 = other.getIntervalLen();

		if (len1 < len2)
			return true;
		else if (len1 > len2)
			return false;
		else
		{
			if (getIntervalStart() < other.getIntervalStart())
				return true;
			else
				return false;
		}
	}

	bool operator==(CLNodeInfo other) {
		return getIntervalStart() == other.getIntervalStart();
	}

	CLNodeInfo & operator=(const CLNodeInfo & other) {
		m_info &= ~((1ul << COLOR_SHIFT) - 1);
		m_info |= other.m_info & ((1ul << COLOR_SHIFT) - 1);

		return *this;
	}
};

enum ERotateState
{
	L, R, New, N
};

template<typename TTestClass>
class _CLIntervalTree
{
	friend class _CLIntervalTreeNode<TTestClass>;

	friend TTestClass;

	_CLIntervalTreeNode<TTestClass> *m_root;

	bool insert(_CLIntervalTreeNode<TTestClass> *node, ERotateState & subTreeRotateState);
	_CLIntervalTreeNode<TTestClass> * getAndRemove(unsigned long len, bool & needAdjust, bool returnWhenFirstMeet);
	_CLIntervalTreeNode<TTestClass> * getAndRemove(unsigned long len, unsigned long start, bool & needAdjust);

	ENodeColor getRootColor() {
		if (m_root)
			return m_root->getColor();
		else
			return black;
	}

	void setRootColor(ENodeColor color) {
		if(m_root)
			m_root->setNodeColor(color);
	}

public:
	_CLIntervalTree() :m_root(NULL) {}
	_CLIntervalTree(_CLIntervalTreeNode<TTestClass> * root) :m_root(root) {}

	bool insert(_CLIntervalTreeNode<TTestClass> *node);
	_CLIntervalTreeNode<TTestClass> * getAndRemove(unsigned long len);
	_CLIntervalTreeNode<TTestClass> * getAndRemove(unsigned long len, unsigned long start);
};

template<typename TTestClass>
class _CLIntervalTreeNode
{
	friend class _CLIntervalTree<TTestClass>;

	friend TTestClass;

	CLNodeInfo m_info;
	_CLIntervalTree<TTestClass> m_leftSubTree;
	_CLIntervalTree<TTestClass> m_rightSubTree;

	_CLIntervalTreeNode * insert(_CLIntervalTreeNode *node, ERotateState & subTreeRotateState);
	_CLIntervalTreeNode * getAndRemove(unsigned long len, _CLIntervalTreeNode **newRoot, bool & needAdjust, bool returnWhenFirstMeet);
	_CLIntervalTreeNode * getAndRemove(unsigned long len, unsigned long start, _CLIntervalTreeNode **newRoot, bool & needAdjust);
	
	ENodeColor getColor() {
		return m_info.getColor();
	}

	void setNodeColor(ENodeColor color) {
		m_info.setColor(color);
	}

	bool isLeaf();

	_CLIntervalTreeNode * rotateL();
	_CLIntervalTreeNode * rotateR();

	_CLIntervalTreeNode * adjustLL();
	_CLIntervalTreeNode * adjustLR();
	_CLIntervalTreeNode * adjustRL();
	_CLIntervalTreeNode * adjustRR();
	void paintChildsBlack();
	_CLIntervalTreeNode * doAdjustDuringInsertion(ERotateState subTreeRotateState, ERotateState & rotateState, bool insertToLeftChild);
	_CLIntervalTreeNode * doAdjustDuringDeletion(bool removeFromLeftChild, bool & keepAdjusting);

public:
	_CLIntervalTreeNode(unsigned long start, unsigned long len) :
		m_info(start, len, red) {}

	unsigned long getIntervalStart() {
		return m_info.getIntervalStart();
	}

	unsigned long getIntervalLen() {
		return m_info.getIntervalLen();
	}

	void setInterval(unsigned long start, unsigned long len) {
		m_info.setInterval(start, len);
	}
};

typedef _CLIntervalTreeNode<void> CLIntervalTreeNode;
typedef _CLIntervalTree<void> CLIntervalTree;