#include "D3D11PipelineManager.h"

SG::D3D11PipelineManager::D3D11PipelineManager(ID3D11Device * device)
{
	this->device = device;
}

SG::SGResult SG::D3D11PipelineManager::CreateClearRenderTargetJob(const SGGuid & guid, const SGClearRenderTargetJob & job)
{
	clearRenderTargetJobs.lock();
	clearRenderTargetJobs[guid] = job;
	clearRenderTargetJobs.unlock();
	return SGResult::OK;
}

SG::SGResult SG::D3D11PipelineManager::CreatePipeline(const SGGuid & guid, const SGPipeline & pipeline)
{
	pipelines.lock();
	pipelines[guid] = pipeline;
	pipelines.unlock();
	return SGResult::OK;
}
