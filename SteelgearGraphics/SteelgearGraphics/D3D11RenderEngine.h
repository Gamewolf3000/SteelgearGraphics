#pragma once

#include <d3d11_4.h>
#include <dxgi1_6.h>

#include "SGRenderEngine.h"

#include "D3D11BufferHandler.h"
#include "D3D11DrawDataHandler.h"
#include "D3D11SamplerHandler.h"
#include "D3D11ShaderManager.h"
#include "D3D11StateHandler.h"
#include "D3D11TextureHandler.h"
#include "SGGraphicsPipelineHandler.h"

namespace SG
{
	class D3D11RenderEngine : public SGRenderEngine
	{
	public:
		D3D11RenderEngine(const SGRenderSettings& settings);
		~D3D11RenderEngine();


	private:
		ID3D11Device* device;
		ID3D11DeviceContext* immediateContext;
		std::vector<ID3D11DeviceContext> defferedContexts;

		D3D11BufferHandler* bufferHandler;
		D3D11DrawDataHandler* drawDataHandler;
		D3D11SamplerHandler* samplerHandler;
		D3D11ShaderManager* shaderManager;
		D3D11StateHandler* stateHandler;
		D3D11TextureHandler* textureHandler;
		SGGraphicsPipelineHandler pipelineHandler;
	};
}