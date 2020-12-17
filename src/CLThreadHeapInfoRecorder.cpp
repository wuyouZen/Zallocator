#include "CLThreadHeapInfoRecorder.h"

const unsigned SEGMENT_FREE = 0;

template<typename TTestClass>
_CLThreadHeapInfoRecorder<TTestClass>* _CLThreadHeapInfoRecorder<TTestClass>::m_instance = nullptr;

template class _CLThreadHeapInfoRecorder<void>;

class CLTestThreadHeapInfoRecorder;
template class _CLThreadHeapInfoRecorder<CLTestThreadHeapInfoRecorder>;