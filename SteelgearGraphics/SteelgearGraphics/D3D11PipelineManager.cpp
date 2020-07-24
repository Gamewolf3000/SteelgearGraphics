#include "D3D11PipelineManager.h"

SG::D3D11PipelineManager::D3D11PipelineManager(ID3D11Device * device)
{
	this->device = device;
}

SG::D3D11PipelineManager::~D3D11PipelineManager()
{

}

SG::SGResult SG::D3D11PipelineManager::CreateRenderJob(const SGGuid & guid, const SGRenderJob & job)
{
	renderJobs.lock();
	renderJobs[guid] = job;
	renderJobs.unlock();

	return SGResult::OK;
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

SG::SGResult SG::D3D11PipelineManager::CreateDrawCall(const SGGuid & guid, UINT vertexCount, UINT startVertexLocation)
{
	DrawCall toStore;
	toStore.type = DrawType::DRAW;
	toStore.data.draw.vertexCount = vertexCount;
	toStore.data.draw.startVertexLocation = startVertexLocation;

	drawCalls.lock();
	drawCalls[guid] = toStore;
	drawCalls.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11PipelineManager::CreateDrawIndexedCall(const SGGuid & guid, UINT indexCount, UINT startIndexLocation, INT baseVertexLocation)
{
	DrawCall toStore;
	toStore.type = DrawType::DRAW_INDEXED;
	toStore.data.drawIndexed.indexCount = indexCount;
	toStore.data.drawIndexed.startIndexLocation = startIndexLocation;
	toStore.data.drawIndexed.baseVertexLocation = baseVertexLocation;

	drawCalls.lock();
	drawCalls[guid] = toStore;
	drawCalls.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11PipelineManager::CreateDrawInstancedCall(const SGGuid & guid, UINT vertexCountPerInstance, UINT instanceCount, UINT startVertexLocation, UINT startInstanceLocation)
{
	DrawCall toStore;
	toStore.type = DrawType::DRAW_INSTANCED;
	toStore.data.drawInstanced.vertexCountPerInstance = vertexCountPerInstance;
	toStore.data.drawInstanced.instanceCount = instanceCount;
	toStore.data.drawInstanced.startVertexLocation = startVertexLocation;
	toStore.data.drawInstanced.startInstanceLocation = startInstanceLocation;

	drawCalls.lock();
	drawCalls[guid] = toStore;
	drawCalls.unlock();

	return SGResult::OK;
}

SG::SGResult SG::D3D11PipelineManager::CreateDrawIndexedInstancedCall(const SGGuid & guid, UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation, INT baseVertexLocation, UINT startInstanceLocation)
{
	DrawCall toStore;
	toStore.type = DrawType::DRAW_INDEXED_INSTANCED;
	toStore.data.drawIndexedInstanced.indexCountPerInstance = indexCountPerInstance;
	toStore.data.drawIndexedInstanced.instanceCount = instanceCount;
	toStore.data.drawIndexedInstanced.startIndexLocation = startIndexLocation;
	toStore.data.drawIndexedInstanced.baseVertexLocation = baseVertexLocation;
	toStore.data.drawIndexedInstanced.startInstanceLocation = startInstanceLocation;

	drawCalls.lock();
	drawCalls[guid] = toStore;
	drawCalls.unlock();

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
