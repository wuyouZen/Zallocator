#include "CLRootInfoManager.h"

template<typename TTestClass>
_CLRootInfoManager<TTestClass>* _CLRootInfoManager<TTestClass>::m_instance = nullptr;

template<typename TTestClass>
mutex _CLRootInfoManager<TTestClass>::m_rootsLock;

template class _CLRootInfoManager<void>;

class CLTestRootInfoManager;
template class _CLRootInfoManager<CLTestRootInfoManager>;