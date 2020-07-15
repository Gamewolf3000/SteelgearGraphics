#pragma once

#include <d3d11_4.h>
#include <exception>
#include <stdexcept>

#include "SGRenderEngine.h"

#include "D3D11BufferHandler.h"
#include "D3D11DrawDataHandler.h"
#include "D3D11SamplerHandler.h"
#include "D3D11ShaderManager.h"
#include "D3D11StateHandler.h"
#include "D3D11TextureHandler.h"
#include "D3D11PipelineManager.h"

namespace SG
{

	class D3D11RenderEngine : public SGRenderEngine
	{
	public:
		D3D11RenderEngine(const SGRenderSettings& settings);
		~D3D11RenderEngine();

	private:
		ID3D11Device* device = nullptr;
		ID3D11DeviceContext* immediateContext = nullptr;
		std::vector<ID3D11DeviceContext*> defferedContexts;
		IDXGISwapChain* swapChain = nullptr;

		D3D11BufferHandler* bufferHandler;
		D3D11DrawDataHandler* drawDataHandler;
		D3D11SamplerHandler* samplerHandler;
		D3D11ShaderManager* shaderManager;
		D3D11StateHandler* stateHandler;
		D3D11TextureHandler* textureHandler;
		D3D11PipelineManager* pipelineManager;

		void CreateDeviceAndContext(const SGRenderSettings& settings);
		void CreateSwapChain(const SGRenderSettings& settings);

		// Inherited via SGRenderEngine
		void SwapUpdateBuffer() override;
		void SwapToWorkWithBuffer() override;
		void HandleRenderJob(const std::vector<SGPipelineJob>& jobs) override;

		void HandlePipelineJobs(const std::vector<SGPipelineJob>& jobs, int startPos, int endPos, ID3D11DeviceContext* context);
		void HandleClearRenderTargetJob(const SGClearRenderTargetJob& job, ID3D11DeviceContext* context);
	};
}