#pragma once
#include <set>

using std::set;

struct STAllocatedSpace
{
	unsigned long spaceLen;
	unsigned long spaceStart;

	STAllocatedSpace(unsigned long len, unsigned long start) :
		spaceLen(len), spaceStart(start) {}

	bool operator<(const STAllocatedSpace& s) const 
	{
		return spaceStart < s.spaceStart;
	}
};

class CLAllocatedSpaceTracer
{
	set<STAllocatedSpace> m_allocatedSpace;

public:
	void trace(unsigned long len, void* start) {
		m_allocatedSpace.emplace(len, (unsigned long)start);
	}

	auto begin()->decltype(m_allocatedSpace.begin())
	{
		return m_allocatedSpace.begin();
	}

	auto rbegin()->decltype(m_allocatedSpace.rbegin())
	{
		return m_allocatedSpace.rbegin();
	}

	auto end()->decltype(m_allocatedSpace.end())
	{
		return m_allocatedSpace.end();
	}
};