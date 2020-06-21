#pragma once

#include <d3d11_4.h>

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

	struct SGRenderPipeline
	{
		Association association;
		PipelineComponent inputAssembly;
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

	struct SGComputePipeline
	{
		Association association;
		SGGuid shader;
		std::vector<ShaderResource> constantBuffers;
		std::vector<ShaderResource> shaderResourceViews;
		std::vector<ShaderResource> samplers;
		std::vector<ShaderResource> uavs;
	};

	struct SGGraphicsPipeline
	{
		std::vector<SGGuid> subPipelines;
	};

	class SGGraphicsPipelineHandler : public SGGraphicsHandler
	{
	public:
		SGGraphicsPipelineHandler(ID3D11Device* device);
		~SGGraphicsPipelineHandler();

		SGResult CreateInputLayout(const SGGuid& guid, const SGGuid& vertexShaderGuid, const std::vector<SGInputElement>& inputElements);


	private:
		enum PipelineType
		{
			RENDER,
			COMPUTE
		};

		struct D3D11InputLayoutData
		{
			ID3D11InputLayout* inputLayout = nullptr;
		};

		std::unordered_map<SGGuid, D3D11InputLayoutData> inputLayouts;
		std::unordered_map<SGGuid, SGRenderPipeline> renderPipelines;
		std::unordered_map<SGGuid, SGComputePipeline> computePipelines;
		std::unordered_map<SGGuid, SGGraphicsPipeline> graphicsPipelines;
	};
}