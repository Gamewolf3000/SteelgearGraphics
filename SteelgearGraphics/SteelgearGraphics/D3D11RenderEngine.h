#pragma once

#include <d3d11_4.h>
#include <exception>
#include <stdexcept>

#include "SGRenderEngine.h"

#include "D3D11BufferHandler.h"
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

		D3D11BufferHandler* BufferHandler();
		D3D11ShaderManager* ShaderManager();
		D3D11TextureHandler* TextureHandler();
		D3D11PipelineManager* PipelineManager();

	private:
		ID3D11Device* device = nullptr;
		ID3D11DeviceContext* immediateContext = nullptr;
		std::vector<ID3D11DeviceContext*> defferedContexts;
		IDXGISwapChain* swapChain = nullptr;

		D3D11BufferHandler* bufferHandler;
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
		void ExecuteJobs(const std::vector<SGPipelineJob>& jobs) override;

		void HandlePipelineJobs(const std::vector<SGPipelineJob>& jobs, int startPos, int endPos, ID3D11DeviceContext* context);

		void HandleRenderJob(const SGRenderJob& job, const std::vector<SGGraphicalEntityID>& entities, ID3D11DeviceContext* context);
		void SetShaders(const SGRenderJob& job, ID3D11DeviceContext* context);
		void HandleGlobalRenderJob(const SGRenderJob& job, const std::vector<SGGraphicalEntityID>& entities, ID3D11DeviceContext* context);
		void HandleGroupRenderJob(const SGRenderJob& job, const std::vector<SGGraphicalEntityID>& entities, ID3D11DeviceContext* context);
		void HandleEntityRenderJob(const SGRenderJob& job, const std::vector<SGGraphicalEntityID>& entities, ID3D11DeviceContext* context);

		void HandleClearRenderTargetJob(const SGClearRenderTargetJob& job, ID3D11DeviceContext* context);
	};
}