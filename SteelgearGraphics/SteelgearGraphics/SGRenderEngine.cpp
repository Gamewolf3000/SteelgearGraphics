#include "SGRenderEngine.h"

#include <utility>

SG::SGRenderEngine::SGRenderEngine(const SGRenderSettings& settings)
{
	this->threadedRenderLoop = settings.threadedRenderLoop;
	threadPool = new SGThreadPool(settings.nrOfContexts >= 1 ? settings.nrOfContexts - 1 : 0);

	if (threadedRenderLoop)
	{
		std::thread renderThread(&SG::SGRenderEngine::RenderThreadFunction, this);
		renderThread.detach();
	}
}

void SG::SGRenderEngine::Render(const std::vector<SGGraphicsJob>& jobs)
{
	dataIndexMutex.lock();
	pipelineJobs[toUpdate] = jobs;
	std::swap(toUpdate, toUseNext);
	SwapUpdateBuffer();
	dataIndexMutex.unlock();

	if (!threadedRenderLoop)
	{
		std::swap(toWorkWith, toUseNext);
		SwapToWorkWithBuffer();
		ExecuteJobs(pipelineJobs[toWorkWith]);
	}
}

SG::SGGraphicalEntityID SG::SGRenderEngine::CreateEntity()
{
	SGGraphicalEntityID toReturn;
	entityMutex.lock();
	toReturn = graphicalEntities.size();
	graphicalEntities.push_back(SGGraphicalEntity());
	entityMutex.unlock();
	return toReturn;
}

void SG::SGRenderEngine::SetEntityToGroup(const SGGraphicalEntityID & entity, const SGGuid & groupGuid)
{
	if constexpr (DEBUG_VERSION)
		if (entity >= graphicalEntities.size())
			throw std::runtime_error("Error setting entity to group, entity does not exist");

	graphicalEntities[entity].groupGuid = groupGuid;
}

void SG::SGRenderEngine::RenderThreadFunction()
{
	renderthreadActive = true;
	int lastIndex = toUseNext;

	while (engineActive)
	{
		if (lastIndex == toUseNext)
			continue;

		dataIndexMutex.lock();
		lastIndex = toWorkWith;
		std::swap(toWorkWith, toUseNext);
		SwapToWorkWithBuffer();
		dataIndexMutex.unlock();

		ExecuteJobs(pipelineJobs[toWorkWith]);
	}

	renderthreadActive = false;
}
