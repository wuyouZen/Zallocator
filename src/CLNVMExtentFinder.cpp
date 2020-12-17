#include "CLNVMExtentFinder.h"
#include "CLPoliciesManager.h"

template<typename TTestClass>
bool _CLNVMExtentFinder<TTestClass>::insertExtentHeadTail(void * addr, unsigned long len, bool isLargeObj)
{
	bool res = false;

	if (m_extentTree.insert(addr, CLExtent(len, isLargeObj)))
	{
		if (m_extentTree.insert((void *)((unsigned long)addr + len - CLPoliciesManager::NVM_PAGE_SIZE), CLExtent(len, isLargeObj)))
			res = true;
		else
		{
			m_extentTree.remove(addr);
			res = false;
		}
	}

	return res;
}

template<typename TTestClass>
void _CLNVMExtentFinder<TTestClass>::removeExtentHeadTail(void * addr, unsigned long len)
{	
	m_extentTree.remove(addr);
	m_extentTree.remove((void*)((unsigned long)addr + len - CLPoliciesManager::NVM_PAGE_SIZE));
}

template<typename TTestClass>
bool _CLNVMExtentFinder<TTestClass>::init()
{
	return m_extentTree.init();
}

template<typename TTestClass>
void _CLNVMExtentFinder<TTestClass>::destroy()
{
	m_extentTree.destroy();
}

template<typename TTestClass>
bool _CLNVMExtentFinder<TTestClass>::insertExtentOfRun(CLExtent extent, void * addr, unsigned long len)
{
	unsigned long cur = 0;

	while (cur < len)
	{
		if (m_extentTree.insert((void *)((unsigned long)addr + cur), extent))
			cur += CLPoliciesManager::NVM_PAGE_SIZE;
		else
		{
			removeExtentOfRun(addr, len);
			return false;
		}
	}

	return true;
}

template<typename TTestClass>
void _CLNVMExtentFinder<TTestClass>::removeExtentOfRun(void * addr, unsigned long len)
{
	unsigned long cur = 0;
	
	while (cur < len)
	{
		m_extentTree.remove((void*)((unsigned long)addr + cur));
		cur += CLPoliciesManager::NVM_PAGE_SIZE;
	}

	return;
}

template<typename TTestClass>
bool _CLNVMExtentFinder<TTestClass>::insertExtentOfLargeObj(void * start, unsigned long len)
{
	return insertExtentHeadTail(start, len, true);
}

template<typename TTestClass>
void _CLNVMExtentFinder<TTestClass>::removeExtentOfLargeObj(void * addr, unsigned long len)
{
	removeExtentHeadTail(addr, len);
}

template<typename TTestClass>
bool _CLNVMExtentFinder<TTestClass>::insertExtentOfFreeSpace(void * start, unsigned long len)
{
	return insertExtentHeadTail(start, len, false);
}

template<typename TTestClass>
void _CLNVMExtentFinder<TTestClass>::removeExtentOfFreeSpace(void * start, unsigned long len)
{
	CLExtent extent = getExtent(start);
	if (extent.isValid() && extent.isFree())
	{
		removeExtentHeadTail(start, len);
		if (extent.getLen() - len > 0)
			insertExtentOfFreeSpace((void *)((unsigned long)start + len), extent.getLen() - len);
	}
}

template<typename TTestClass>
CLExtent _CLNVMExtentFinder<TTestClass>::getExtent(void * addr)
{
	return m_extentTree.get(addr);
}

template class _CLNVMExtentFinder<void>;

class CLTestNVMExtentFinder;
template class _CLNVMExtentFinder<CLTestNVMExtentFinder>;