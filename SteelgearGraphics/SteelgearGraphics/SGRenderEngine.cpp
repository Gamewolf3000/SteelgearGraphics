#include "SGRenderEngine.h"

#include <utility>

SG::SGRenderEngine::SGRenderEngine(const SGRenderSettings& settings)
{
	threadPool = new SGThreadPool(settings.nrOfContexts >= 1 ? settings.nrOfContexts - 1 : 0);
}

void SG::SGRenderEngine::Render(const std::vector<SGPipelineJob>& jobs)
{
	dataIndexMutex.lock();
	pipelineJobs[toUpdate] = jobs;
	std::swap(toUpdate, toUseNext);
	SwapUpdateBuffer();
	dataIndexMutex.unlock();
}

void SG::SGRenderEngine::RenderThreadFunction()
{
	int lastIndex = -1;

	while (active)
	{
		if (lastIndex == toUseNext)
			continue;

		dataIndexMutex.lock();
		lastIndex = toWorkWith;
		std::swap(toWorkWith, toUseNext);
		SwapToWorkWithBuffer();
		dataIndexMutex.unlock();


	}
}
