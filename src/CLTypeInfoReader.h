#pragma once
#include <unordered_map>
#include <string>

using std::unordered_map;
using std::string;

typedef uint16_t TypeInfoID;

const static TypeInfoID INVALID_TYPE_ID = 0xffff;

struct STPtrEntry
{
	union
	{
		struct
		{
			unsigned long address : 48;
			unsigned long typeID : 16;
		};
		struct
		{
			unsigned long ptrOffset : 48;
			unsigned long ptrTypeID : 16;
		};
	};
};

struct STTypeInfo
{
	struct
	{
		unsigned long objectSize : 48;
		unsigned long ptrsNum : 16;
	};

	STPtrEntry ptrs[1];
};

template<typename TTestClass>
class _CLTypeInfoReader {
	friend TTestClass;

	unordered_map<string, TypeInfoID> m_typeInfos;
	char* m_typeInfoBuffer;
	bool m_doneInit;

	static _CLTypeInfoReader* m_instance;

	_CLTypeInfoReader(const string& typeInfoFileName);

public:
	static bool init(const string& typeInfoFileName) {
		if (!m_instance)
		{
			m_instance = new _CLTypeInfoReader(typeInfoFileName);
			if (!m_instance->m_doneInit)
			{
				delete m_instance;
				return false;
			}
		}
		return m_instance;
	}

	static _CLTypeInfoReader* getInstance() {
		return m_instance;
	}

	STTypeInfo* getTypeInfoByID(TypeInfoID id) {
		if(m_typeInfoBuffer)
			return (STTypeInfo*)(m_typeInfoBuffer + (id << 3));
		return nullptr;
	}

	TypeInfoID convertNameToID(const string& typeName) {
		auto iter = m_typeInfos.find(typeName);
		if (iter != m_typeInfos.end())
			return iter->second;
		return INVALID_TYPE_ID;
	}

	STTypeInfo* getTypeInfoByName(const string& typeName) {
		TypeInfoID id = convertNameToID(typeName);
		if (id != INVALID_TYPE_ID)
			return getTypeInfoByID(id);
		return nullptr;
	}
};

typedef _CLTypeInfoReader<void> CLTypeInfoReader;