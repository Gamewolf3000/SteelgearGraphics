#pragma once

#include <d3d11_4.h>
#include <utility>

#include "SGGraphicsHandler.h"

#include "D3D11CommonTypes.h"

namespace SG
{
	struct PipelineComponent
	{
		Association source;
		SGGuid componentGuid;
	};

	struct RenderShader
	{
		SGGuid shader;
		std::vector<ShaderResource> constantBuffers;
		std::vector<ShaderResource> shaderResourceViews;
		std::vector<ShaderResource> samplers;
	};

	struct SGRenderJob
	{
		Association association;
		SGGuid inputAssembly;
		std::vector<PipelineComponent> vertexBuffers;
		PipelineComponent indexBuffer;
		RenderShader vertexShader;
		RenderShader hullShader;
		RenderShader domainShader;
		RenderShader geometryShader;
		RenderShader pixelShader;
		std::vector<PipelineComponent> uavs;
		std::vector<PipelineComponent> rtvs;
		PipelineComponent dsv;
		PipelineComponent blendState;
		PipelineComponent drawCall;
	};

	struct SGComputeJob
	{
		Association association;
		SGGuid shader;
		std::vector<ShaderResource> constantBuffers;
		std::vector<ShaderResource> shaderResourceViews;
		std::vector<ShaderResource> samplers;
		std::vector<ShaderResource> uavs;
	};

	struct SGClearRenderTargetJob
	{
		SGGuid toClear;
		float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	};

	struct SGClearDepthStencilJob
	{	
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

		SGResult CreateDrawCall(const SGGuid& guid, UINT vertexCount = 0, UINT startVertexLocation = 0);
		SGResult CreateDrawIndexedCall(const SGGuid& guid, UINT indexCount = 0, UINT startIndexLocation = 0, INT baseVertexLocation = 0);
		SGResult CreateDrawInstancedCall(const SGGuid& guid, UINT vertexCountPerInstance = 0, UINT instanceCount = 0, UINT startVertexLocation = 0, UINT startInstanceLocation = 0);
		SGResult CreateDrawIndexedInstancedCall(const SGGuid& guid, UINT indexCountPerInstance = 0, UINT instanceCount = 0, UINT startIndexLocation = 0, INT baseVertexLocation = 0, UINT startInstanceLocation = 0);

	private:

		friend class D3D11RenderEngine;

		enum class DrawType
		{
			DRAW,
			DRAW_INDEXED,
			DRAW_INSTANCED,
			DRAW_INDEXED_INSTANCED
		};

		struct DrawData
		{
			UINT vertexCount;
			UINT startVertexLocation;
		};

		struct DrawIndexedData
		{
			UINT indexCount;
			UINT startIndexLocation;
			INT baseVertexLocation;
		};

		struct DrawInstancedData
		{
			UINT vertexCountPerInstance;
			UINT instanceCount;
			UINT startVertexLocation;
			UINT startInstanceLocation;
		};

		struct DrawIndexedInstanced
		{
			UINT indexCountPerInstance;
			UINT instanceCount;
			UINT startIndexLocation;
			INT  baseVertexLocation;
			UINT startInstanceLocation;
		};

		struct DrawCall
		{
			DrawType type;
			union
			{
				DrawData draw;
				DrawIndexedData drawIndexed;
				DrawInstancedData drawInstanced;
				DrawIndexedInstanced drawIndexedInstanced;
			} data;
		};

		LockableUnorderedMap<SGGuid, SGRenderJob> renderJobs;
		LockableUnorderedMap<SGGuid, SGComputeJob> computeJobs;
		LockableUnorderedMap<SGGuid, SGClearRenderTargetJob> clearRenderTargetJobs;
		LockableUnorderedMap<SGGuid, SGClearDepthStencilJob> clearDepthStencilJobs;
		LockableUnorderedMap<SGGuid, SGPipeline> pipelines;
		LockableUnorderedMap<SGGuid, DrawCall> drawCalls;

		ID3D11Device* device;

		SGRenderJob GetRenderJob(const SGGuid& guid);
		SGComputeJob GetComputeJob(const SGGuid& guid);
		SGClearRenderTargetJob GetClearRenderTargetJob(const SGGuid& guid);
		SGClearDepthStencilJob GetClearDepthStencilJob(const SGGuid& guid);
		SGPipeline GetPipeline(const SGGuid& guid);
	};
}