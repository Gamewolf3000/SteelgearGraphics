#pragma once

#include <d3d11_4.h>
#include <utility>

#include "SGRenderEngine.h"
#include "SGResult.h"
#include "FrameMap.h"

#include "D3D11CommonTypes.h"

namespace SG
{
	struct ConstantBuffer
	{
		bool clearAtEnd = false;
		PipelineComponent component;
	};

	struct ResourceView
	{
		enum class ResourceType
		{
			BUFFER,
			TEXTURE
		} type;

		bool clearAtEnd = false;
		PipelineComponent component;
	};

	struct RenderShader
	{
		SGGuid shader;
		std::vector<ConstantBuffer> constantBuffers;
		std::vector<ResourceView> shaderResourceViews;
		std::vector<PipelineComponent> samplers;
	};

	struct SGVertexBuffer
	{
		bool clearAtEnd = false;
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
		bool clearAtEnd = false;
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
		TRIANGLESTRIP,
		LINELIST_ADJ,
		LINESTRIP_ADJ,
		TRIANGLELIST_ADJ,
		TRIANGLESTRIP_ADJ,
		CONTROL_POINT_PATCHLIST_1,
		CONTROL_POINT_PATCHLIST_2,
		CONTROL_POINT_PATCHLIST_3,
		CONTROL_POINT_PATCHLIST_4,
		CONTROL_POINT_PATCHLIST_5,
		CONTROL_POINT_PATCHLIST_6,
		CONTROL_POINT_PATCHLIST_7,
		CONTROL_POINT_PATCHLIST_8,
		CONTROL_POINT_PATCHLIST_9,
		CONTROL_POINT_PATCHLIST_10,
		CONTROL_POINT_PATCHLIST_11,
		CONTROL_POINT_PATCHLIST_12,
		CONTROL_POINT_PATCHLIST_13,
		CONTROL_POINT_PATCHLIST_14,
		CONTROL_POINT_PATCHLIST_15,
		CONTROL_POINT_PATCHLIST_16,
		CONTROL_POINT_PATCHLIST_17,
		CONTROL_POINT_PATCHLIST_18,
		CONTROL_POINT_PATCHLIST_19,
		CONTROL_POINT_PATCHLIST_20,
		CONTROL_POINT_PATCHLIST_21,
		CONTROL_POINT_PATCHLIST_22,
		CONTROL_POINT_PATCHLIST_23,
		CONTROL_POINT_PATCHLIST_24,
		CONTROL_POINT_PATCHLIST_25,
		CONTROL_POINT_PATCHLIST_26,
		CONTROL_POINT_PATCHLIST_27,
		CONTROL_POINT_PATCHLIST_28,
		CONTROL_POINT_PATCHLIST_29,
		CONTROL_POINT_PATCHLIST_30,
		CONTROL_POINT_PATCHLIST_31,
		CONTROL_POINT_PATCHLIST_32
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
		std::vector<ResourceView> rtvs;
		std::vector<ResourceView> uavs;
		std::vector<PipelineComponent> viewports;
		ResourceView dsv;
		PipelineComponent rasterizerState;
		PipelineComponent blendState;
		PipelineComponent drawCall;
	};

	struct SGComputeJob
	{
		Association association;
		SGGuid shader;
		PipelineComponent dispatchCall;
		std::vector<ConstantBuffer> constantBuffers;
		std::vector<ResourceView> shaderResourceViews;
		std::vector<ResourceView> unorderedAccessViews;
		std::vector<PipelineComponent> samplers;
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
		~D3D11PipelineManager() = default;

		SGResult CreateRenderJob(const SGGuid& guid, const SGRenderJob& job);
		void RemoveRenderJob(const SGGuid& guid);

		SGResult CreateComputeJob(const SGGuid& guid, const SGComputeJob& job);
		void RemoveComputeJob(const SGGuid& guid);

		SGResult CreateClearRenderTargetJob(const SGGuid& guid, const SGClearRenderTargetJob& job);
		void RemoveClearRenderTargetJob(const SGGuid& guid);

		SGResult CreateClearDepthStencilJob(const SGGuid& guid, const SGClearDepthStencilJob& job);
		void RemoveClearDepthStencilJob(const SGGuid& guid);

		SGResult CreatePipeline(const SGGuid& guid, const SGPipeline& pipeline);
		void RemovePipeline(const SGGuid& guid);

	private:

		friend class D3D11RenderEngine;

		FrameMap<SGGuid, SGRenderJob> renderJobs;
		FrameMap<SGGuid, SGComputeJob> computeJobs;
		FrameMap<SGGuid, SGClearRenderTargetJob> clearRenderTargetJobs;
		FrameMap<SGGuid, SGClearDepthStencilJob> clearDepthStencilJobs;
		FrameMap<SGGuid, SGPipeline> pipelines;

		ID3D11Device* device;

		void FinishFrame();
		void SwapFrame();

		SGRenderJob GetRenderJob(const SGGuid& guid);
		SGComputeJob GetComputeJob(const SGGuid& guid);
		SGClearRenderTargetJob GetClearRenderTargetJob(const SGGuid& guid);
		SGClearDepthStencilJob GetClearDepthStencilJob(const SGGuid& guid);
		SGPipeline GetPipeline(const SGGuid& guid);
	};
}