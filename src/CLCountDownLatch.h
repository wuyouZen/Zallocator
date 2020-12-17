#pragma once
#include <mutex>
#include <condition_variable>

class CLCountDownLatch
{
	std::condition_variable m_connd;
	std::mutex m_mtx;

	unsigned m_count;

public:
	CLCountDownLatch(unsigned count = 0) :m_count(count) {}
	
	void setCount(unsigned count)
	{
		m_count = count;
	}

	void wait()
	{
		std::unique_lock<std::mutex> lck(m_mtx);
		while (m_count > 0)
			m_connd.wait(lck);
	}

	void countDown()
	{
		std::unique_lock<std::mutex> lck(m_mtx);
		--m_count;
		if (m_count == 0)
			m_connd.notify_all();
	}
};