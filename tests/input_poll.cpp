#include "../source/input_poll.hpp"
#include <chrono>
#include <future>
#include <iostream>
#include <stdexcept>

int main()
{
	std::recursive_mutex mutex;
	std::unique_lock<std::recursive_mutex> lock(mutex);
	bool completed = false;
	std::future<bool> worker;
	reshade::poll_input_unlocked(lock, [&] {
		worker = std::async(std::launch::async, [&] {
			std::lock_guard<std::recursive_mutex> input_lock(mutex);
			return true;
		});
		completed = worker.wait_for(std::chrono::seconds(2)) == std::future_status::ready;
	});
	if (!lock.owns_lock())
		throw std::runtime_error("Input state must be locked again after polling");
	lock.unlock();
	worker.get();
	if (!completed)
	{
		std::cerr << "FAIL: synchronous input worker blocked by polling thread\n";
		return 1;
	}
	lock.lock();
	try
	{
		reshade::poll_input_unlocked(lock, [] { throw std::runtime_error("poll failure"); });
		return 2;
	}
	catch (const std::runtime_error &)
	{
		if (!lock.owns_lock())
			return 3;
	}
	std::cout << "PASS: input worker can complete, lock restored, exceptions propagated\n";
}
