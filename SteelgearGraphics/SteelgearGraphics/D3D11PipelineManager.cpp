#include "D3D11PipelineManager.h"

SG::D3D11PipelineManager::D3D11PipelineManager(ID3D11Device * device)
{
	this->device = device;
}

SG::D3D11PipelineManager::~D3D11PipelineManager()
{
	for (auto& layout : inputLayouts)
		ReleaseCOM(layout.second.inputLayout);
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

SG::SGRenderJob SG::D3D11PipelineManager::GetRenderJob(const SGGuid & guid)
{
	if constexpr (DEBUG_VERSION)
	{
		if(renderJobs.find(guid) == renderJobs.end())
			throw std::runtime_error("Error fetching render job, guid does not exist");
	}

	return renderJobs[guid];
}

SG::SGComputeJob SG::D3D11PipelineManager::GetComputeJob(const SGGuid & guid)
{
	if constexpr (DEBUG_VERSION)
	{
		if (computeJobs.find(guid) == computeJobs.end())
			throw std::runtime_error("Error fetching render job, guid does not exist");
	}

	return computeJobs[guid];
}

SG::SGClearRenderTargetJob SG::D3D11PipelineManager::GetClearRenderTargetJob(const SGGuid & guid)
{
	if constexpr (DEBUG_VERSION)
	{
		if (clearRenderTargetJobs.find(guid) == clearRenderTargetJobs.end())
			throw std::runtime_error("Error fetching clear rendertarget job, guid does not exist");
	}

	return clearRenderTargetJobs[guid];
}

SG::SGClearDepthStencilJob SG::D3D11PipelineManager::GetClearDepthStencilJob(const SGGuid & guid)
{
	if constexpr (DEBUG_VERSION)
	{
		if (clearDepthStencilJobs.find(guid) == clearDepthStencilJobs.end())
			throw std::runtime_error("Error fetching clear rendertarget job, guid does not exist");
	}

	return clearDepthStencilJobs[guid];
}

SG::SGPipeline SG::D3D11PipelineManager::GetPipeline(const SGGuid & guid)
{
	if constexpr (DEBUG_VERSION)
	{
		if (pipelines.find(guid) == pipelines.end())
			throw std::runtime_error("Error fetching clear rendertarget job, guid does not exist");
	}

	return pipelines[guid];
}
