#include "CLTypeInfoReader.h"
#include <sstream>
#include <fstream>
#include "CJsonObject.hpp"

using std::ifstream;
using std::stringstream;
using neb::CJsonObject;

template<typename TTestClass>
_CLTypeInfoReader<TTestClass>::_CLTypeInfoReader(const string& typeInfoFileName):m_typeInfoBuffer(nullptr) 
{
	m_doneInit = false;

	ifstream is(typeInfoFileName);
	if (!is.is_open())
		return;

	stringstream ss;
	ss << is.rdbuf();
	is.close();

	CJsonObject typeInfoJson(ss.str());
	CJsonObject typeInfoArray;
	if (!typeInfoJson.Get("typeInfos", typeInfoArray))
		return;
	if (!typeInfoArray.IsArray())
		return;

	unsigned numEntry = typeInfoArray.GetArraySize();
	unsigned long spaceNeed = 0;
	TypeInfoID curOff = 0;

	for (unsigned i = 0; i < numEntry; i++) {
		auto ptrs = typeInfoArray[i]["ptrs"];
		if (!ptrs.IsArray())
			return;

		string typeName;
		if (!typeInfoArray[i].Get("name", typeName))
			return;

		int numPtrs = ptrs.GetArraySize();
		unsigned long rootInfoSize = sizeof(unsigned long) * (numPtrs + 1);
		if (spaceNeed + rootInfoSize > INVALID_TYPE_ID)
			return;

		m_typeInfos.insert(std::make_pair(typeName, curOff >> 3));
		curOff += rootInfoSize;
		spaceNeed += rootInfoSize;
	}

	if (!spaceNeed)
		return;

	m_typeInfoBuffer = (char*)malloc(spaceNeed);
	if (!m_typeInfoBuffer)
		return;

	char* curPos = m_typeInfoBuffer;
	for (unsigned i = 0; i < numEntry; i++) {
		auto ptrs = typeInfoArray[i]["ptrs"];
		unsigned long objectSize;
		unsigned long numPtrs;
		string ptrTypeName;
		STTypeInfo* curEntry = (STTypeInfo*)curPos;

		if (!typeInfoArray[i].Get("size", objectSize)) {
			delete m_typeInfoBuffer;
			return;
		}

		numPtrs = ptrs.GetArraySize();
		curEntry->objectSize = objectSize;
		curEntry->ptrsNum = numPtrs;

		for (unsigned j = 0; j < numPtrs; j++) {
			if (!ptrs[j].Get("typeName", ptrTypeName)) {
				delete m_typeInfoBuffer;
				return;
			}

			TypeInfoID ptrTypeID = convertNameToID(ptrTypeName);
			if (ptrTypeID == INVALID_TYPE_ID) {
				delete m_typeInfoBuffer;
				return;
			}

			unsigned ptrOffset;
			if (!ptrs[j].Get("offset", ptrOffset)) {
				delete m_typeInfoBuffer;
				return;
			}

			curEntry->ptrs[j].ptrOffset = ptrOffset;
			curEntry->ptrs[j].ptrTypeID = ptrTypeID;
		}

		curPos += sizeof(unsigned long) * (numPtrs + 1);
	}

	m_doneInit = true;
}

template<typename TTestClass>
_CLTypeInfoReader<TTestClass>* _CLTypeInfoReader<TTestClass>::m_instance = nullptr;

template class _CLTypeInfoReader<void>;

class CLTestTypeInfoReader;
template class _CLTypeInfoReader<CLTestTypeInfoReader>;