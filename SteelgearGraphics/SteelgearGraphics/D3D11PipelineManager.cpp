#include "D3D11PipelineManager.h"

SG::D3D11PipelineManager::D3D11PipelineManager(ID3D11Device * device)
{
	this->device = device;
}

SG::SGResult SG::D3D11PipelineManager::CreateRenderJob(const SGGuid & guid, const SGRenderJob & job)
{
	renderJobs.AddElement(guid, job);
	return SGResult::OK;
}

void SG::D3D11PipelineManager::RemoveRenderJob(const SGGuid& guid)
{
	renderJobs.RemoveElement(guid);
}

SG::SGResult SG::D3D11PipelineManager::CreateComputeJob(const SGGuid & guid, const SGComputeJob & job)
{
	computeJobs.AddElement(guid, job);
	return SGResult::OK;
}

void SG::D3D11PipelineManager::RemoveComputeJob(const SGGuid& guid)
{
	computeJobs.RemoveElement(guid);
}

SG::SGResult SG::D3D11PipelineManager::CreateClearRenderTargetJob(const SGGuid & guid, const SGClearRenderTargetJob & job)
{
	clearRenderTargetJobs.AddElement(guid, job);
	return SGResult::OK;
}

void SG::D3D11PipelineManager::RemoveClearRenderTargetJob(const SGGuid& guid)
{
	clearRenderTargetJobs.RemoveElement(guid);
}

SG::SGResult SG::D3D11PipelineManager::CreateClearDepthStencilJob(const SGGuid & guid, const SGClearDepthStencilJob & job)
{
	clearDepthStencilJobs.AddElement(guid, job);
	return SGResult::OK;
}

void SG::D3D11PipelineManager::RemoveClearDepthStencilJob(const SGGuid& guid)
{
	clearDepthStencilJobs.RemoveElement(guid);
}

SG::SGResult SG::D3D11PipelineManager::CreatePipeline(const SGGuid & guid, const SGPipeline & pipeline)
{
	pipelines.AddElement(guid, pipeline);
	return SGResult::OK;
}

void SG::D3D11PipelineManager::RemovePipeline(const SGGuid& guid)
{
	pipelines.RemoveElement(guid);
}

void SG::D3D11PipelineManager::FinishFrame()
{
	renderJobs.FinishFrame();
	computeJobs.FinishFrame();
	clearRenderTargetJobs.FinishFrame();
	clearDepthStencilJobs.FinishFrame();
	pipelines.FinishFrame();
}

void SG::D3D11PipelineManager::SwapFrame()
{
	renderJobs.UpdateActive();
	computeJobs.UpdateActive();
	clearRenderTargetJobs.UpdateActive();
	clearDepthStencilJobs.UpdateActive();
	pipelines.UpdateActive();
}

SG::SGRenderJob SG::D3D11PipelineManager::GetRenderJob(const SGGuid & guid)
{
	if constexpr (DEBUG_VERSION)
	{
		if(!renderJobs.HasElement(guid))
			throw std::runtime_error("Error fetching render job, guid does not exist");
	}

	return renderJobs[guid];
}

SG::SGComputeJob SG::D3D11PipelineManager::GetComputeJob(const SGGuid & guid)
{
	if constexpr (DEBUG_VERSION)
	{
		if (!computeJobs.HasElement(guid))
			throw std::runtime_error("Error fetching compute job, guid does not exist");
	}

	return computeJobs[guid];
}

SG::SGClearRenderTargetJob SG::D3D11PipelineManager::GetClearRenderTargetJob(const SGGuid & guid)
{
	if constexpr (DEBUG_VERSION)
	{
		if (!clearRenderTargetJobs.HasElement(guid))
			throw std::runtime_error("Error fetching clear rendertarget job, guid does not exist");
	}

	return clearRenderTargetJobs[guid];
}

SG::SGClearDepthStencilJob SG::D3D11PipelineManager::GetClearDepthStencilJob(const SGGuid & guid)
{
	if constexpr (DEBUG_VERSION)
	{
		if (!clearDepthStencilJobs.HasElement(guid))
			throw std::runtime_error("Error fetching clear rendertarget job, guid does not exist");
	}

	return clearDepthStencilJobs[guid];
}

SG::SGPipeline SG::D3D11PipelineManager::GetPipeline(const SGGuid & guid)
{
	if constexpr (DEBUG_VERSION)
	{
		if (!pipelines.HasElement(guid))
			throw std::runtime_error("Error fetching clear rendertarget job, guid does not exist");
	}

	return pipelines[guid];
}
