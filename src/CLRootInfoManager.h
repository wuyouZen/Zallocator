#pragma once
#include <unordered_map>
#include <string>
#include <cstring>
#include <mutex>
#include "Utils.h"

using std::unordered_map;
using std::string;
using std::mutex;
using std::lock_guard;

template<typename TTestClass>
class _CLRootInfoManager 
{
	friend TTestClass;
	const static unsigned long ROOTS_INFO_END = 0ul;
	const static unsigned long ROOTS_INFO_DELETED = 0xffffffffffffffff;

	unordered_map<string, void*> m_roots;
	void* m_rootSpaceStart;
	void* m_rootSpaceEnd;
	unsigned long m_rootSpaceMaxSize;

	static _CLRootInfoManager* m_instance;
	static mutex m_rootsLock;

	static unsigned long align(unsigned long size) {
		return (size + 7) & ~7ul;
	}

public:
	class CLRootsIter 
	{
		template<typename T>
		friend class _CLRootInfoManager;

		void* m_curPos;

	public:
		CLRootsIter(void* addr = nullptr) :m_curPos(addr) {}

		CLRootsIter& operator++() {
			if (m_curPos) {
				void* nextPos = m_curPos;

				do {
					if (*(unsigned long*)nextPos == ROOTS_INFO_END) {
						m_curPos = nextPos;
						return *this;
					}
					char* rootNameStart = (char*)((unsigned long*)nextPos + 1);
					char* typeNameStart = rootNameStart + strlen(rootNameStart) + 1;
					nextPos = (void*)(typeNameStart + strlen(typeNameStart) + 1);
					nextPos = (void*)align((unsigned long)nextPos);
				} while (*(unsigned long*)nextPos == ROOTS_INFO_DELETED);

				m_curPos = nextPos;
			}

			return *this;
		}

		CLRootsIter operator++(int) {
			CLRootsIter res(*this);
			++(*this);
			return res;
		}

		bool reachEnd() {
			if (m_curPos)
				return *(unsigned long*)m_curPos == ROOTS_INFO_END;
			return true;
		}

		void* get() {
			if (m_curPos) {
				unsigned long tmp = *(unsigned long*)m_curPos;
				if (tmp == ROOTS_INFO_DELETED || tmp == ROOTS_INFO_END)
					return nullptr;
				return m_curPos;
			}
			return m_curPos;
		}
	};

private:
	_CLRootInfoManager(void* rootSpaceStart, unsigned long maxSize) :
		m_rootSpaceStart(rootSpaceStart), m_rootSpaceMaxSize(maxSize)
	{
		CLRootsIter iter(m_rootSpaceStart);
		while (!iter.reachEnd()) {
			void* rootInfo = iter.get();
			char* rootName = (char*)rootInfo + sizeof(void*);
			m_roots.insert(std::make_pair(string(rootName), rootInfo));
			++iter;
		}

		m_rootSpaceEnd = iter.m_curPos;
	}

	CLRootsIter _begin() {
		return CLRootsIter(m_rootSpaceStart);
	}

	void* _findRoot(const string& rootName) {
		auto it = m_roots.find(rootName);
		if (it != m_roots.end())
			return it->second;
		else
			return nullptr;
	}

	void _deleteRoot(const string& rootName) {
		_setRootNodeAddr(rootName, (void*)ROOTS_INFO_DELETED);
		m_roots.erase(rootName);
	}

	bool _setRootNodeAddr(const string& rootName, void* addr) {
		void* rootInfoStart = _findRoot(rootName);
		if (rootInfoStart) {
			*(void**)rootInfoStart = addr;
			clflush(rootInfoStart);
			return true;
		}

		return false;
	}

	void* _registRootToNVM(const string& rootName, const string& typeName, void* addr) {
		void* rootInfoStart = _findRoot(rootName);
		if (rootInfoStart)
			return nullptr;
		
		unsigned long rootNameSize = rootName.size() + 1;
		unsigned long typeNameSize = typeName.size() + 1;
		unsigned long rootInfoSize = align(rootNameSize + typeNameSize + sizeof(void*));

		if ((uint8_t*)m_rootSpaceStart + m_rootSpaceMaxSize >= (uint8_t*)m_rootSpaceEnd + rootInfoSize + sizeof(void*)) {
			rootInfoStart = m_rootSpaceEnd;
			void** pRootNodeAddr = (void**)m_rootSpaceEnd;
			char* rootNameStr = (char*)m_rootSpaceEnd + sizeof(void*);
			char* typeNameStr = rootNameStr + rootNameSize;
			unsigned long* pEndFlag = (unsigned long*)(typeNameStr + typeNameSize);

			strcpy(rootNameStr, rootName.c_str());
			strcpy(typeNameStr, typeName.c_str());
			*pEndFlag = ROOTS_INFO_END;
			clflushRange(rootInfoStart, rootInfoSize + sizeof(void*));

			*pRootNodeAddr = addr;
			clflush(pRootNodeAddr);

			m_rootSpaceEnd = (uint8_t*)m_rootSpaceEnd + rootInfoSize;
			return rootInfoStart;
		}
		else
			return nullptr;
	}

	bool _registRoot(const string& rootName, const string& typeName, void* addr) {
		void* rootInfo;
		if (rootInfo = _registRootToNVM(rootName, typeName, addr)) {
			m_roots.insert(std::make_pair(rootName, rootInfo));
			return true;
		}
		return false;
	}

public:
	static bool init(void* rootSpaceStart, unsigned long maxSize) {
		m_instance = new _CLRootInfoManager<TTestClass>(rootSpaceStart, maxSize);
		return m_instance != nullptr;
	}

	static CLRootsIter begin() {
		if (m_instance)
			return m_instance->_begin();
		return CLRootsIter();
	}

	static void* findRoot(const string& rootName) {
		lock_guard<mutex> guardRootsLock(m_rootsLock);
		if (m_instance)
			return m_instance->_findRoot(rootName);
		return nullptr;
	}

	static void deleteRoot(const string& rootName) {
		lock_guard<mutex> guardRootsLock(m_rootsLock);
		if (m_instance)
			m_instance->_deleteRoot(rootName);
	}

	static bool setRootNodeAddr(const string& rootName, void* addr) {
		lock_guard<mutex> guardRootsLock(m_rootsLock);
		if (m_instance)
			return m_instance->_setRootNodeAddr(rootName, addr);
		return false;
	}

	static bool registRoot(const string& rootName, const string& typeName, void* addr) {
		lock_guard<mutex> guardRootsLock(m_rootsLock);
		if (m_instance)
			return m_instance->_registRoot(rootName, typeName, addr);
		return false;
	}
};

typedef _CLRootInfoManager<void> CLRootInfoManager;