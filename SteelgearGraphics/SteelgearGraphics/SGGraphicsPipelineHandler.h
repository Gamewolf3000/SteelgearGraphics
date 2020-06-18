#pragma once

#include <d3d11_4.h>
#include <vector>
#include <unordered_map>

#include "SGResult.h"
#include "SGRenderEngine.h"
#include "SGGuid.h"

#include "D3D11CommonTypes.h"

namespace SG
{
	enum class Association
	{
		GLOBAL,
		GROUP,
		ENTITY
	};

	enum class DrawType
	{
		DRAW,
		DRAW_INDEXED,
		DRAW_INSTANCED,
		DRAW_INDEXED_INSTANCED
	};

	struct ShaderResource
	{
		Association source;
		SGGuid resourceGuid;
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
		SGGuid inputAssembly;
		std::vector<ShaderResource> vertexBuffers;
		std::vector<ShaderResource> indexBuffers;
		RenderShader vertexShader;
		RenderShader hullShader;
		RenderShader domainShader;
		RenderShader geometryShader;
		RenderShader pixelShader;
		std::vector<ShaderResource> uavs;
		std::vector<SGGuid> rtvs;
		SGGuid dsv;
		SGGuid blendState;
		DrawType drawType;
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

	class SGGraphicsPipelineHandler
	{
	public:
		SGGraphicsPipelineHandler(ID3D11Device* device);
		~SGGraphicsPipelineHandler();

	private:
		enum PipelineType
		{
			RENDER,
			COMPUTE
		};

		std::unordered_map<SGGuid, SGRenderPipeline> renderPipelines;
		std::unordered_map<SGGuid, SGComputePipeline> computePipelines;
		std::unordered_map<SGGuid, SGGraphicsPipeline> graphicsPipelines;

	};
}