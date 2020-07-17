#pragma once

#include <mutex>
#include <unordered_map>

template<class key, class value>
class LockableUnorderedMap : public std::unordered_map<key, value>
{
private:
	std::mutex mutex;

public:
	LockableUnorderedMap() {};

	inline void lock()
	{
		mutex.lock();
	}

	inline void unlock()
	{
		mutex.unlock();
	}
};