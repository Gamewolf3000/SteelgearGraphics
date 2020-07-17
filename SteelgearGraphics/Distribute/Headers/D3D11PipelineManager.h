#pragma once

#include <d3d11_4.h>
#include <utility>

#include "SGGraphicsHandler.h"

#include "D3D11CommonTypes.h"

namespace SG
{
	enum class Association
	{
		GLOBAL,
		GROUP,
		ENTITY
	};

	struct SGInputElement
	{
		std::string semanticName;
		UINT semanticIndex;
		DXGI_FORMAT format;
		UINT inputSlot;
		UINT alignedByteOffset;
		bool instancedData;
		UINT instanceDataStepRate;
	};

	struct ShaderResource
	{
		Association source;
		SGGuid resourceGuid;
	};

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
		PipelineComponent drawCall; // Replace with SGGuid to a DrawInfo struct in a new handler
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
		CLEAR_DEPTH_STENCIL
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

		SGResult CreateInputLayout(const SGGuid& guid, const SGGuid& vertexShaderGuid, const std::vector<SGInputElement>& inputElements);
		SGResult CreateRenderJob(const SGGuid& guid, const SGRenderJob& job);
		SGResult CreateComputeJob(const SGGuid& guid, const SGComputeJob& job);
		SGResult CreateClearRenderTargetJob(const SGGuid& guid, const SGClearRenderTargetJob& job);
		SGResult CreateClearDepthStencilJob(const SGGuid& guid, const SGClearDepthStencilJob& job);
		SGResult CreatePipeline(const SGGuid& guid, const SGPipeline& pipeline);

		SGRenderJob GetRenderJob(const SGGuid& guid);
		SGComputeJob GetComputeJob(const SGGuid& guid);
		SGClearRenderTargetJob GetClearRenderTargetJob(const SGGuid& guid);
		SGClearDepthStencilJob GetClearDepthStencilJob(const SGGuid& guid);
		SGPipeline GetPipeline(const SGGuid& guid);


	private:

		struct D3D11InputLayoutData
		{
			ID3D11InputLayout* inputLayout = nullptr;
		};

		LockableUnorderedMap<SGGuid, D3D11InputLayoutData> inputLayouts;
		LockableUnorderedMap<SGGuid, SGRenderJob> renderJobs;
		LockableUnorderedMap<SGGuid, SGComputeJob> computeJobs;
		LockableUnorderedMap<SGGuid, SGClearRenderTargetJob> clearRenderTargetJobs;
		LockableUnorderedMap<SGGuid, SGClearDepthStencilJob> clearDepthStencilJobs;
		LockableUnorderedMap<SGGuid, SGPipeline> pipelines;

		ID3D11Device* device;
	};
}