#include "CLIntervalTree.h"

template<typename TTestClass>
_CLIntervalTreeNode<TTestClass> * _CLIntervalTreeNode<TTestClass>::insert(_CLIntervalTreeNode<TTestClass> * node, ERotateState & rotateState)
{
	ERotateState subTreeRotateState;
	bool success, insertToLeftChild = false;
	_CLIntervalTreeNode *newRoot = this;

	if (__glibc_unlikely(node->m_info == m_info))
		return NULL;

	if (node->m_info < m_info)
	{
		insertToLeftChild = true;
		success = m_leftSubTree.insert(node, subTreeRotateState);
	}
	else
		success = m_rightSubTree.insert(node, subTreeRotateState);

	if (__glibc_unlikely(!success))
		return NULL;

	if (subTreeRotateState != N)
	{
		if (subTreeRotateState == New)
		{
			if (m_info.getColor() == black)
				rotateState = N;
			else
				rotateState = insertToLeftChild ? L : R;
		}
		else
			newRoot = doAdjustDuringInsertion(subTreeRotateState, rotateState, insertToLeftChild);
	}
	else
		rotateState = N;

	return newRoot;
}

template<typename TTestClass>
_CLIntervalTreeNode<TTestClass> * _CLIntervalTreeNode<TTestClass>::getAndRemove(unsigned long len, _CLIntervalTreeNode<TTestClass> ** newRoot, bool & keepAdjust, bool returnWhenFirstMeet)
{
	_CLIntervalTreeNode<TTestClass> *res = NULL;
	*newRoot = this;
	bool needAdjust, removeFromLeftSubTree;
	unsigned long curIntervalLen = m_info.getIntervalLen();

	if (curIntervalLen < len)
		res = m_rightSubTree.getAndRemove(len, needAdjust, false);
	else if (curIntervalLen > len && !returnWhenFirstMeet)
	{
		removeFromLeftSubTree = true;
		res = m_leftSubTree.getAndRemove(len, needAdjust, false);
	}

	if (res == NULL && curIntervalLen >= len)
	{
		if (isLeaf())
		{
			res = this;
			if (m_info.getColor() == red)
			{
				*newRoot = NULL;
				keepAdjust = false;
			}
			else
			{
				if (m_leftSubTree.m_root)
				{
					*newRoot = m_leftSubTree.m_root;
					(*newRoot)->setNodeColor(black);
					keepAdjust = false;
				}
				else if (m_rightSubTree.m_root)
				{
					*newRoot = m_rightSubTree.m_root;
					(*newRoot)->setNodeColor(black);
					keepAdjust = false;
				}
				else
				{
					*newRoot = NULL;
					keepAdjust = true;
				}
			}

			return res;
		}
		else
		{
			CLNodeInfo tmp = m_leftSubTree.m_root->m_info;
			m_leftSubTree.m_root->m_info = m_info;
			m_info = tmp;

			removeFromLeftSubTree = true;
			res = m_leftSubTree.getAndRemove(len, needAdjust, true);
		}
	}

	if (__glibc_unlikely(!res))
		return NULL;

	if (needAdjust)
		*newRoot = doAdjustDuringDeletion(removeFromLeftSubTree, keepAdjust);
	else
		keepAdjust = false;

	return res;
}

template<typename TTestClass>
_CLIntervalTreeNode<TTestClass> * _CLIntervalTreeNode<TTestClass>::getAndRemove(unsigned long len, unsigned long start, _CLIntervalTreeNode<TTestClass> ** newRoot, bool & keepAdjust)
{
	_CLIntervalTreeNode<TTestClass> *res = NULL;
	*newRoot = this;
	bool needAdjust, removeFromLeftSubTree;
	unsigned long curIntervalLen = m_info.getIntervalLen();
	unsigned long curStart = m_info.getIntervalStart();

	if (curIntervalLen < len || (curIntervalLen == len && curStart < start))
		res = m_rightSubTree.getAndRemove(len, start, needAdjust);
	else if (curIntervalLen > len || (curIntervalLen == len && curStart > start))
	{
		removeFromLeftSubTree = true;
		res = m_leftSubTree.getAndRemove(len, start, needAdjust);
	}
	else
	{
		if (isLeaf())
		{
			res = this;
			if (m_info.getColor() == red)
			{
				*newRoot = NULL;
				keepAdjust = false;
			}
			else
			{
				if (m_leftSubTree.m_root)
				{
					*newRoot = m_leftSubTree.m_root;
					(*newRoot)->setNodeColor(black);
					keepAdjust = false;
				}
				else if (m_rightSubTree.m_root)
				{
					*newRoot = m_rightSubTree.m_root;
					(*newRoot)->setNodeColor(black);
					keepAdjust = false;
				}
				else
				{
					*newRoot = NULL;
					keepAdjust = true;
				}
			}

			return res;
		}
		else
		{
			CLNodeInfo tmp = m_leftSubTree.m_root->m_info;
			m_leftSubTree.m_root->m_info = m_info;
			m_info = tmp;

			removeFromLeftSubTree = true;
			res = m_leftSubTree.getAndRemove(len, start, needAdjust);
		}
	}

	if (__glibc_unlikely(!res))
		return NULL;

	if (needAdjust)
		*newRoot = doAdjustDuringDeletion(removeFromLeftSubTree, keepAdjust);
	else
		keepAdjust = false;

	return res;
}

template<typename TTestClass>
bool _CLIntervalTreeNode<TTestClass>::isLeaf()
{
	return !m_leftSubTree.m_root || !m_leftSubTree.m_root;
}

template<typename TTestClass>
_CLIntervalTreeNode<TTestClass> * _CLIntervalTreeNode<TTestClass>::rotateL()
{
	_CLIntervalTree<TTestClass> leftSubTree = m_leftSubTree;
	m_leftSubTree = leftSubTree.m_root->m_rightSubTree;
	leftSubTree.m_root->m_rightSubTree = _CLIntervalTree<TTestClass>(this);

	return leftSubTree.m_root;
}

template<typename TTestClass>
_CLIntervalTreeNode<TTestClass> * _CLIntervalTreeNode<TTestClass>::rotateR()
{
	_CLIntervalTree<TTestClass> rightSubTree = m_rightSubTree;
	m_rightSubTree = rightSubTree.m_root->m_leftSubTree;
	rightSubTree.m_root->m_leftSubTree = _CLIntervalTree<TTestClass>(this);

	return rightSubTree.m_root;
}

template<typename TTestClass>
_CLIntervalTreeNode<TTestClass> * _CLIntervalTreeNode<TTestClass>::adjustLL()
{
	_CLIntervalTreeNode<TTestClass> *newRoot = rotateL();
	setNodeColor(red);
	newRoot->setNodeColor(black);

	return newRoot;
}

template<typename TTestClass>
_CLIntervalTreeNode<TTestClass> * _CLIntervalTreeNode<TTestClass>::adjustLR()
{
	_CLIntervalTreeNode<TTestClass> * newRoot;
	
	m_leftSubTree.m_root = m_leftSubTree.m_root->rotateR();
	newRoot = rotateL();

	setNodeColor(red);
	newRoot->setNodeColor(black);

	return newRoot;
}

template<typename TTestClass>
_CLIntervalTreeNode<TTestClass> * _CLIntervalTreeNode<TTestClass>::adjustRL()
{
	_CLIntervalTreeNode<TTestClass> * newRoot;

	m_rightSubTree.m_root = m_rightSubTree.m_root->rotateL();
	newRoot = rotateR();

	setNodeColor(red);
	newRoot->setNodeColor(black);

	return newRoot;
}

template<typename TTestClass>
_CLIntervalTreeNode<TTestClass> * _CLIntervalTreeNode<TTestClass>::adjustRR()
{
	_CLIntervalTreeNode<TTestClass> * newRoot = rotateR();
	setNodeColor(red);
	newRoot->setNodeColor(black);

	return newRoot;
}

template<typename TTestClass>
void _CLIntervalTreeNode<TTestClass>::paintChildsBlack()
{
	m_info.setColor(red);
	m_leftSubTree.setRootColor(black);
	m_rightSubTree.setRootColor(black);
}

template<typename TTestClass>
_CLIntervalTreeNode<TTestClass> * _CLIntervalTreeNode<TTestClass>::doAdjustDuringInsertion(ERotateState subTreeRotateState, ERotateState & rotateState, bool insertToLeftChild)
{
	_CLIntervalTreeNode<TTestClass> *newRoot = this;

	if (subTreeRotateState == L)
	{
		if (insertToLeftChild)
		{
			if (m_rightSubTree.getRootColor() == red)
			{
				paintChildsBlack();
				rotateState = New;
			}
			else {
				newRoot = adjustLL();
				rotateState = N;
			}
		}
		else
		{
			if (m_leftSubTree.getRootColor() == red)
			{
				paintChildsBlack();
				rotateState = New;
			}
			else
			{
				newRoot = adjustRL();
				rotateState = N;
			}
		}
	}
	else
	{
		if (insertToLeftChild)
		{
			if (m_rightSubTree.getRootColor() == red)
			{
				paintChildsBlack();
				rotateState = New;
			}
			else
			{
				newRoot = adjustLR();
				rotateState = N;
			}
		}
		else
		{
			if (m_leftSubTree.getRootColor() == red)
			{
				paintChildsBlack();
				rotateState = New;
			}
			else
			{
				newRoot = adjustRR();
				rotateState = N;
			}
		}
	}

	return newRoot;
}

template<typename TTestClass>
_CLIntervalTreeNode<TTestClass> * _CLIntervalTreeNode<TTestClass>::doAdjustDuringDeletion(bool removeFromLeftChild, bool & keepAdjusting)
{
	_CLIntervalTreeNode<TTestClass> *newRoot = this;
	_CLIntervalTree<TTestClass> bro = removeFromLeftChild ? m_rightSubTree : m_leftSubTree;
	_CLIntervalTree<TTestClass> leftNephew = bro.m_root->m_leftSubTree, rightNephew = bro.m_root->m_rightSubTree;
	ENodeColor leftNephewColor = leftNephew.getRootColor(), rightNephewColor = rightNephew.getRootColor();
	bool useless;

	if (bro.getRootColor() == red)
	{
		if (removeFromLeftChild)
			newRoot = rotateR();
		else
			newRoot = rotateL();

		bro.setRootColor(black);
		setNodeColor(red);

		if (removeFromLeftChild)
			newRoot->m_leftSubTree.m_root = newRoot->m_leftSubTree.m_root->doAdjustDuringDeletion(true, useless);
		else
			newRoot->m_rightSubTree.m_root = newRoot->m_rightSubTree.m_root->doAdjustDuringDeletion(false, useless);

		keepAdjusting = false;
	}
	else
	{
		if (leftNephewColor == red || rightNephewColor == red)
		{
			if (!removeFromLeftChild)
			{
				if (leftNephewColor == red)
				{
					m_leftSubTree.setRootColor(getColor());
					setNodeColor(black);
					newRoot = rotateL();
					newRoot->m_leftSubTree.setRootColor(black);
				}
				else
				{
					m_leftSubTree.m_root = m_leftSubTree.m_root->rotateR();
					m_leftSubTree.setRootColor(getColor());
					setNodeColor(black);
					newRoot = rotateL();
				}
			}
			else
			{
				if (rightNephewColor == red)
				{
					m_rightSubTree.setRootColor(getColor());
					setNodeColor(black);
					newRoot = rotateR();
					newRoot->m_rightSubTree.setRootColor(black);
				}
				else
				{
					m_rightSubTree.m_root = m_rightSubTree.m_root->rotateL();
					m_rightSubTree.setRootColor(getColor());
					setNodeColor(black);
					newRoot = rotateR();
				}
			}

			keepAdjusting = false;
		}
		else
		{
			if (getColor() == red)
			{
				bro.setRootColor(red);
				setNodeColor(black);
				keepAdjusting = false;
			}
			else
			{
				bro.setRootColor(red);
				keepAdjusting = true;
			}
		}
	}

	return newRoot;
}

template<typename TTestClass>
bool _CLIntervalTree<TTestClass>::insert(_CLIntervalTreeNode<TTestClass> * node, ERotateState & subTreeRotateState)
{
	_CLIntervalTreeNode<TTestClass> *newRoot;

	if (m_root)
	{
		newRoot = m_root->insert(node, subTreeRotateState);
		if (newRoot)
		{
			m_root = newRoot;
			return true;
		}

		return false;
	}
	else
	{
		subTreeRotateState = New;
		m_root = node;
		return true;
	}
}

template<typename TTestClass>
_CLIntervalTreeNode<TTestClass> * _CLIntervalTree<TTestClass>::getAndRemove(unsigned long len, bool & needAdjust, bool returnWhenFirstMeet)
{
	_CLIntervalTreeNode<TTestClass> *newRoot, *res = NULL;

	if (m_root)
	{
		res = m_root->getAndRemove(len, &newRoot, needAdjust, returnWhenFirstMeet);
		if (res)
			m_root = newRoot;
	}

	return res;
}

template<typename TTestClass>
_CLIntervalTreeNode<TTestClass>* _CLIntervalTree<TTestClass>::getAndRemove(unsigned long len, unsigned long start, bool & needAdjust)
{
	_CLIntervalTreeNode<TTestClass> *newRoot, *res = NULL;

	if (m_root)
	{
		res = m_root->getAndRemove(len, start, &newRoot, needAdjust);
		if (res)
			m_root = newRoot;
	}

	return res;
}

template<typename TTestClass>
bool _CLIntervalTree<TTestClass>::insert(_CLIntervalTreeNode<TTestClass> * node)
{
	ERotateState subTreeRotateState;
	node->setNodeColor(red);

	if (insert(node, subTreeRotateState))
	{
		m_root->setNodeColor(black);
		return true;
	}
	else
		return false;
}

template<typename TTestClass>
_CLIntervalTreeNode<TTestClass> * _CLIntervalTree<TTestClass>::getAndRemove(unsigned long len)
{
	bool needAjust;
	_CLIntervalTreeNode<TTestClass> *res = getAndRemove(len, needAjust, false);

	return res;
}

template<typename TTestClass>
_CLIntervalTreeNode<TTestClass>* _CLIntervalTree<TTestClass>::getAndRemove(unsigned long len, unsigned long start)
{
	bool needAjust;
	return getAndRemove(len, start, needAjust);
}

template class _CLIntervalTreeNode<void>;
template class _CLIntervalTree<void>;

class CLTestIntervalTree;
template class _CLIntervalTreeNode<CLTestIntervalTree>;
template class _CLIntervalTree<CLTestIntervalTree>;