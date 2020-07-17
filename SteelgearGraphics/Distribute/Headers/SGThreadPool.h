#pragma once

#include <mutex>
#include <functional>
#include <vector>
#include <queue>
#include <condition_variable>

namespace SG
{
	enum class FunctionStatus
	{
		ENQUEUED,
		PROCESSING,
		FINISHED
	};

	class SGThreadPool
	{
	private:

		struct StoredFunction
		{
			std::function<void(void)> function;
			FunctionStatus* statusPtr = nullptr;
		};

		std::vector<bool> threadStatus;
		std::queue<StoredFunction> functionsToExecute;
		std::mutex functionMutex;
		std::condition_variable cv;
		volatile bool poolActive = true;

		void ThreadFunction(int threadID);

	public:
		SGThreadPool(int nrOfThreadsInPool);
		~SGThreadPool();

		void EnqueFunction(FunctionStatus* statusPtr, const std::function<void(void)>& function);

		template<class returnType, class... argTypes>
		void EnqueFunction(FunctionStatus* statusPtr, const std::function<returnType(argTypes...)>& function, argTypes&&... arguments);

		template<class returnType, class... argTypes>
		void EnqueFunction(FunctionStatus* statusPtr, returnType(*function)(argTypes...), argTypes&&... arguments);
	};




}

template<class returnType, class ...argTypes>
inline void SG::SGThreadPool::EnqueFunction(FunctionStatus* statusPtr, const std::function<returnType(argTypes...)>& function, argTypes&&... arguments)
{
	EnqueFunction(statusPtr, std::bind(function, arguments...));
}

template<class returnType, class ...argTypes>
inline void SG::SGThreadPool::EnqueFunction(FunctionStatus* statusPtr, returnType(*function)(argTypes...), argTypes && ...arguments)
{
	EnqueFunction(statusPtr, std::bind(function, arguments...));
}
