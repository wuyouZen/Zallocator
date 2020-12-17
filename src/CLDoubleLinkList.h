#pragma once
#include <cstddef>

template<typename T>
struct STDoubleLinkListNode
{
	STDoubleLinkListNode *m_next;
	STDoubleLinkListNode *m_prev;
	T val;
};

template<typename T, typename TTestClass>
class _CLDoubleLinkList
{
	friend TTestClass;

	STDoubleLinkListNode<T> *m_head;

public:
	_CLDoubleLinkList() :
		m_head(NULL) {}
	_CLDoubleLinkList(STDoubleLinkListNode<T> *head) :
		m_head(head) {}

	void push(STDoubleLinkListNode<T> *node) {
		node->m_next = m_head;
		node->m_prev = NULL;

		if (m_head)
			m_head->m_prev = node;
		m_head = node;

		return;
	}

	STDoubleLinkListNode<T> *getHead() {
		return m_head;
	}

	void erase(STDoubleLinkListNode<T> *node) {
		STDoubleLinkListNode<T> *nextNode, *prevNode;

		nextNode = node->m_next;
		prevNode = node->m_prev;

		if (prevNode)
			prevNode->m_next = nextNode;
		else
			m_head = nextNode;

		if (nextNode)
			nextNode->m_prev = prevNode;

		return;
	}
};

template<typename T>
using CLDoubleLinkList = _CLDoubleLinkList<T, void>;