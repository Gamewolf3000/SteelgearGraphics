#include "SGThreadPool.h"
#include <thread>

void SG::SGThreadPool::ThreadFunction(int threadID)
{
	std::mutex cvMutex;
	std::unique_lock<std::mutex>* cvLock = new std::unique_lock<std::mutex>(cvMutex);

	while (poolActive == true)
	{
		functionMutex.lock();

		if (functionsToExecute.size())
		{
			std::function<void(void)> toExecute = functionsToExecute.front().function;
			FunctionStatus* statusPtr = functionsToExecute.front().statusPtr;
			functionsToExecute.pop();
			functionMutex.unlock();

			if (statusPtr)
				*statusPtr = FunctionStatus::PROCESSING;

			toExecute();

			if (statusPtr)
				*statusPtr = FunctionStatus::FINISHED;
		}
		else
		{
			functionMutex.unlock();
			cv.wait(*cvLock);
		}
	}

	delete cvLock;
	threadStatus[threadID] = false;
	return;
}

void SG::SGThreadPool::EnqueFunction(FunctionStatus* statusPtr, const std::function<void(void)>& function)
{
	functionMutex.lock();
	functionsToExecute.push({ function, statusPtr });

	if (statusPtr)
		*statusPtr = FunctionStatus::ENQUEUED;

	functionMutex.unlock();

	cv.notify_one();
}

SG::SGThreadPool::SGThreadPool(int nrOfThreadsInPool)
{
	threadStatus.resize(nrOfThreadsInPool, true);
	for (int i = 0; i < nrOfThreadsInPool; ++i)
	{
		std::thread t(&SG::SGThreadPool::ThreadFunction, this, i);
		t.detach();
	}
}

SG::SGThreadPool::~SGThreadPool()
{
	poolActive = false;
	cv.notify_all();

	for (unsigned int i = 0; i < threadStatus.size(); ++i)
	{
		while (threadStatus[i])
		{
			//Spinwait
		}
	}
}
