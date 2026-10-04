#pragma once
#include <mutex>

namespace reshade
{
	template <typename Poll>
	void poll_input_unlocked(std::unique_lock<std::recursive_mutex> &lock, Poll poll)
	{
		// Wine's key polling can synchronously wait for a message thread which needs this mutex.
		lock.unlock();
		struct restore_lock
		{
			std::unique_lock<std::recursive_mutex> &lock;
			~restore_lock() { lock.lock(); }
		} restore { lock };
		poll();
	}
}
