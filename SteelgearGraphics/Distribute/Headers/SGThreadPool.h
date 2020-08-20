#pragma once

#include <mutex>
#include <functional>
#include <vector>
#include <queue>
#include <condition_variable>
#include <atomic>

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
			std::atomic<FunctionStatus>* statusPtr = nullptr;
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

		void EnqueFunction(std::atomic<FunctionStatus>* statusPtr, const std::function<void(void)>& function);

		template<class returnType, class... argTypes>
		void EnqueFunction(std::atomic<FunctionStatus>* statusPtr, const std::function<returnType(argTypes...)>& function, argTypes&&... arguments);

		template<class returnType, class... argTypes>
		void EnqueFunction(std::atomic<FunctionStatus>* statusPtr, returnType(*function)(argTypes...), argTypes&&... arguments);

		bool FunctionsEnqued();
	};




}

template<class returnType, class ...argTypes>
inline void SG::SGThreadPool::EnqueFunction(std::atomic<FunctionStatus>* statusPtr, const std::function<returnType(argTypes...)>& function, argTypes&&... arguments)
{
	EnqueFunction(statusPtr, std::bind(function, arguments...));
}

template<class returnType, class ...argTypes>
inline void SG::SGThreadPool::EnqueFunction(std::atomic<FunctionStatus>* statusPtr, returnType(*function)(argTypes...), argTypes && ...arguments)
{
	EnqueFunction(statusPtr, std::bind(function, arguments...));
}
