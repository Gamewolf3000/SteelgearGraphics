#pragma once

#include <d3d11_4.h>
#include <utility>

#include "SGGraphicsHandler.h"

#include "D3D11CommonTypes.h"

namespace SG
{
	struct RenderShader
	{
		SGGuid shader;
		std::vector<PipelineComponent> constantBuffers;
		std::vector<PipelineComponent> shaderResourceViews;
		std::vector<PipelineComponent> samplers;
	};

	struct SGVertexBuffer
	{
		PipelineComponent buffer;
		PipelineComponent stride;
		PipelineComponent offset;
	};

	enum class IndexBufferFormat
	{
		IB_32_BIT,
		IB_16_BIT
	};

	struct SGIndexBuffer
	{
		PipelineComponent buffer;
		PipelineComponent offset;
		IndexBufferFormat format = IndexBufferFormat::IB_32_BIT;
	};

	enum class SGTopology
	{
		POINTLIST,
		LINELIST,
		LINESTRIP,
		TRIANGLELIST,
		TRIANGLESTRIP
	};

	struct SGRenderJob
	{
		Association association;
		SGTopology topology;
		SGGuid inputAssembly;
		std::vector<SGVertexBuffer> vertexBuffers;
		SGIndexBuffer indexBuffer;
		RenderShader vertexShader;
		RenderShader hullShader;
		RenderShader domainShader;
		RenderShader geometryShader;
		RenderShader pixelShader;
		std::vector<PipelineComponent> uavs;
		std::vector<PipelineComponent> rtvs;
		std::vector<PipelineComponent> viewports;
		PipelineComponent dsv;
		PipelineComponent blendState;
		PipelineComponent drawCall;
	};

	struct SGComputeJob
	{
		Association association;
		SGGuid shader;
		std::vector<PipelineComponent> constantBuffers;
		std::vector<PipelineComponent> shaderResourceViews;
		std::vector<PipelineComponent> samplers;
		std::vector<PipelineComponent> uavs;
	};

	struct SGClearRenderTargetJob
	{
		SGGuid toClear;
		float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	};

	struct SGClearDepthStencilJob
	{	
		SGGuid toClear;
		bool clearDepth;
		bool clearStencil;
		FLOAT depthClearValue;
		UINT8 stencilClearValue;
	};

	enum class PipelineJobType
	{
		RENDER,
		COMPUTE,
		CLEAR_RENDER_TARGET,
		CLEAR_DEPTH_STENCIL,
		COPY_RESOURCE
	};

	struct SGPipeline
	{
		std::vector<std::pair<PipelineJobType, SGGuid>> jobs;
	};

	class D3D11PipelineManager
	{
	public:
		D3D11PipelineManager(ID3D11Device* device);
		~D3D11PipelineManager();

		SGResult CreateRenderJob(const SGGuid& guid, const SGRenderJob& job);
		SGResult CreateComputeJob(const SGGuid& guid, const SGComputeJob& job);
		SGResult CreateClearRenderTargetJob(const SGGuid& guid, const SGClearRenderTargetJob& job);
		SGResult CreateClearDepthStencilJob(const SGGuid& guid, const SGClearDepthStencilJob& job);
		SGResult CreatePipeline(const SGGuid& guid, const SGPipeline& pipeline);

	private:

		friend class D3D11RenderEngine;



		LockableUnorderedMap<SGGuid, SGRenderJob> renderJobs;
		LockableUnorderedMap<SGGuid, SGComputeJob> computeJobs;
		LockableUnorderedMap<SGGuid, SGClearRenderTargetJob> clearRenderTargetJobs;
		LockableUnorderedMap<SGGuid, SGClearDepthStencilJob> clearDepthStencilJobs;
		LockableUnorderedMap<SGGuid, SGPipeline> pipelines;

		ID3D11Device* device;

		SGRenderJob GetRenderJob(const SGGuid& guid);
		SGComputeJob GetComputeJob(const SGGuid& guid);
		SGClearRenderTargetJob GetClearRenderTargetJob(const SGGuid& guid);
		SGClearDepthStencilJob GetClearDepthStencilJob(const SGGuid& guid);
		SGPipeline GetPipeline(const SGGuid& guid);
	};
}